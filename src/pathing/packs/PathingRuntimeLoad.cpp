#include "PathingIndex.h"

#include "Globals.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>
#include "miniz/miniz.h"

namespace PathingDetail
{
	static int LocateIconEntry(mz_zip_archive& zip, std::string entry)
	{
		DecodeXmlEntities(entry);
		std::replace(entry.begin(), entry.end(), '\\', '/');
		while (entry.rfind("./", 0) == 0)
			entry.erase(0, 2);
		while (!entry.empty() && entry.front() == '/')
			entry.erase(entry.begin());
		int idx = ZipLocate(zip, entry);
		if (idx < 0 && entry.rfind("Data/", 0) == 0)
			idx = ZipLocate(zip, entry.substr(5));
		if (idx < 0)
			idx = ZipLocate(zip, std::string("Data/") + entry);
		if (idx < 0 && entry.rfind("POIs/", 0) == 0)
			idx = ZipLocate(zip, entry.substr(5));
		if (idx < 0)
			idx = ZipLocate(zip, std::string("POIs/") + entry);
		return idx;
	}

	void QueueIconFromPack(const std::wstring& packPath, const std::string& iconFile)
	{
		if (iconFile.empty() || packPath.empty())
			return;
		const std::string tid = IconTextureId(iconFile);
		{
			std::lock_guard<std::mutex> lock(gIconMutex);
			if (gIconQueued[tid] || gIconRetain.count(tid))
				return;
			gIconQueued[tid] = true;
		}

		std::vector<uint8_t> file;
		if (!ReadFileW(packPath, file, kMaxZipBytes) || file.empty())
			return;
		mz_zip_archive zip{};
		if (!mz_zip_reader_init_mem(&zip, file.data(), file.size(), 0))
			return;
		const int idx = LocateIconEntry(zip, iconFile);
		std::vector<uint8_t> bytes;
		if (idx >= 0)
			ZipExtractIndex(zip, idx, bytes, 4u * 1024u * 1024u);
		mz_zip_reader_end(&zip);
		if (bytes.empty())
			return;

		std::lock_guard<std::mutex> lock(gIconMutex);
		gIconRetain[tid] = bytes;
		PendingIcon pend;
		pend.id = tid;
		pend.bytes = std::move(bytes);
		gPendingIcons.push_back(std::move(pend));
	}

	void UploadPendingIcons()
	{
		if (!G::API || !G::API->Textures_GetOrCreateFromMemory)
			return;
		std::vector<PendingIcon> pending;
		{
			std::lock_guard<std::mutex> lock(gIconMutex);
			pending.swap(gPendingIcons);
		}
		for (PendingIcon& p : pending)
		{
			if (p.bytes.empty())
				continue;
			Texture_t* existing = G::API->Textures_Get ? G::API->Textures_Get(p.id.c_str()) : nullptr;
			if (existing && existing->Resource)
				continue;
			G::API->Textures_GetOrCreateFromMemory(
				p.id.c_str(), p.bytes.data(), static_cast<uint64_t>(p.bytes.size()));
		}
	}

	void LoadMapMarkers(uint32_t mapId, uint32_t epoch)
	{
		std::vector<IndexedPoi> poiCopy;
		std::vector<std::string> enabledCopy;
		std::unordered_map<std::string, MarkerStyle> styleCopy;
		{
			std::lock_guard<std::mutex> lock(gMutex);
			poiCopy = gPoiIndex;
			enabledCopy = gEnabledPaths;
			styleCopy = gCategoryStyles;
		}

		if (enabledCopy.empty())
		{
			std::lock_guard<std::mutex> lock(gMutex);
			if (gEpoch.load(std::memory_order_acquire) != epoch)
				return;
			gActiveMap = mapId;
			gLoadedEnabledGen = gEnabledGen.load(std::memory_order_acquire);
			gCurrentMarkers.clear();
			gContentRevision.fetch_add(1, std::memory_order_release);
			return;
		}

		std::vector<const IndexedPoi*> poiCands;
		poiCands.reserve(512);
		for (const IndexedPoi& poi : poiCopy)
		{
			if (poi.mapId != mapId)
				continue;
			if (!TypeEnabledWithEnabled(poi.type, enabledCopy))
				continue;
			poiCands.push_back(&poi);
		}

		std::vector<PathingTrails::Marker> markers;
		markers.reserve(std::min(poiCands.size(), kMaxMarkersPerMap));
		std::unordered_map<std::string, std::wstring> assetsNeeded;

		for (const IndexedPoi* poiPtr : poiCands)
		{
			const IndexedPoi& poi = *poiPtr;
			const MarkerStyle style = ResolveStyle(poi.type, poi.style, styleCopy);
			PathingTrails::Marker m{};
			m.mapId = mapId;
			m.color = style.hasColor ? style.color : 0xFFFFCC33u;
			m.world = { poi.wx, poi.wy, poi.wz };
			m.minimapVisible = style.minimapVisible;
			m.inGameVisible = style.inGameVisible;
			m.mapDisplaySize = std::max(1.f, style.mapDisplaySize);
			m.minSize = std::max(1.f, style.minSize);
			m.maxSize = std::max(m.minSize, style.maxSize);
			m.iconSize = std::max(0.05f, style.iconSize);
			m.heightOffset = style.heightOffset;
			m.fadeNear = style.fadeNear;
			m.fadeFar = style.fadeFar;
			m.alpha = std::clamp(style.alpha, 0.f, 1.f);
			std::snprintf(m.label, sizeof(m.label), "%s", poi.type.c_str());
			if (!poi.guid.empty())
				std::snprintf(m.guid, sizeof(m.guid), "%s", poi.guid.c_str());
			m.behavior = style.behavior;
			m.autoTrigger = style.autoTrigger;
			m.triggerRange = style.hasTriggerRange ? style.triggerRange : 2.f;
			m.resetLength = style.resetLength;
			m.invertBehavior = style.invertBehavior;
			if (style.hasHide)
				std::snprintf(m.hide, sizeof(m.hide), "%s", style.hide.c_str());
			if (style.hasShow)
				std::snprintf(m.show, sizeof(m.show), "%s", style.show.c_str());
			if (style.hasTipName)
				std::snprintf(m.tipName, sizeof(m.tipName), "%s", style.tipName.c_str());
			if (style.hasTipDescription)
				std::snprintf(m.tipDescription, sizeof(m.tipDescription), "%s",
					style.tipDescription.c_str());
			if (style.hasInfo)
				std::snprintf(m.info, sizeof(m.info), "%s", style.info.c_str());
			if (style.hasCopy)
				std::snprintf(m.copy, sizeof(m.copy), "%s", style.copy.c_str());
			if (style.hasCopyMessage)
				std::snprintf(m.copyMessage, sizeof(m.copyMessage), "%s",
					style.copyMessage.c_str());
			if (style.hasSchedule)
				std::snprintf(m.schedule, sizeof(m.schedule), "%s", style.schedule.c_str());
			if (style.hasScheduleDuration)
				m.scheduleDuration = style.scheduleDuration;
			if (style.hasScriptOnce)
				m.scriptOnce = style.scriptOnce;
			if (style.hasScriptTrigger)
				m.scriptTrigger = style.scriptTrigger;
			if (style.hasScriptFilter)
				m.scriptFilter = style.scriptFilter;
			if (style.hasScriptTick)
				m.scriptTick = style.scriptTick;
			if (style.hasScriptFocus)
				m.scriptFocus = style.scriptFocus;
			if (!style.iconFile.empty())
			{
				const std::string tid = IconTextureId(style.iconFile);
				std::snprintf(m.iconId, sizeof(m.iconId), "%s", tid.c_str());
				assetsNeeded.emplace(style.iconFile, poi.packPath);
			}
			markers.push_back(std::move(m));
			if (markers.size() >= kMaxMarkersPerMap)
				break;
		}

		for (const auto& kv : assetsNeeded)
			QueueIconFromPack(kv.second, kv.first);

		std::lock_guard<std::mutex> lock(gMutex);
		if (gEpoch.load(std::memory_order_acquire) != epoch)
			return;
		gActiveMap = mapId;
		gLoadedEnabledGen = gEnabledGen.load(std::memory_order_acquire);
		gCurrentMarkers = std::move(markers);
		gContentRevision.fetch_add(1, std::memory_order_release);
	}
}

#pragma once

#include "PathingTrails.h"
#include "PathingParse.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>
#include "miniz/miniz.h"

/* Pack index / load state for Trail Tools (authoring + Copy from Pathing). */
namespace PathingDetail
{
	using PathingParse::MarkerStyle;
	using PathingParse::IndexedTrail;
	using PathingParse::IndexedPoi;
	using PathingParse::kMaxZipBytes;
	using PathingParse::ToLower;
	using PathingParse::ReadFileW;
	using PathingParse::ZipLocate;
	using PathingParse::ZipExtractIndex;
	using PathingParse::ZipReadEntry;
	using PathingParse::IndexXml;
	using PathingParse::IndexPoisXml;
	using PathingParse::CollectCategoryMapIds;
	using PathingParse::ParseMarkerMenuXml;
	using PathingParse::ResolveStyle;
	using PathingParse::DecodeXmlEntities;

	constexpr size_t kMaxMarkersPerMap = 800;

	extern std::mutex gMutex;
	extern std::atomic<uint32_t> gEpoch;
	extern std::atomic<uint32_t> gLoadGen;
	extern std::atomic<bool> gLoading;
	extern std::atomic<bool> gForceReload;
	extern std::atomic<bool> gIndexStarted;
	extern std::atomic<uint32_t> gEnabledGen;
	extern uint32_t gLoadedEnabledGen;
	extern std::atomic<int> gPackCount;
	extern std::vector<std::string> gPackNames;
	extern std::thread gWorker;

	extern std::vector<IndexedTrail> gIndex;
	extern std::vector<IndexedPoi> gPoiIndex;
	extern std::unordered_map<std::string, MarkerStyle> gCategoryStyles;
	extern std::vector<PathingTrails::Category> gMenu;
	extern std::atomic<uint64_t> gMenuRevision;
	extern std::atomic<uint64_t> gContentRevision;
	extern uint32_t gActiveMap;
	extern std::vector<PathingTrails::Marker> gCurrentMarkers;
	extern std::vector<std::string> gEnabledPaths;

	struct PendingIcon
	{
		std::string id;
		std::vector<uint8_t> bytes;
	};
	extern std::mutex gIconMutex;
	extern std::vector<PendingIcon> gPendingIcons;
	extern std::unordered_map<std::string, bool> gIconQueued;
	extern std::unordered_map<std::string, std::vector<uint8_t>> gIconRetain;

	std::string IconTextureId(const std::string& iconFile);
	bool IsMountShortcutMarker(const PathingTrails::Marker& marker);

	bool PrefixMatchesType(const std::string& typeLow, const std::string& prefixLow);
	bool TypeCategoryEnabled(const std::string& type, const std::vector<std::string>& enabled);
	bool TypeEnabledWithEnabled(const std::string& type, const std::vector<std::string>& enabled);

	void MergeCategoryTree(std::vector<PathingTrails::Category>& dest, PathingTrails::Category&& src);
	void MarkEnabled(std::vector<PathingTrails::Category>& nodes);
	void IndexPack(const std::wstring& packPath, std::vector<IndexedTrail>& out,
		std::vector<IndexedPoi>& poisOut, std::vector<PathingTrails::Category>& menuOut,
		std::unordered_map<std::string, MarkerStyle>& stylesOut, uint32_t epoch);
	void ListTacoFiles(const std::wstring& dir, std::vector<std::wstring>& out);
	void WorkerIndexAndLoad(uint32_t epoch, uint32_t mapId);
	void LoadMapMarkers(uint32_t mapId, uint32_t epoch);
	void QueueIconFromPack(const std::wstring& packPath, const std::string& iconFile);
	void UploadPendingIcons();
}

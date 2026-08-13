#include "PathingIndex.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace PathingDetail
{
	std::mutex gMutex;
	std::atomic<uint32_t> gEpoch{0};
	std::atomic<uint32_t> gLoadGen{0};
	std::atomic<bool> gLoading{false};
	std::atomic<bool> gForceReload{false};
	std::atomic<bool> gIndexStarted{false};
	std::atomic<uint32_t> gEnabledGen{1};
	uint32_t gLoadedEnabledGen = 0;
	std::atomic<int> gPackCount{0};
	std::vector<std::string> gPackNames;
	std::thread gWorker;

	std::vector<IndexedTrail> gIndex;
	std::vector<IndexedPoi> gPoiIndex;
	std::unordered_map<std::string, MarkerStyle> gCategoryStyles;
	std::vector<PathingTrails::Category> gMenu;
	std::atomic<uint64_t> gMenuRevision{1};
	std::atomic<uint64_t> gContentRevision{1};
	uint32_t gActiveMap = 0;
	std::vector<PathingTrails::Marker> gCurrentMarkers;
	std::vector<std::string> gEnabledPaths;

	std::mutex gIconMutex;
	std::vector<PendingIcon> gPendingIcons;
	std::unordered_map<std::string, bool> gIconQueued;
	std::unordered_map<std::string, std::vector<uint8_t>> gIconRetain;

	std::string IconTextureId(const std::string& iconFile)
	{
		std::string id = "TW_ICO_";
		for (char c : iconFile)
		{
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9'))
				id += c;
			else
				id += '_';
		}
		if (id.size() > 120)
			id.resize(120);
		return id;
	}

	bool IsMountShortcutMarker(const PathingTrails::Marker&)
	{
		return false;
	}

	bool PrefixMatchesType(const std::string& typeLow, const std::string& prefixLow)
	{
		if (prefixLow.empty())
			return false;
		if (typeLow == prefixLow)
			return true;
		if (typeLow.size() > prefixLow.size() &&
			typeLow.compare(0, prefixLow.size(), prefixLow) == 0 &&
			typeLow[prefixLow.size()] == '.')
			return true;
		return false;
	}

	bool TypeCategoryEnabled(const std::string& type, const std::vector<std::string>& enabled)
	{
		if (enabled.empty() || type.empty())
			return false;
		const std::string low = ToLower(type);
		for (const std::string& p : enabled)
		{
			if (p.empty())
				continue;
			if (PrefixMatchesType(low, ToLower(p)))
				return true;
		}
		return false;
	}

	bool TypeEnabledWithEnabled(const std::string& type, const std::vector<std::string>& enabled)
	{
		return TypeCategoryEnabled(type, enabled);
	}

	void MarkEnabled(std::vector<PathingTrails::Category>& nodes)
	{
		for (PathingTrails::Category& n : nodes)
		{
			n.enabled = TypeCategoryEnabled(n.path, gEnabledPaths);
			MarkEnabled(n.children);
		}
	}
}

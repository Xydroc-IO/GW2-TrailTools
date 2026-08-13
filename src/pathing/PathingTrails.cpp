#include "PathingTrails.h"

#include "AddonPaths.h"
#include "Globals.h"
#include "PathingIndex.h"
#include "Settings.h"
#include "WorldGpsD3d.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace PathingDetail;

void PathingTrails::Init()
{
	AddonPaths::EnsureUnder(AddonPaths::DataDir(), L"pathing");
	if (G::PathingEnabled[0])
		ParseEnabledPaths(G::PathingEnabled);
}

void PathingTrails::Shutdown()
{
	WorldGpsD3d::Shutdown();
	const uint32_t epoch = gEpoch.fetch_add(1, std::memory_order_acq_rel) + 1;
	(void)epoch;
	gForceReload.store(false, std::memory_order_release);
	if (gWorker.joinable())
	{
		/* Detached workers check gEpoch; join only if still ours. */
		gWorker.detach();
	}
	std::lock_guard<std::mutex> lock(gMutex);
	gIndex.clear();
	gPoiIndex.clear();
	gCategoryStyles.clear();
	gMenu.clear();
	gCurrentMarkers.clear();
	gIndexStarted.store(false, std::memory_order_release);
	gLoading.store(false, std::memory_order_release);
}

void PathingTrails::BeginFrame()
{
	UploadPendingIcons();
}

static void SpawnWorker(uint32_t mapId)
{
	const uint32_t epoch = gEpoch.fetch_add(1, std::memory_order_acq_rel) + 1;
	const uint32_t loadGen = gLoadGen.fetch_add(1, std::memory_order_acq_rel) + 1;
	gLoading.store(true, std::memory_order_release);
	if (gWorker.joinable())
		gWorker.detach();
	gWorker = std::thread([epoch, loadGen, mapId]() {
		struct Guard
		{
			uint32_t loadGen = 0;
			~Guard()
			{
				if (gLoadGen.load(std::memory_order_acquire) == loadGen)
					gLoading.store(false, std::memory_order_release);
			}
		} guard{ loadGen };
		WorkerIndexAndLoad(epoch, mapId);
	});
}

void PathingTrails::Update(uint32_t mapId)
{
	if (mapId == 0)
		return;

	if (!gIndexStarted.load(std::memory_order_acquire))
	{
		gIndexStarted.store(true, std::memory_order_release);
		SpawnWorker(mapId);
		return;
	}

	if (gLoading.load(std::memory_order_acquire))
		return;

	uint32_t active = 0;
	uint32_t loadedGen = 0;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		active = gActiveMap;
		loadedGen = gLoadedEnabledGen;
	}
	const uint32_t wantGen = gEnabledGen.load(std::memory_order_acquire);
	const bool force = gForceReload.exchange(false, std::memory_order_acq_rel);
	if (mapId == active && wantGen == loadedGen && !force)
		return;

	const uint32_t epoch = gEpoch.fetch_add(1, std::memory_order_acq_rel) + 1;
	const uint32_t loadGen = gLoadGen.fetch_add(1, std::memory_order_acq_rel) + 1;
	gLoading.store(true, std::memory_order_release);
	if (gWorker.joinable())
		gWorker.detach();
	gWorker = std::thread([epoch, loadGen, mapId]() {
		struct Guard
		{
			uint32_t loadGen = 0;
			~Guard()
			{
				if (gLoadGen.load(std::memory_order_acquire) == loadGen)
					gLoading.store(false, std::memory_order_release);
			}
		} guard{ loadGen };
		LoadMapMarkers(mapId, epoch);
	});
}

void PathingTrails::ReloadPacks()
{
	gIndexStarted.store(false, std::memory_order_release);
	gForceReload.store(true, std::memory_order_release);
	uint32_t mapId = 0;
	if (G::Mumble)
	{
		const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
		if (ctx)
			mapId = ctx->mapId;
	}
	if (mapId == 0)
	{
		std::lock_guard<std::mutex> lock(gMutex);
		mapId = gActiveMap;
	}
	SpawnWorker(mapId);
}

std::vector<PathingTrails::Marker> PathingTrails::CurrentMarkers()
{
	std::unique_lock<std::mutex> lock(gMutex, std::try_to_lock);
	if (!lock.owns_lock())
		return {};
	return gCurrentMarkers;
}

std::vector<std::string> PathingTrails::EnabledPaths()
{
	std::lock_guard<std::mutex> lock(gMutex);
	return gEnabledPaths;
}

void PathingTrails::SetEnabledPaths(const std::vector<std::string>& paths)
{
	std::lock_guard<std::mutex> lock(gMutex);
	gEnabledPaths = paths;
	MarkEnabled(gMenu);
	gMenuRevision.fetch_add(1, std::memory_order_release);
	gEnabledGen.fetch_add(1, std::memory_order_release);
	gForceReload.store(true, std::memory_order_release);
}

void PathingTrails::SerializeEnabledPaths(char* out, size_t outLen)
{
	if (!out || outLen == 0)
		return;
	out[0] = 0;
	const std::vector<std::string> paths = EnabledPaths();
	size_t used = 0;
	for (size_t i = 0; i < paths.size(); ++i)
	{
		const std::string& p = paths[i];
		if (p.empty())
			continue;
		const size_t need = p.size() + (used ? 1u : 0u);
		if (used + need + 1 >= outLen)
			break;
		if (used)
			out[used++] = '|';
		std::memcpy(out + used, p.c_str(), p.size());
		used += p.size();
		out[used] = 0;
	}
}

void PathingTrails::ParseEnabledPaths(const char* pipeList)
{
	std::vector<std::string> paths;
	if (pipeList && pipeList[0])
	{
		std::string cur;
		for (const char* p = pipeList; ; ++p)
		{
			const char c = *p;
			if (c == '|' || c == ',' || c == '\n' || c == '\r' || c == 0)
			{
				while (!cur.empty() && (cur.back() == ' ' || cur.back() == '\t'))
					cur.pop_back();
				size_t start = 0;
				while (start < cur.size() && (cur[start] == ' ' || cur[start] == '\t'))
					++start;
				if (start < cur.size())
					paths.push_back(cur.substr(start));
				cur.clear();
				if (c == 0)
					break;
				continue;
			}
			cur.push_back(c);
		}
	}
	SetEnabledPaths(paths);
}

void PathingTrails::SetCategoryEnabled(const std::string& path, bool enabled)
{
	if (path.empty())
		return;
	std::lock_guard<std::mutex> lock(gMutex);
	const std::string low = ToLower(path);
	gEnabledPaths.erase(
		std::remove_if(gEnabledPaths.begin(), gEnabledPaths.end(),
			[&](const std::string& p) {
				const std::string el = ToLower(p);
				return PrefixMatchesType(el, low) || PrefixMatchesType(low, el);
			}),
		gEnabledPaths.end());
	if (enabled)
		gEnabledPaths.push_back(path);
	MarkEnabled(gMenu);
	gMenuRevision.fetch_add(1, std::memory_order_release);
	gEnabledGen.fetch_add(1, std::memory_order_release);
	gForceReload.store(true, std::memory_order_release);
	Settings::SetDirty();
}

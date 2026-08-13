#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/* Types + pack runtime API used by Trail Tools authoring. */
namespace PathingTrails
{
	struct Point
	{
		float x = 0.f;
		float y = 0.f;
	};

	struct WorldPoint
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	struct Trail
	{
		uint32_t mapId = 0;
		uint32_t color = 0xFFFFFFFFu;
		char     label[96]{};
		char     guid[96]{};
		char     textureId[160]{};
		bool     minimapVisible = true;
		bool     inGameVisible = true;
		float    alpha = 1.f;
		float    trailScale = 1.f;
		float    animSpeed = 1.f;
		float    fadeNear = -1.f;
		float    fadeFar = -1.f;
		char     schedule[96]{};
		float    scheduleDuration = 0.f;
		bool     luaHidden = false;
		bool     luaRemoved = false;
		std::vector<Point> points;
		std::vector<WorldPoint> worldPoints;
	};

	struct Marker
	{
		uint32_t mapId = 0;
		uint32_t color = 0xFFFFCC33;
		char     label[96]{};
		char     iconId[160]{};
		char     guid[96]{};
		Point    pos;
		WorldPoint world;
		bool     minimapVisible = true;
		bool     inGameVisible = true;
		bool     mapVisible = true;
		float    alpha = 1.f;
		float    iconSize = 1.f;
		float    heightOffset = 1.5f;
		float    mapDisplaySize = 20.f;
		float    fadeNear = -1.f;
		float    fadeFar = -1.f;
		float    minSize = 5.f;
		float    maxSize = 2048.f;
		int      behavior = 0;
		bool     autoTrigger = false;
		float    triggerRange = 2.f;
		float    resetLength = 0.f;
		bool     invertBehavior = false;
		char     tipName[96]{};
		char     tipDescription[384]{};
		char     info[768]{};
		char     copy[256]{};
		char     copyMessage[128]{};
		char     hide[256]{};
		char     show[256]{};
		char     schedule[96]{};
		float    scheduleDuration = 0.f;
		std::string scriptOnce;
		std::string scriptTrigger;
		std::string scriptFilter;
		std::string scriptTick;
		std::string scriptFocus;
		bool     luaHidden = false;
		bool     luaRemoved = false;
		bool     luaDynamic = false;
	};

	struct Category
	{
		std::string path;
		std::string label;
		std::string tip;
		int         trails = 0;
		bool        enabled = false;
		bool        separator = false;
		bool        hidden = false;
		std::vector<Category> children;
	};

	struct WorldSnippet
	{
		uint32_t color = 0xFFFFFFFFu;
		char textureId[160]{};
		char label[96]{};
		float alpha = 1.f;
		float trailScale = 1.f;
		float fadeNear = -1.f;
		float fadeFar = -1.f;
		float uvAlong0 = 0.f;
		std::vector<WorldPoint> points;
	};

	void Init();
	void Shutdown();
	void Update(uint32_t mapId);
	void BeginFrame();
	void ReloadPacks();

	std::vector<Marker> CurrentMarkers();
	void SetCategoryEnabled(const std::string& path, bool enabled);
	void SerializeEnabledPaths(char* out, size_t outLen);
	void ParseEnabledPaths(const char* pipeList);
	std::vector<std::string> EnabledPaths();
	void SetEnabledPaths(const std::vector<std::string>& paths);
}

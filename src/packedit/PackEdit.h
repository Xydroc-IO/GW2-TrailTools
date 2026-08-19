#pragma once

#include "PathingParse.h"
#include "PathingTrails.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/* In-memory marker pack for the Editor tab (our model — not MarkerPackEditor). */
namespace PackEdit
{
	enum class AttrKind { String, Float, Int, Bool };

	struct AttrDef
	{
		const char* name;
		AttrKind    kind;
		bool        onPoi;
		bool        onTrail;
		bool        onCat;
	};

	struct PeCategory
	{
		std::string name;
		std::string display;
		std::string path;
		bool hidden = false;
		std::unordered_map<std::string, std::string> attrs;
		std::vector<PeCategory> children;
	};

	struct PePathable
	{
		bool        isTrail = false;
		bool        tombstone = false;
		std::string type;
		std::string guid;
		std::string xmlFile;
		std::string trailData;
		std::string rawTag;
		uint32_t    mapId = 0;
		float       x = 0.f, y = 0.f, z = 0.f;
		float       rotate = 0.f;
		PathingParse::MarkerStyle style{};
		std::unordered_map<std::string, std::string> extra;
		std::vector<PathingTrails::WorldPoint> points;
	};

	struct PeEntry
	{
		std::string name;
		std::vector<uint8_t> bytes;
	};

	struct PeDoc
	{
		std::wstring path;
		bool fromZip = true;
		bool dirty = false;
		char status[384]{};
		std::vector<PeEntry> entries;
		std::vector<PeCategory> roots;
		std::vector<PePathable> items;
		std::unordered_set<std::string> hidden; /* category paths */
		std::unordered_map<std::string, PathingParse::MarkerStyle> catStyles;
		int  selItem = -1;
		int  selPoint = -1;
		std::vector<int> selItems;
		bool worldDraw = true;
		bool gizmoOn = true;
		bool rotateMode = false;
		bool thisMapOnly = true;
		bool popTree = false;
		bool popDet = false;
		bool popRes = false;
		bool popMap = false;
	};

	const AttrDef* AttrTable(int& count);
	void DrawTab();
	void DrawWorldToggles();
	void Tick();
	void RenderWorld();
	bool OpenZip(const std::wstring& path, std::string& err);
	bool OpenFolder(const std::wstring& dir, std::string& err);
	void NewEmpty();
	void ClosePack();
	bool SaveZip(const std::wstring& path, std::string& err);
	void PushUndo();
	bool Undo();
	bool Redo();
	void AddPoiAtFeet();
	void AddPoiAt(float x, float y, float z, uint32_t mapId);
	void AddTrailEmpty();
	void AddCategory();
	void TombstoneSelected();
	void DuplicateSelected();
	void SelectToggle(int index);
	bool IsSelected(int index);
	void DrawPopouts();
	int  LintIssues();
	PathingParse::MarkerStyle EffectiveStyle(const PePathable& p);
}

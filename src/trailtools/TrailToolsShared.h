#pragma once

#include "PathingTrails.h"

#include <cstdint>
#include <string>
#include <vector>

/* Draft pack state for Trail Tools authoring (in-memory + disk under pathing/authoring/). */
namespace TrailToolsDetail
{
	struct CategoryNode
	{
		std::string name;
		std::string displayName;
		std::string iconFile;
		std::string texture;
		float       fadeNear = -1.f;
		float       fadeFar = -1.f;
		float       trailScale = 1.f;
		float       iconSize = 1.f;
		float       alpha = 1.f;
		uint32_t    color = 0; /* 0 = omit; else AARRGGBB */
		std::string schedule; /* Blish UTC cron; empty = always */
		float       scheduleDuration = 0.f;
		std::vector<CategoryNode> children;
	};

	struct DraftPoi
	{
		uint32_t    mapId = 0;
		float       x = 0.f;
		float       y = 0.f;
		float       z = 0.f;
		std::string type;
		std::string guid;
		int         behavior = 0;
		bool        autoTrigger = false;
		float       triggerRange = 2.f;
		float       resetLength = 0.f;
		bool        invertBehavior = false;
		float       fadeNear = -1.f;
		float       fadeFar = -1.f;
		float       alpha = 1.f;
		float       iconSize = 1.f;
		float       heightOffset = 1.5f;
		float       mapDisplaySize = 20.f;
		float       minSize = 5.f;
		float       maxSize = 2048.f;
		bool        minimapVisible = true;
		bool        inGameVisible = true;
		int         achievementId = -1; /* −1 = omit */
		int         achievementBit = -1;
		std::string festival;
		int         profession = 0; /* 0 = omit / any */
		int         race = 0;
		int         mount = 0;
		std::string toggleCategory;
		std::string tipName;
		std::string tipDescription;
		std::string info;
		std::string copy;
		std::string copyMessage;
		std::string schedule;
		float       scheduleDuration = 0.f;
		std::string iconFile;
		std::string hide;
		std::string show;
		std::string scriptOnce;
		std::string scriptTrigger;
		std::string scriptFilter;
		std::string scriptTick;
		std::string scriptFocus;
	};

	struct DraftTrail
	{
		std::string fileRel; /* e.g. Data/example.trl (TacO trailData) */
		std::string type;    /* category path e.g. mymarkers.example */
		uint32_t    mapId = 0;
		/* Optional per-<Trail> overrides (category Looks used when unset / default). */
		std::string texture;
		float       animSpeed = 1.f;
		float       alpha = 1.f;
		float       fadeNear = -1.f;
		float       fadeFar = -1.f;
		float       trailScale = 1.f;
		/* World meters X Y Z (Y up). (0,0,0) = section break. */
		std::vector<PathingTrails::WorldPoint> points;
	};

	struct DraftPack
	{
		char          packName[64] = "ExamplePack";
		char          displayName[96] = "Example Pack";
		CategoryNode  root;
		std::vector<DraftPoi>   pois;
		std::vector<DraftTrail> trails;
		DraftTrail    active; /* currently recording */
		char          markerType[160] = {};
		char          trailType[160] = {}; /* default category for new/active trail */
		char          trailFileStem[64] = "example";
		char          lastTrlDir[260] = {}; /* last Load/Save As folder */
		char          xmlPath[260] = {}; /* open OverlayData project path (utf-8) */
		bool          xmlDirty = false;
		char          status[384] = {};
		bool          previewEnabled = true;
		bool          trailDirty = false;
		int           xmlLayout = 0; /* unused: always one OverlayData file */
		int           selectedPoi = -1;
		int           selectedTrail = -1;
		int           selectedPoint = -1; /* index in active.points */
	};

	extern DraftPack gDraft;
	extern bool      gPlaceOnce;
	extern bool      gFocus;
	extern int       gTab; /* 0 Pack, 1 Content, 2 Live, 3 Keybinds */

	/* Multiple TrailsN / MarkersN editors (mockup: Trails1+Trails2, Markers1+Markers2). */
	constexpr int kMaxTrailEditors = 5;
	constexpr int kMaxMarkerEditors = 4;

	struct TrailEditorSlot
	{
		bool      open = false;
		bool      placeOnce = false;
		bool      focus = false;
		bool      dirty = false;
		int       selectedPoint = -1;
		std::vector<int> selectedPoints; /* multi-select (Ctrl/Shift in TrailsN) */
		DraftTrail trail{};
		char      stem[64] = "Trail";
		float     geomX = -1.f;
		float     geomY = -1.f;
		float     geomW = 0.f;
		float     geomH = 0.f;
	};

	/* World click place/select (Live tab). Plane = feet Y via Mumble camera. */
	extern bool gWorldPickEnabled;
	extern int  gWorldPickMode; /* 0 place marker, 1 add trail pt, 2 select nearest */

	struct MarkerEditorSlot
	{
		bool  open = false;
		bool  placeOnce = false;
		bool  focus = false;
		int   poiIndex = -1; /* index into gDraft.pois */
		float geomX = -1.f;
		float geomY = -1.f;
		float geomW = 0.f;
		float geomH = 0.f;
	};

	extern TrailEditorSlot  gTrailEditors[kMaxTrailEditors];
	extern MarkerEditorSlot gMarkerEditors[kMaxMarkerEditors];
	/* Trails / Markers XML desks as their own windows (hub can stay on Live). */
	extern bool  gShowTrailsDesk;
	extern bool  gShowMarkersDesk;
	extern bool  gPlaceOnceTrailsDesk;
	extern bool  gFocusTrailsDesk;
	extern bool  gPlaceOnceMarkersDesk;
	extern bool  gFocusMarkersDesk;
	extern float gTrailsDeskX, gTrailsDeskY, gTrailsDeskW, gTrailsDeskH;
	extern float gMarkersDeskX, gMarkersDeskY, gMarkersDeskW, gMarkersDeskH;
	/* Last-focused TrailsN for keybind recording (−1 = gDraft.active). */
	extern int gTrailRecordSlot;
	/* Hub: ignore Pop out / New window for one frame after a side-rail tab click. */
	extern bool gHubSkipOpenClicks;
	/* True while PushTrailEditorToActive holds a TrailsN trail in gDraft.active. */
	extern bool gTrailEditorDrawActive;

	DraftTrail& RecordingTrail(); /* editor slot if focused, else gDraft.active */
	int& RecordingSelectedPoint();
	bool& RecordingTrailDirty();

	/* Legacy single-flag aliases used by unload; prefer desks + editor slots. */
	extern bool gPopoutTrails;
	extern bool gPopoutMarkers;

	/* Hub, a desk, or any TrailsN/MarkersN editor is open (draft preview). */
	bool AnyAuthoringPadOpen();
	void CloseAllPopouts();
	void OpenTrailsDesk();
	void OpenMarkersDesk();
	int  OpenNewTrailEditor(); /* −1 if full; opens next free TrailsN (keeps others open) */
	int  OpenTrailEditorSlot(int slot); /* open/focus specific TrailsN; −1 if bad */
	int  OpenMarkerEditor(int poiIndex, bool forceNew = false); /* forceNew skips focus-existing */
	int  OpenNewMarkerEditor(); /* next free MarkersN for selected (or newly dropped) POI */
	/* Swap gDraft.active ↔ editor slot for raw UI / keybind recording. */
	void PushTrailEditorToActive(int slot);
	void PopTrailEditorFromActive(int slot);

	void SetStatus(const char* fmt, ...);
	void SeedDefaultCategories();
	void SanitizePackName(char* name, size_t len);
	std::string RootCategoryName();
	std::string CategoryPath(const CategoryNode& node, const std::string& parentPath);
	void CollectLeafPaths(const CategoryNode& node, const std::string& parentPath,
		std::vector<std::string>& out, bool trailLeaves);
	std::wstring AuthoringRoot();
	std::wstring PackDir();
	bool EnsureWorkspace();
	bool WriteDefaultAssets(); /* ExampleMarker.png + Trail.png if missing */
	bool OpenAuthoringFolder();
	void CopyClipboard(const char* text);
	bool ReadMumblePose(uint32_t& mapId, float& x, float& y, float& z);
	std::string MakeGuidBase64();
	bool HasDraftPreview(); /* trail pts or POIs on current map */
	CategoryNode* FindCategoryByPath(CategoryNode& node, const std::string& wantPath,
		const std::string& parentPath = {});
	/* After pack rename: remap category paths, POI types, trail types, Data/ paths. */
	void RemapDraftAfterPackRename(const std::string& oldPackName, const std::string& oldRoot);
	void ApplyTrailLookPreset(int presetIndex);
	void ApplyMarkerLookPreset(int presetIndex);
	const char* const* TrailLookPresetNames(int* count);
	const char* const* MarkerLookPresetNames(int* count);

	/* Session + import (TrailToolsPersist / TrailToolsImport). */
	bool SaveDraftSession();
	bool LoadDraftSession();
	bool ImportTacoToDraft(const std::wstring& tacoPath, std::string& err);

	/* Shared OverlayData project desk (Trails + Markers hubs). */
	void UpsertActiveTrailInPack();
	void UpsertSelectedPoiInPack(); /* no-op if none selected; POIs already live in draft */
	bool SaveProjectXml(bool saveAs);
	bool LoadProjectXml();
	void NewProjectXml();
}

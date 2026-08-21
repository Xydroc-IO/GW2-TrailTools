#pragma once

#include "TrailToolsShared.h"

#include <string>

/* Trail Tools pad tab drawers (TrailToolsPad*.cpp). */
namespace TrailToolsDetail
{
	void DrawLiveTab(); /* pose / UberTool / world click — drawn on Content */
	void DrawContentTab(); /* Live + OverlayData authoring hub */
	void DrawTrailTab(); /* full tab when docked: desk + raw (legacy entry) */
	void DrawTrailDesk(bool asPopout = false); /* category + list + TrailsN (no XML chrome) */
	void DrawTrailRawEditor(); /* uses gDraft.active (after PushTrailEditorToActive) */
	void DrawMarkersTab();
	void DrawMarkersDesk(bool asPopout = false); /* category + list + MarkersN (no XML chrome) */
	void DrawMarkerRawEditor(); /* uses gDraft.selectedPoi */
	void DrawMarkerRawEditorForSlot(int slot); /* MarkersN bound to slot.poiIndex */
	void DrawSelectedPoiEditor(DraftPoi& p); /* MarkersN Settings + legacy raw */
	void DrawXmlProjectStrip(); /* Content — OverlayData path + New/Load/Save */
	void DrawXmlEditorBody();
	void DrawXmlEditorPane(float height);
	bool RenderXmlEditorPad(); /* pop-out OverlayData text editor */
	void DrawPackTab();
	void DrawLooksDefaultsUi(); /* Nexus Options — trail texture / marker icon defaults */
	void DrawKeybindsTab();
	void DrawPoiScriptAttrs(DraftPoi& p);
	void DrawPoiBehaviorAndFilters(DraftPoi& p);
	void DrawLuaFilesUi();
	void DrawTrailAttrsSection();
	void DrawTrailGeomSection();

	/* Shared by TrailToolsPadTrailDesk / Raw / Helpers. */
	void SyncActiveType();
	void SyncActiveFileRelFromStem();
	void ApplyStemFromFileRel();
	void MarkDirty();

	std::wstring Utf8ToWide(const char* u);
	std::string WideToUtf8(const std::wstring& w);
	std::wstring PackRelToAbs(const std::string& fileRel);
	std::wstring TrailsFolder();
	std::wstring ActiveTrlPath();
	void RememberDirFromPath(const std::wstring& fullPath);
	std::wstring DialogStartDir();
	bool IsSectionBreak(const PathingTrails::WorldPoint& p);
	bool TryAbsUnderPack(const std::wstring& absPath, std::string& outRel);
	void RegisterActiveInPack();
	bool SaveActiveToPath(const std::wstring& path);
	bool DialogPickTrl(bool saveAs, std::wstring& outPath);
}

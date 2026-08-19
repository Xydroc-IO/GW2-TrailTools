#include "TrailToolsShared.h"
#include "TrailToolsBinds.h"

namespace TrailToolsDetail
{
	DraftPack gDraft{};
	bool      gPlaceOnce = false;
	bool      gFocus = false;
	int       gTab = 0;

	TrailEditorSlot  gTrailEditors[kMaxTrailEditors]{};
	MarkerEditorSlot gMarkerEditors[kMaxMarkerEditors]{};
	bool  gShowTrailsDesk = false;
	bool  gShowMarkersDesk = false;
	bool  gShowXmlEdit = false;
	bool  gPlaceOnceTrailsDesk = false;
	bool  gFocusTrailsDesk = false;
	bool  gPlaceOnceMarkersDesk = false;
	bool  gFocusMarkersDesk = false;
	bool  gPlaceOnceXmlEdit = false;
	bool  gFocusXmlEdit = false;
	float gTrailsDeskX = -1.f, gTrailsDeskY = -1.f, gTrailsDeskW = 0.f, gTrailsDeskH = 0.f;
	float gMarkersDeskX = -1.f, gMarkersDeskY = -1.f, gMarkersDeskW = 0.f, gMarkersDeskH = 0.f;
	float gXmlEditX = -1.f, gXmlEditY = -1.f, gXmlEditW = 0.f, gXmlEditH = 0.f;
	std::string gXmlEdit;
	bool  gXmlEditDirty = false;
	int   gTrailRecordSlot = -1;
	int   gTrailEditorDrawSlot = -1;
	bool  gHubSkipOpenClicks = false;
	bool  gTrailEditorDrawActive = false;
	bool  gWorldPickEnabled = false;
	int   gWorldPickMode = 0;
	bool  gUberToolEnabled = true;
	bool  gGroundSnap = true;
	bool  gPopoutTrails = false;
	bool  gPopoutMarkers = false;

	DraftTrail& RecordingTrail()
	{
		/* During Push/Pop draw, slot.trail is swapped empty — use active. */
		if (gTrailEditorDrawActive)
			return gDraft.active;
		if (gTrailRecordSlot >= 0 && gTrailRecordSlot < kMaxTrailEditors &&
			gTrailEditors[gTrailRecordSlot].open)
			return gTrailEditors[gTrailRecordSlot].trail;
		return gDraft.active;
	}

	int& RecordingSelectedPoint()
	{
		if (gTrailEditorDrawActive)
			return gDraft.selectedPoint;
		if (gTrailRecordSlot >= 0 && gTrailRecordSlot < kMaxTrailEditors &&
			gTrailEditors[gTrailRecordSlot].open)
			return gTrailEditors[gTrailRecordSlot].selectedPoint;
		return gDraft.selectedPoint;
	}

	bool& RecordingTrailDirty()
	{
		if (gTrailEditorDrawActive)
			return gDraft.trailDirty;
		if (gTrailRecordSlot >= 0 && gTrailRecordSlot < kMaxTrailEditors &&
			gTrailEditors[gTrailRecordSlot].open)
			return gTrailEditors[gTrailRecordSlot].dirty;
		return gDraft.trailDirty;
	}

	bool& RecordingWorldShown()
	{
		int slot = -1;
		if (gTrailEditorDrawActive)
			slot = gTrailEditorDrawSlot;
		else
			slot = gTrailRecordSlot;
		if (slot >= 0 && slot < kMaxTrailEditors && gTrailEditors[slot].open)
			return gTrailEditors[slot].worldShown;
		return gDraft.trailWorldShown;
	}

	bool DraftWorldVisible()
	{
		if (!gDraft.previewEnabled)
			return false;
		int slot = -1;
		if (gTrailEditorDrawActive)
			slot = gTrailEditorDrawSlot;
		else
			slot = gTrailRecordSlot;
		if (slot < 0 || slot >= kMaxTrailEditors || !gTrailEditors[slot].open)
			return false;
		if (TrailToolsBinds::Get().trailRecording)
			return true;
		return gTrailEditors[slot].worldShown;
	}

	void ClearWorldDraftTrails()
	{
		TrailToolsBinds::Get().trailRecording = false;
		TrailToolsBinds::Get().trailPaused = false;
		gDraft.active.points.clear();
		gDraft.active.mapId = 0;
		gDraft.selectedPoint = -1;
		gDraft.trailWorldShown = false;
		gDraft.trailDirty = true;
		for (int i = 0; i < kMaxTrailEditors; ++i)
		{
			gTrailEditors[i].trail.points.clear();
			gTrailEditors[i].trail.mapId = 0;
			gTrailEditors[i].selectedPoint = -1;
			gTrailEditors[i].selectedPoints.clear();
			gTrailEditors[i].worldShown = false;
			gTrailEditors[i].dirty = true;
		}
		SetStatus("Cleared world draft trail (not in a Trails window).");
	}
}

namespace
{
	struct SeedOnce
	{
		SeedOnce()
		{
			TrailToolsDetail::SeedDefaultCategories();
			TrailToolsBinds::SetDefaults();
		}
	};
	SeedOnce gSeed;
}

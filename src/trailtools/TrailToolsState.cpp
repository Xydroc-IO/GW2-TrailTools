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
	bool  gPlaceOnceTrailsDesk = false;
	bool  gFocusTrailsDesk = false;
	bool  gPlaceOnceMarkersDesk = false;
	bool  gFocusMarkersDesk = false;
	float gTrailsDeskX = -1.f, gTrailsDeskY = -1.f, gTrailsDeskW = 0.f, gTrailsDeskH = 0.f;
	float gMarkersDeskX = -1.f, gMarkersDeskY = -1.f, gMarkersDeskW = 0.f, gMarkersDeskH = 0.f;
	int   gTrailRecordSlot = -1;
	bool  gHubSkipOpenClicks = false;
	bool  gTrailEditorDrawActive = false;
	bool  gWorldPickEnabled = false;
	int   gWorldPickMode = 0;
	bool  gUberToolEnabled = true;
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

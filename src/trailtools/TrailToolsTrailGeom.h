#pragma once

#include "TrailToolsShared.h"

#include <vector>

/* Geometry ops on DraftTrail point polylines (section breaks = 0,0,0). */
namespace TrailToolsTrailGeom
{
	bool IsBreak(const PathingTrails::WorldPoint& p);
	void Reverse(TrailToolsDetail::DraftTrail& trail);
	/* Insert midpoints so consecutive non-break spacing ≤ maxSpacing meters. */
	void Densify(TrailToolsDetail::DraftTrail& trail, float maxSpacing);
	/* Chaikin corner-cutting; preserves section breaks. passes = 1..2. */
	void Smooth(TrailToolsDetail::DraftTrail& trail, int passes = 1);
	void DeleteIndices(TrailToolsDetail::DraftTrail& trail, std::vector<int>& selected,
		int& primarySel);
	void ClearSelection(std::vector<int>& selected);
	bool IsSelected(const std::vector<int>& selected, int idx);
	void ToggleSelect(std::vector<int>& selected, int idx);
	void SelectRange(std::vector<int>& selected, int a, int b);
}

#include "TrailToolsBinds.h"

#include "TrailToolsEditUndo.h"
#include "TrailToolsShared.h"
#include "TrailToolsUberTool.h"

namespace
{
	int NearestTrailPointIndex(float x, float y, float z)
	{
		using namespace TrailToolsDetail;
		DraftTrail& tr = RecordingTrail();
		int best = -1;
		float bestD = 1.e30f;
		for (int i = 0; i < static_cast<int>(tr.points.size()); ++i)
		{
			const auto& p = tr.points[static_cast<size_t>(i)];
			if (p.x == 0.f && p.y == 0.f && p.z == 0.f)
				continue;
			const float dx = p.x - x, dy = p.y - y, dz = p.z - z;
			const float d = dx * dx + dy * dy + dz * dz;
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		return best;
	}
}

void TrailToolsBinds::ActionTrailStop()
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();
	if (!gBinds.trailRecording)
	{
		SetStatus("Not recording.");
		return;
	}
	gBinds.trailRecording = false;
	gBinds.trailPaused = false;
	SetStatus("Recording stopped (%zu pts).", RecordingTrail().points.size());
}

void TrailToolsBinds::ActionTrailInsertVector()
{
	using namespace TrailToolsDetail;
	DraftTrail& tr = RecordingTrail();
	int& sel = RecordingSelectedPoint();
	bool& dirty = RecordingTrailDirty();
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	if (tr.mapId == 0)
		tr.mapId = mapId;
	else if (tr.mapId != mapId)
	{
		SetStatus("Map mismatch - trail %u, you %u.", tr.mapId, mapId);
		return;
	}
	if (tr.type.empty() && gDraft.trailType[0])
		tr.type = gDraft.trailType;
	TrailToolsEditUndo::PushTrail();
	tr.points.push_back({ x, y, z });
	sel = static_cast<int>(tr.points.size()) - 1;
	dirty = true;
	RecordingWorldShown() = true;
	SetStatus("Point #%zu at feet.", tr.points.size());
}

void TrailToolsBinds::ActionTrailSelectNearest()
{
	using namespace TrailToolsDetail;
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	const int best = NearestTrailPointIndex(x, y, z);
	if (best < 0)
	{
		SetStatus("No vectors to select.");
		return;
	}
	RecordingSelectedPoint() = best;
	gDraft.selectedPoi = -1;
	TrailToolsUberTool::Unlock();
	TrailToolsUberTool::FollowSelection();
	SetStatus("Selected #%d — Move to Feet, Delete Nearest, or drag with UberTool.", best);
}

void TrailToolsBinds::ActionTrailMoveToFeet()
{
	using namespace TrailToolsDetail;
	DraftTrail& tr = RecordingTrail();
	int& sel = RecordingSelectedPoint();
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	if (sel < 0 || sel >= static_cast<int>(tr.points.size()))
	{
		SetStatus("Select a point first.");
		return;
	}
	auto& pt = tr.points[static_cast<size_t>(sel)];
	if (pt.x == 0.f && pt.y == 0.f && pt.z == 0.f)
	{
		SetStatus("Cannot move a section break - pick a vector.");
		return;
	}
	TrailToolsEditUndo::PushTrail();
	pt.x = x;
	pt.y = y;
	pt.z = z;
	RecordingTrailDirty() = true;
	TrailToolsUberTool::FollowSelection();
	SetStatus("Moved #%d to feet.", sel);
}

void TrailToolsBinds::ActionTrailDeleteNearest()
{
	using namespace TrailToolsDetail;
	DraftTrail& tr = RecordingTrail();
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	const int best = NearestTrailPointIndex(x, y, z);
	if (best < 0)
	{
		SetStatus("No vectors to delete.");
		return;
	}
	TrailToolsEditUndo::PushTrail();
	tr.points.erase(tr.points.begin() + best);
	int& sel = RecordingSelectedPoint();
	if (sel == best)
		sel = -1;
	else if (sel > best)
		--sel;
	if (sel >= static_cast<int>(tr.points.size()))
		sel = static_cast<int>(tr.points.size()) - 1;
	RecordingTrailDirty() = true;
	SetStatus("Deleted nearest (#%d, %zu left).", best, tr.points.size());
}

void TrailToolsBinds::ActionMarkerSelectNearest()
{
	using namespace TrailToolsDetail;
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	int best = -1;
	float bestD = 1.e30f;
	for (int i = 0; i < static_cast<int>(gDraft.pois.size()); ++i)
	{
		const DraftPoi& p = gDraft.pois[static_cast<size_t>(i)];
		if (p.mapId != mapId)
			continue;
		const float dx = p.x - x, dy = p.y - y, dz = p.z - z;
		const float d = dx * dx + dy * dy + dz * dz;
		if (d < bestD)
		{
			bestD = d;
			best = i;
		}
	}
	if (best < 0)
	{
		SetStatus("No markers on this map.");
		return;
	}
	gDraft.selectedPoi = best;
	RecordingSelectedPoint() = -1;
	TrailToolsUberTool::SelectPoi(best);
	SetStatus("Selected marker #%d.", best);
}

void TrailToolsBinds::ActionMarkerMoveToFeet()
{
	using namespace TrailToolsDetail;
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	int sel = TrailToolsUberTool::ActivePoiIndex();
	if (sel < 0)
		sel = gDraft.selectedPoi;
	if (sel < 0 || sel >= static_cast<int>(gDraft.pois.size()))
	{
		SetStatus("Select a marker first.");
		return;
	}
	TrailToolsEditUndo::PushPois();
	DraftPoi& p = gDraft.pois[static_cast<size_t>(sel)];
	p.mapId = mapId;
	p.x = x;
	p.y = y;
	p.z = z;
	gDraft.selectedPoi = sel;
	TrailToolsUberTool::SelectPoi(sel);
	SetStatus("Moved marker #%d to feet.", sel);
}

#include "TrailToolsWorldPick.h"

#include "Globals.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdint>

bool TrailToolsWorldPick::RayFeetPlane(float& outX, float& outY, float& outZ)
{
	using namespace WorldGpsMath;
	if (!G::Mumble)
		return false;
	uint32_t mapId = 0;
	float fx = 0.f, fy = 0.f, fz = 0.f;
	if (!TrailToolsDetail::ReadMumblePose(mapId, fx, fy, fz))
		return false;

	const ImGuiIO& io = ImGui::GetIO();
	const float sw = io.DisplaySize.x;
	const float sh = io.DisplaySize.y;
	if (sw < 8.f || sh < 8.f)
		return false;

	const float* cp = G::Mumble->fCameraPosition;
	const float* cf = G::Mumble->fCameraFront;
	const float* ct = G::Mumble->fCameraTop;
	if (!ReasonablePos(cp[0], cp[1], cp[2]))
		return false;

	Vec3 cam{cp[0], cp[1], cp[2]};
	Vec3 f = Vec3{cf[0], cf[1], cf[2]}.Normalised();
	Vec3 topHint{ct[0], ct[1], ct[2]};
	Vec3 worldUp = (topHint.LengthSq() > 0.01f) ? topHint.Normalised() : Vec3{0.f, 1.f, 0.f};
	if (std::fabs(f.Dot(worldUp)) > 0.98f)
		worldUp = Vec3{0.f, 1.f, 0.f};
	Vec3 r = worldUp.Cross(f).Normalised();
	Vec3 u = f.Cross(r).Normalised();
	if (r.LengthSq() < 0.5f || u.LengthSq() < 0.5f)
		return false;

	const float fov = ParseFovRadians();
	const float aspect = sw / sh;
	const float tanHalf = std::tan(fov * 0.5f);
	const float ndcX = (io.MousePos.x / sw) * 2.f - 1.f;
	const float ndcY = 1.f - (io.MousePos.y / sh) * 2.f;
	Vec3 dir = (f + r * (ndcX * aspect * tanHalf) + u * (ndcY * tanHalf)).Normalised();
	if (std::fabs(dir.y) < 1e-4f)
		return false;
	const float t = (fy - cam.y) / dir.y;
	if (t < 0.5f || t > 4000.f)
		return false;
	outX = cam.x + dir.x * t;
	outY = fy;
	outZ = cam.z + dir.z * t;
	return ReasonablePos(outX, outY, outZ);
}

int TrailToolsWorldPick::NearestTrailPointScreen(float mx, float my, float maxPx)
{
	using namespace TrailToolsDetail;
	using namespace WorldGpsMath;
	const ImGuiIO& io = ImGui::GetIO();
	Mat4 vp{};
	Vec3 cam{};
	if (!BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		return -1;
	int best = -1;
	float bestD = maxPx * maxPx;
	for (int i = 0; i < static_cast<int>(gDraft.active.points.size()); ++i)
	{
		const auto& p = gDraft.active.points[static_cast<size_t>(i)];
		if (TrailToolsTrailGeom::IsBreak(p))
			continue;
		float sx = 0.f, sy = 0.f;
		if (!WorldToScreen({ p.x, p.y, p.z }, vp, io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			continue;
		const float dx = sx - mx, dy = sy - my;
		const float d = dx * dx + dy * dy;
		if (d < bestD)
		{
			bestD = d;
			best = i;
		}
	}
	return best;
}

int TrailToolsWorldPick::NearestPoiScreen(float mx, float my, float maxPx, uint32_t mapId)
{
	using namespace TrailToolsDetail;
	using namespace WorldGpsMath;
	const ImGuiIO& io = ImGui::GetIO();
	Mat4 vp{};
	Vec3 cam{};
	if (!BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		return -1;
	int best = -1;
	float bestD = maxPx * maxPx;
	for (int i = 0; i < static_cast<int>(gDraft.pois.size()); ++i)
	{
		const auto& p = gDraft.pois[static_cast<size_t>(i)];
		if (mapId && p.mapId != mapId)
			continue;
		float sx = 0.f, sy = 0.f;
		if (!WorldToScreen({ p.x, p.y, p.z }, vp, io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			continue;
		const float dx = sx - mx, dy = sy - my;
		const float d = dx * dx + dy * dy;
		if (d < bestD)
		{
			bestD = d;
			best = i;
		}
	}
	return best;
}

void TrailToolsWorldPick::Tick()
{
	using namespace TrailToolsDetail;
	if (!gWorldPickEnabled)
		return;
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		return;

	uint32_t mapId = 0;
	float fx = 0.f, fy = 0.f, fz = 0.f;
	const bool pose = ReadMumblePose(mapId, fx, fy, fz);

	if (gWorldPickMode == 2)
	{
		const int ti = NearestTrailPointScreen(io.MousePos.x, io.MousePos.y, 28.f);
		const int mi = NearestPoiScreen(io.MousePos.x, io.MousePos.y, 28.f, mapId);
		if (ti >= 0)
		{
			gDraft.selectedPoint = ti;
			SetStatus("Selected trail point #%d.", ti);
			return;
		}
		if (mi >= 0)
		{
			gDraft.selectedPoi = mi;
			SetStatus("Selected marker #%d.", mi);
			return;
		}
		SetStatus("Nothing under cursor.");
		return;
	}

	float hx = 0.f, hy = 0.f, hz = 0.f;
	if (!RayFeetPlane(hx, hy, hz))
	{
		SetStatus("Click pick missed feet plane.");
		return;
	}

	if (gWorldPickMode == 0)
	{
		if (!gDraft.markerType[0])
		{
			SetStatus("Set a marker type first.");
			return;
		}
		TrailToolsEditUndo::PushPois();
		DraftPoi p;
		p.mapId = mapId;
		p.x = hx;
		p.y = hy;
		p.z = hz;
		p.type = gDraft.markerType;
		p.guid = MakeGuidBase64();
		gDraft.pois.push_back(std::move(p));
		gDraft.selectedPoi = static_cast<int>(gDraft.pois.size()) - 1;
		SetStatus("Placed marker at click (%.1f, %.1f, %.1f).", hx, hy, hz);
		return;
	}

	/* mode 1 — add trail point */
	if (!pose)
	{
		SetStatus("No Mumble pose for trail map.");
		return;
	}
	TrailToolsEditUndo::PushTrail();
	DraftTrail& tr = RecordingTrail();
	if (tr.mapId == 0)
		tr.mapId = mapId;
	else if (tr.mapId != mapId)
	{
		SetStatus("Map mismatch - trail %u, you %u.", tr.mapId, mapId);
		return;
	}
	tr.points.push_back({ hx, hy, hz });
	RecordingSelectedPoint() = static_cast<int>(tr.points.size()) - 1;
	RecordingTrailDirty() = true;
	SetStatus("Trail point #%zu at click.", tr.points.size());
}

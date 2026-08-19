#include "TrailToolsWorldPick.h"

#include "Globals.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsGround.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdint>

bool TrailToolsWorldPick::CameraRay(WorldGpsMath::Vec3& cam, WorldGpsMath::Vec3& dir)
{
	const ImGuiIO& io = ImGui::GetIO();
	return CameraRayAt(io.MousePos.x, io.MousePos.y, cam, dir);
}

bool TrailToolsWorldPick::CameraRayAt(float mx, float my, WorldGpsMath::Vec3& cam, WorldGpsMath::Vec3& dir)
{
	using namespace WorldGpsMath;
	if (!G::Mumble)
		return false;
	const ImGuiIO& io = ImGui::GetIO();
	const float sw = io.DisplaySize.x;
	const float sh = io.DisplaySize.y;
	Mat4 vp{};
	Vec3 camOut{};
	if (!BuildViewProj(sw, sh, vp, camOut))
		return false;
	Vec3 dummy{};
	if (!ScreenRay(mx, my, sw, sh, vp, dummy, dir))
		return false;
	cam = camOut;
	return dir.LengthSq() > 0.5f;
}

bool TrailToolsWorldPick::RayPlaneY(float planeY, float& outX, float& outY, float& outZ)
{
	using namespace WorldGpsMath;
	Vec3 cam{}, dir{};
	if (!CameraRay(cam, dir))
		return false;
	if (std::fabs(dir.y) < 1e-4f)
		return false;
	const float t = (planeY - cam.y) / dir.y;
	if (t < 0.5f || t > 4000.f)
		return false;
	outX = cam.x + dir.x * t;
	outY = planeY;
	outZ = cam.z + dir.z * t;
	return ReasonablePos(outX, outY, outZ);
}

bool TrailToolsWorldPick::RayPlaneYAt(float planeY, float mx, float my,
	float& outX, float& outY, float& outZ)
{
	using namespace WorldGpsMath;
	Vec3 cam{}, dir{};
	if (!CameraRayAt(mx, my, cam, dir))
		return false;
	if (std::fabs(dir.y) < 1e-4f)
		return false;
	const float t = (planeY - cam.y) / dir.y;
	if (t < 0.5f || t > 4000.f)
		return false;
	outX = cam.x + dir.x * t;
	outY = planeY;
	outZ = cam.z + dir.z * t;
	return ReasonablePos(outX, outY, outZ);
}

bool TrailToolsWorldPick::RayFeetPlane(float& outX, float& outY, float& outZ)
{
	if (TrailToolsDetail::gGroundSnap && TrailToolsGround::RaySnap(outX, outY, outZ))
		return true;
	uint32_t mapId = 0;
	float fx = 0.f, fy = 0.f, fz = 0.f;
	if (!TrailToolsDetail::ReadMumblePose(mapId, fx, fy, fz))
		return false;
	return RayPlaneY(fy, outX, outY, outZ);
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
	DraftTrail& tr = RecordingTrail();
	for (int i = 0; i < static_cast<int>(tr.points.size()); ++i)
	{
		const auto& p = tr.points[static_cast<size_t>(i)];
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

#include "TrailToolsUberTool.h"
#include "TrailToolsUberToolInternal.h"

#include "Globals.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"
#include "TrailToolsWorldPick.h"
#include "WorldClick.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <windows.h>

namespace TrailToolsUberToolDetail
{
	State gSt;
}

namespace
{
	using namespace TrailToolsUberToolDetail;
	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;

	bool  gArmPlane = false;
	float gArmMx = 0.f, gArmMy = 0.f;

	void Mouse(float& mx, float& my)
	{
		const ImGuiIO& io = ImGui::GetIO();
		mx = io.MousePos.x;
		my = io.MousePos.y;
	}

	bool LButton()
	{
		return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	}

	float DistPtSeg2(float px, float py, float ax, float ay, float bx, float by, float& t)
	{
		const float abx = bx - ax, aby = by - ay;
		const float apx = px - ax, apy = py - ay;
		const float ab2 = abx * abx + aby * aby;
		t = (ab2 > 1e-8f) ? (apx * abx + apy * aby) / ab2 : 0.f;
		if (t < 0.f)
			t = 0.f;
		else if (t > 1.f)
			t = 1.f;
		const float qx = ax + abx * t, qy = ay + aby * t;
		const float dx = px - qx, dy = py - qy;
		return dx * dx + dy * dy;
	}
}

bool TrailToolsUberToolDetail::GetSel(WorldGpsMath::Vec3& o)
{
	using namespace TrailToolsDetail;
	if (gSt.kind == Kind::Trail)
	{
		DraftTrail& tr = RecordingTrail();
		if (gSt.index < 0 || gSt.index >= static_cast<int>(tr.points.size()))
			return false;
		const auto& p = tr.points[static_cast<size_t>(gSt.index)];
		if (TrailToolsTrailGeom::IsBreak(p))
			return false;
		o = { p.x, p.y, p.z };
		return true;
	}
	if (gSt.kind == Kind::Poi)
	{
		if (gSt.index < 0 || gSt.index >= static_cast<int>(gDraft.pois.size()))
			return false;
		const auto& p = gDraft.pois[static_cast<size_t>(gSt.index)];
		o = { p.x, p.y, p.z };
		return true;
	}
	return false;
}

void SetSel(const WorldGpsMath::Vec3& o)
{
	using namespace TrailToolsDetail;
	using namespace TrailToolsUberToolDetail;
	if (gSt.kind == Kind::Trail)
	{
		DraftTrail& tr = RecordingTrail();
		if (gSt.index < 0 || gSt.index >= static_cast<int>(tr.points.size()))
			return;
		auto& p = tr.points[static_cast<size_t>(gSt.index)];
		p.x = o.x;
		p.y = o.y;
		p.z = o.z;
		RecordingTrailDirty() = true;
		RecordingSelectedPoint() = gSt.index;
		return;
	}
	if (gSt.kind == Kind::Poi && gSt.index >= 0 &&
		gSt.index < static_cast<int>(gDraft.pois.size()))
	{
		auto& p = gDraft.pois[static_cast<size_t>(gSt.index)];
		p.x = o.x;
		p.y = o.y;
		p.z = o.z;
		gDraft.selectedPoi = gSt.index;
	}
}

float TrailToolsUberToolDetail::GizmoLen(const WorldGpsMath::Vec3& origin)
{
	using WorldGpsMath::Vec3;
	if (!G::Mumble)
		return 8.f;
	Vec3 cam{
		G::Mumble->fCameraPosition[0],
		G::Mumble->fCameraPosition[1],
		G::Mumble->fCameraPosition[2]
	};
	const float dist = std::sqrt((origin - cam).LengthSq());
	return std::clamp(dist * 0.22f, 4.f, 14.f);
}

int TrailToolsUberToolDetail::HitAxis(const WorldGpsMath::Vec3& origin, const WorldGpsMath::Mat4& vp,
	float sw, float sh, float mx, float my)
{
	using WorldGpsMath::Vec3;
	const float len = GizmoLen(origin);
	float ox = 0.f, oy = 0.f;
	if (!WorldGpsMath::WorldToScreen(origin, vp, sw, sh, ox, oy))
		return 0;
	int best = 0;
	float bestD = 40.f * 40.f;
	const Axis axes[3] = { Axis::X, Axis::Y, Axis::Z };
	for (int i = 0; i < 3; ++i)
	{
		const Vec3 tip = origin + AxisDir(axes[i]) * len;
		float tx = 0.f, ty = 0.f;
		if (!WorldGpsMath::WorldToScreen(tip, vp, sw, sh, tx, ty))
			continue;
		float t = 0.f;
		float d = DistPtSeg2(mx, my, ox, oy, tx, ty, t);
		const float tdx = tx - mx, tdy = ty - my;
		const float tipD = tdx * tdx + tdy * tdy;
		if (tipD < d)
			d = tipD;
		if (d < bestD)
		{
			bestD = d;
			best = static_cast<int>(axes[i]);
		}
	}
	return best;
}

namespace
{
	using namespace TrailToolsUberToolDetail;
	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;

	bool AxisT(const Vec3& origin, const Vec3& axis, const Vec3& cam, const Vec3& ray, float& t)
	{
		const float ab = axis.Dot(ray);
		const float den = 1.f - ab * ab;
		if (den < 1e-6f)
			return false;
		const Vec3 w = origin - cam;
		t = (w.Dot(axis) - w.Dot(ray) * ab) / den;
		return true;
	}

	bool ClosestTrailSeg(float mx, float my, const Mat4& vp, float sw, float sh,
		int& outAfter, float& outT, float maxPx)
	{
		using namespace TrailToolsDetail;
		DraftTrail& tr = RecordingTrail();
		outAfter = -1;
		float best = maxPx * maxPx;
		for (int i = 0; i + 1 < static_cast<int>(tr.points.size()); ++i)
		{
			const auto& a = tr.points[static_cast<size_t>(i)];
			const auto& b = tr.points[static_cast<size_t>(i + 1)];
			if (TrailToolsTrailGeom::IsBreak(a) || TrailToolsTrailGeom::IsBreak(b))
				continue;
			float ax = 0.f, ay = 0.f, bx = 0.f, by = 0.f;
			if (!WorldGpsMath::WorldToScreen({ a.x, a.y, a.z }, vp, sw, sh, ax, ay))
				continue;
			if (!WorldGpsMath::WorldToScreen({ b.x, b.y, b.z }, vp, sw, sh, bx, by))
				continue;
			float t = 0.f;
			const float d = DistPtSeg2(mx, my, ax, ay, bx, by, t);
			if (d < best)
			{
				best = d;
				outAfter = i;
				outT = t;
			}
		}
		return outAfter >= 0;
	}

	void CancelDrag()
	{
		if (gSt.drag == Axis::None)
			return;
		SetSel({ gSt.origX, gSt.origY, gSt.origZ });
		gSt.drag = Axis::None;
		gSt.pushed = false;
		gArmPlane = false;
		TrailToolsDetail::SetStatus("UberTool: move cancelled.");
	}

	void BeginDrag(Axis ax, const Vec3& origin, float mx, float my)
	{
		using namespace TrailToolsDetail;
		gArmPlane = false;
		gSt.drag = ax;
		gSt.origX = origin.x;
		gSt.origY = origin.y;
		gSt.origZ = origin.z;
		gSt.grabMx = mx;
		gSt.grabMy = my;
		if (!gSt.pushed)
		{
			if (gSt.kind == Kind::Trail)
				TrailToolsEditUndo::PushTrail();
			else
				TrailToolsEditUndo::PushPois();
			gSt.pushed = true;
		}
		Vec3 cam{}, ray{};
		if (ax != Axis::Plane && TrailToolsWorldPick::CameraRayAt(mx, my, cam, ray))
			AxisT(origin, AxisDir(ax), cam, ray, gSt.axisT0);
	}

	void TickDrag()
	{
		using namespace TrailToolsDetail;
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
			ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			CancelDrag();
			return;
		}
		if (!LButton())
		{
			gSt.drag = Axis::None;
			gSt.pushed = false;
			return;
		}
		float mx = 0.f, my = 0.f;
		Mouse(mx, my);
		/* World overlay is correct; the pick ray is mirrored vs the cursor. */
		mx = 2.f * gSt.grabMx - mx;
		my = 2.f * gSt.grabMy - my;
		if (gSt.drag == Axis::Plane)
		{
			float hx = 0.f, hy = 0.f, hz = 0.f;
			if (!TrailToolsWorldPick::RayPlaneYAt(gSt.origY, mx, my, hx, hy, hz))
				return;
			SetSel({ hx, gSt.origY, hz });
			return;
		}
		Vec3 origin{};
		if (!GetSel(origin))
		{
			gSt.drag = Axis::None;
			return;
		}
		Vec3 cam{}, ray{};
		if (!TrailToolsWorldPick::CameraRayAt(mx, my, cam, ray))
			return;
		float t = 0.f;
		if (!AxisT({ gSt.origX, gSt.origY, gSt.origZ }, AxisDir(gSt.drag), cam, ray, t))
			return;
		const Vec3 pos = Vec3{ gSt.origX, gSt.origY, gSt.origZ } +
			AxisDir(gSt.drag) * (t - gSt.axisT0);
		SetSel(pos);
	}

	void ArmPlane(float mx, float my)
	{
		gArmPlane = true;
		gArmMx = mx;
		gArmMy = my;
	}

	void MaybeStartPlaneDrag()
	{
		if (!gArmPlane || gSt.drag != Axis::None)
			return;
		if (!LButton())
		{
			gArmPlane = false;
			return;
		}
		float mx = 0.f, my = 0.f;
		Mouse(mx, my);
		const float dx = mx - gArmMx, dy = my - gArmMy;
		if (dx * dx + dy * dy < 64.f)
			return;
		Vec3 o{};
		if (!GetSel(o))
		{
			gArmPlane = false;
			return;
		}
		BeginDrag(Axis::Plane, o, mx, my);
	}

	void TrySelectOrInsert(float mx, float my)
	{
		using namespace TrailToolsDetail;
		const ImGuiIO& io = ImGui::GetIO();
		Mat4 vp{};
		Vec3 cam{};
		if (!WorldGpsMath::BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
			return;
		uint32_t mapId = 0;
		float fx = 0.f, fy = 0.f, fz = 0.f;
		ReadMumblePose(mapId, fx, fy, fz);

		Vec3 origin{};
		if (GetSel(origin))
		{
			const int hit = HitAxis(origin, vp, io.DisplaySize.x, io.DisplaySize.y, mx, my);
			if (hit != 0)
			{
				BeginDrag(static_cast<Axis>(hit), origin, mx, my);
				return;
			}
		}

		const int ti = TrailToolsWorldPick::NearestTrailPointScreen(mx, my, 48.f);
		const int mi = TrailToolsWorldPick::NearestPoiScreen(mx, my, 32.f, mapId);

		if (io.KeyCtrl)
		{
			int after = -1;
			float t = 0.f;
			if (ClosestTrailSeg(mx, my, vp, io.DisplaySize.x, io.DisplaySize.y, after, t, 16.f))
			{
				DraftTrail& tr = RecordingTrail();
				const auto& a = tr.points[static_cast<size_t>(after)];
				const auto& b = tr.points[static_cast<size_t>(after + 1)];
				PathingTrails::WorldPoint np;
				np.x = a.x + (b.x - a.x) * t;
				np.y = a.y + (b.y - a.y) * t;
				np.z = a.z + (b.z - a.z) * t;
				TrailToolsEditUndo::PushTrail();
				tr.points.insert(tr.points.begin() + after + 1, np);
				gSt.kind = Kind::Trail;
				gSt.index = after + 1;
				RecordingSelectedPoint() = gSt.index;
				RecordingTrailDirty() = true;
				SetStatus("UberTool: inserted trail point #%d.", gSt.index);
				return;
			}
		}

		if (ti < 0 && mi < 0)
		{
			gSt.kind = Kind::None;
			gSt.index = -1;
			gArmPlane = false;
			return;
		}
		float dt = 1e12f, dm = 1e12f;
		if (ti >= 0)
		{
			DraftTrail& tr = RecordingTrail();
			float sx = 0.f, sy = 0.f;
			const auto& p = tr.points[static_cast<size_t>(ti)];
			if (WorldGpsMath::WorldToScreen({ p.x, p.y, p.z }, vp,
				io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			{
				const float dx = sx - mx, dy = sy - my;
				dt = dx * dx + dy * dy;
			}
		}
		if (mi >= 0)
		{
			const auto& p = gDraft.pois[static_cast<size_t>(mi)];
			float sx = 0.f, sy = 0.f;
			if (WorldGpsMath::WorldToScreen({ p.x, p.y, p.z }, vp,
				io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			{
				const float dx = sx - mx, dy = sy - my;
				dm = dx * dx + dy * dy;
			}
		}
		if (dt <= dm)
		{
			gSt.kind = Kind::Trail;
			gSt.index = ti;
			RecordingSelectedPoint() = ti;
			gDraft.selectedPoi = -1;
			ArmPlane(mx, my);
			SetStatus("UberTool: trail #%d — drag to slide, arrows for XYZ.", ti);
		}
		else
		{
			gSt.kind = Kind::Poi;
			gSt.index = mi;
			gDraft.selectedPoi = mi;
			ArmPlane(mx, my);
			SetStatus("UberTool: marker #%d — drag to slide, arrows for XYZ.", mi);
		}
	}
}

bool TrailToolsUberTool::WantSwallow(float mx, float my)
{
	using namespace TrailToolsDetail;
	using namespace TrailToolsUberToolDetail;
	if (!gUberToolEnabled || !AnyAuthoringPadOpen())
		return false;
	if (gSt.drag != Axis::None || gArmPlane)
		return true;
	const ImGuiIO& io = ImGui::GetIO();
	Mat4 vp{};
	Vec3 cam{};
	if (!WorldGpsMath::BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		return false;
	Vec3 origin{};
	if (GetSel(origin) &&
		HitAxis(origin, vp, io.DisplaySize.x, io.DisplaySize.y, mx, my) != 0)
		return true;
	if (TrailToolsWorldPick::NearestTrailPointScreen(mx, my, 48.f) >= 0)
		return true;
	uint32_t mapId = 0;
	float fx = 0.f, fy = 0.f, fz = 0.f;
	ReadMumblePose(mapId, fx, fy, fz);
	return TrailToolsWorldPick::NearestPoiScreen(mx, my, 32.f, mapId) >= 0;
}

void TrailToolsUberTool::FollowSelection()
{
	using namespace TrailToolsDetail;
	using namespace TrailToolsUberToolDetail;
	if (gSt.drag != Axis::None)
		return;
	const int ts = RecordingSelectedPoint();
	if (ts >= 0)
	{
		gSt.kind = Kind::Trail;
		gSt.index = ts;
		return;
	}
	if (gDraft.selectedPoi >= 0)
	{
		gSt.kind = Kind::Poi;
		gSt.index = gDraft.selectedPoi;
		return;
	}
	gSt.kind = Kind::None;
	gSt.index = -1;
}

bool TrailToolsUberTool::Tick()
{
	using namespace TrailToolsDetail;
	using namespace TrailToolsUberToolDetail;
	if (!gUberToolEnabled || !AnyAuthoringPadOpen())
		return false;
	if (gSt.drag != Axis::None)
	{
		TickDrag();
		return true;
	}
	FollowSelection();
	MaybeStartPlaneDrag();
	if (gSt.drag != Axis::None || gArmPlane)
		return true;
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return false;
	float mx = io.MousePos.x, my = io.MousePos.y;
	float wmx = 0.f, wmy = 0.f;
	const bool worldClick = WorldClick::TakeLeftDown(wmx, wmy);
	if (worldClick)
	{
		mx = wmx;
		my = wmy;
	}
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !worldClick)
		return false;
	TrySelectOrInsert(mx, my);
	return true;
}

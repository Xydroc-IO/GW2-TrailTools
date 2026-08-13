#include "TrailToolsUberTool.h"

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

namespace
{
	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;

	enum class Kind { None, Trail, Poi };
	enum class Axis { None, X, Y, Z };

	struct State
	{
		Kind kind = Kind::None;
		int  index = -1;
		Axis drag = Axis::None;
		bool pushed = false;
		float origX = 0.f, origY = 0.f, origZ = 0.f;
		float axisT0 = 0.f;
	};
	State gSt;

	Vec3 AxisDir(Axis a)
	{
		if (a == Axis::X)
			return { 1.f, 0.f, 0.f };
		if (a == Axis::Y)
			return { 0.f, 1.f, 0.f };
		return { 0.f, 0.f, 1.f };
	}

	bool GetSel(Vec3& o)
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

	void SetSel(const Vec3& o)
	{
		using namespace TrailToolsDetail;
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

	/* Closest t on axis (origin + t*dir) to camera ray. */
	bool AxisT(const Vec3& origin, const Vec3& axis, const Vec3& cam, const Vec3& ray, float& t)
	{
		const Vec3 n = axis.Cross(ray);
		const float n2 = n.LengthSq();
		if (n2 < 1e-8f)
			return false;
		const Vec3 w0 = origin - cam;
		t = w0.Cross(ray).Dot(n) / n2;
		return true;
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

	int HitAxis(const Vec3& origin, const Mat4& vp, float sw, float sh, float mx, float my)
	{
		using namespace TrailToolsDetail;
		Vec3 cam{};
		if (!G::Mumble)
			return 0;
		cam = { G::Mumble->fCameraPosition[0], G::Mumble->fCameraPosition[1],
			G::Mumble->fCameraPosition[2] };
		const float dist = std::sqrt((origin - cam).LengthSq());
		const float len = std::clamp(dist * 0.12f, 1.2f, 8.f);
		float ox = 0.f, oy = 0.f;
		if (!WorldGpsMath::WorldToScreen(origin, vp, sw, sh, ox, oy))
			return 0;
		int best = 0;
		float bestD = 12.f * 12.f;
		const Axis axes[3] = { Axis::X, Axis::Y, Axis::Z };
		for (int i = 0; i < 3; ++i)
		{
			const Vec3 tip = origin + AxisDir(axes[i]) * len;
			float tx = 0.f, ty = 0.f;
			if (!WorldGpsMath::WorldToScreen(tip, vp, sw, sh, tx, ty))
				continue;
			float t = 0.f;
			const float d = DistPtSeg2(mx, my, ox, oy, tx, ty, t);
			if (d < bestD)
			{
				bestD = d;
				best = static_cast<int>(axes[i]);
			}
		}
		return best;
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
		TrailToolsDetail::SetStatus("UberTool: move cancelled.");
	}

	void BeginDrag(Axis ax, const Vec3& origin)
	{
		using namespace TrailToolsDetail;
		gSt.drag = ax;
		gSt.origX = origin.x;
		gSt.origY = origin.y;
		gSt.origZ = origin.z;
		if (!gSt.pushed)
		{
			if (gSt.kind == Kind::Trail)
				TrailToolsEditUndo::PushTrail();
			else
				TrailToolsEditUndo::PushPois();
			gSt.pushed = true;
		}
		Vec3 cam{}, ray{};
		if (TrailToolsWorldPick::CameraRay(cam, ray))
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
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			gSt.drag = Axis::None;
			gSt.pushed = false;
			return;
		}
		Vec3 origin{};
		if (!GetSel(origin))
		{
			gSt.drag = Axis::None;
			return;
		}
		Vec3 cam{}, ray{};
		if (!TrailToolsWorldPick::CameraRay(cam, ray))
			return;
		float t = 0.f;
		if (!AxisT({ gSt.origX, gSt.origY, gSt.origZ }, AxisDir(gSt.drag), cam, ray, t))
			return;
		const Vec3 pos = Vec3{ gSt.origX, gSt.origY, gSt.origZ } +
			AxisDir(gSt.drag) * (t - gSt.axisT0);
		SetSel(pos);
	}

	void TrySelectOrInsert()
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
			const int hit = HitAxis(origin, vp, io.DisplaySize.x, io.DisplaySize.y,
				io.MousePos.x, io.MousePos.y);
			if (hit != 0)
			{
				BeginDrag(static_cast<Axis>(hit), origin);
				return;
			}
		}

		const int ti = TrailToolsWorldPick::NearestTrailPointScreen(
			io.MousePos.x, io.MousePos.y, 22.f);
		const int mi = TrailToolsWorldPick::NearestPoiScreen(
			io.MousePos.x, io.MousePos.y, 22.f, mapId);

		if (io.KeyCtrl)
		{
			int after = -1;
			float t = 0.f;
			if (ClosestTrailSeg(io.MousePos.x, io.MousePos.y, vp,
				io.DisplaySize.x, io.DisplaySize.y, after, t, 16.f))
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
			return;
		}
		/* Prefer whichever is closer in pixels. */
		float dt = 1e12f, dm = 1e12f;
		if (ti >= 0)
		{
			DraftTrail& tr = RecordingTrail();
			float sx = 0.f, sy = 0.f;
			const auto& p = tr.points[static_cast<size_t>(ti)];
			if (WorldGpsMath::WorldToScreen({ p.x, p.y, p.z }, vp,
				io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			{
				const float dx = sx - io.MousePos.x, dy = sy - io.MousePos.y;
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
				const float dx = sx - io.MousePos.x, dy = sy - io.MousePos.y;
				dm = dx * dx + dy * dy;
			}
		}
		if (dt <= dm)
		{
			gSt.kind = Kind::Trail;
			gSt.index = ti;
			RecordingSelectedPoint() = ti;
			gDraft.selectedPoi = -1;
			SetStatus("UberTool: trail point #%d.", ti);
		}
		else
		{
			gSt.kind = Kind::Poi;
			gSt.index = mi;
			gDraft.selectedPoi = mi;
			SetStatus("UberTool: marker #%d.", mi);
		}
	}

	void DrawGizmo(ImDrawList* dl, const Mat4& vp, float sw, float sh, const Vec3& origin)
	{
		Vec3 cam{
			G::Mumble->fCameraPosition[0],
			G::Mumble->fCameraPosition[1],
			G::Mumble->fCameraPosition[2]
		};
		const float dist = std::sqrt((origin - cam).LengthSq());
		const float len = std::clamp(dist * 0.12f, 1.2f, 8.f);
		float ox = 0.f, oy = 0.f;
		if (!WorldGpsMath::WorldToScreen(origin, vp, sw, sh, ox, oy))
			return;
		const ImU32 cols[3] = { IM_COL32(220, 50, 50, 255), IM_COL32(50, 200, 70, 255),
			IM_COL32(50, 90, 230, 255) };
		const Axis axes[3] = { Axis::X, Axis::Y, Axis::Z };
		for (int i = 0; i < 3; ++i)
		{
			const Vec3 tip = origin + AxisDir(axes[i]) * len;
			float tx = 0.f, ty = 0.f;
			if (!WorldGpsMath::WorldToScreen(tip, vp, sw, sh, tx, ty))
				continue;
			const float thick = (gSt.drag == axes[i]) ? 4.f : 2.5f;
			dl->AddLine(ImVec2(ox, oy), ImVec2(tx, ty), cols[i], thick);
			dl->AddCircleFilled(ImVec2(tx, ty), 5.f, cols[i]);
		}
		dl->AddCircleFilled(ImVec2(ox, oy), 4.f, IM_COL32(255, 255, 255, 230));
	}

	void DrawRangeRing(ImDrawList* dl, const Mat4& vp, float sw, float sh, const Vec3& o, float r)
	{
		if (r < 0.15f)
			return;
		ImVec2 pts[33];
		int n = 0;
		for (int i = 0; i <= 32; ++i)
		{
			const float a = (static_cast<float>(i) / 32.f) * 6.2831853f;
			const Vec3 w{ o.x + std::cos(a) * r, o.y, o.z + std::sin(a) * r };
			float sx = 0.f, sy = 0.f;
			if (!WorldGpsMath::WorldToScreen(w, vp, sw, sh, sx, sy))
				continue;
			pts[n++] = ImVec2(sx, sy);
		}
		if (n >= 3)
			dl->AddPolyline(pts, n, IM_COL32(80, 200, 255, 180), true, 1.5f);
	}
}

bool TrailToolsUberTool::Tick()
{
	using namespace TrailToolsDetail;
	if (!gUberToolEnabled || !AnyAuthoringPadOpen())
		return false;
	const ImGuiIO& io = ImGui::GetIO();
	if (gSt.drag != Axis::None)
	{
		TickDrag();
		return true;
	}
	if (io.WantCaptureMouse)
		return false;
	float wmx = 0.f, wmy = 0.f;
	const bool worldClick = WorldClick::TakeLeftDown(wmx, wmy);
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !worldClick)
		return false;
	TrySelectOrInsert();
	return true; /* consume so Live world-click does not also fire */
}

void TrailToolsUberTool::Render()
{
	using namespace TrailToolsDetail;
	if (!gUberToolEnabled || !AnyAuthoringPadOpen() || !G::Mumble || G::Mumble->uiTick == 0)
		return;
	if (gSt.kind == Kind::None)
	{
		if (RecordingSelectedPoint() >= 0)
		{
			gSt.kind = Kind::Trail;
			gSt.index = RecordingSelectedPoint();
		}
		else if (gDraft.selectedPoi >= 0)
		{
			gSt.kind = Kind::Poi;
			gSt.index = gDraft.selectedPoi;
		}
	}
	const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
	if (!ctx || ctx->mapId == 0)
		return;
	if (G::HideWhenMapOpen && (ctx->uiState & static_cast<uint32_t>(UiStateBits::MapOpen)))
		return;
	if (G::HideOutOfGameplay && G::NexusLink && !G::NexusLink->IsGameplay)
		return;

	Vec3 origin{};
	if (!GetSel(origin))
		return;
	if (gSt.kind == Kind::Trail)
	{
		DraftTrail& tr = RecordingTrail();
		if (tr.mapId != 0 && tr.mapId != ctx->mapId)
			return;
	}
	else if (gSt.kind == Kind::Poi)
	{
		if (gDraft.pois[static_cast<size_t>(gSt.index)].mapId != ctx->mapId)
			return;
	}

	const ImGuiIO& io = ImGui::GetIO();
	Mat4 vp{};
	Vec3 cam{};
	if (!WorldGpsMath::BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		return;
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;
	DrawGizmo(dl, vp, io.DisplaySize.x, io.DisplaySize.y, origin);
	if (gSt.kind == Kind::Poi)
	{
		const auto& p = gDraft.pois[static_cast<size_t>(gSt.index)];
		if (p.triggerRange > 0.15f)
			DrawRangeRing(dl, vp, io.DisplaySize.x, io.DisplaySize.y, origin, p.triggerRange);
	}
}

#include "TrailToolsUberTool.h"
#include "TrailToolsUberToolInternal.h"

#include "Globals.h"
#include "TrailToolsShared.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdint>

void TrailToolsUberTool::Render()
{
	using namespace TrailToolsDetail;
	using namespace TrailToolsUberToolDetail;
	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;

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

	const float len = GizmoLen(origin);
	float ox = 0.f, oy = 0.f;
	if (!WorldGpsMath::WorldToScreen(origin, vp, io.DisplaySize.x, io.DisplaySize.y, ox, oy))
		return;
	const ImU32 cols[3] = { IM_COL32(255, 56, 56, 255), IM_COL32(48, 220, 80, 255),
		IM_COL32(56, 110, 255, 255) };
	const Axis axes[3] = { Axis::X, Axis::Y, Axis::Z };
	for (int i = 0; i < 3; ++i)
	{
		const Vec3 tip = origin + AxisDir(axes[i]) * len;
		float tx = 0.f, ty = 0.f;
		if (!WorldGpsMath::WorldToScreen(tip, vp,
			io.DisplaySize.x, io.DisplaySize.y, tx, ty))
			continue;
		const ImVec2 a(ox, oy);
		const float dx = tx - ox, dy = ty - oy;
		const float sl = std::sqrt(dx * dx + dy * dy);
		if (sl < 8.f)
			continue;
		const float ux = dx / sl, uy = dy / sl;
		const float px = -uy, py = ux;
		const float hs = 36.f, hw = 19.f;
		const ImVec2 shaft(tx - ux * hs, ty - uy * hs);
		const float thick = (gSt.drag == axes[i]) ? 20.f : 16.f;
		dl->AddLine(a, shaft, IM_COL32(0, 0, 0, 220), thick + 6.f);
		dl->AddLine(a, shaft, cols[i], thick);
		const ImVec2 t0(tx, ty);
		const ImVec2 t1(tx - ux * hs + px * hw, ty - uy * hs + py * hw);
		const ImVec2 t2(tx - ux * hs - px * hw, ty - uy * hs - py * hw);
		dl->AddTriangleFilled(t0, t1, t2, cols[i]);
		dl->AddTriangle(t0, t1, t2, IM_COL32(0, 0, 0, 230), 2.4f);
	}
	dl->AddCircleFilled(ImVec2(ox, oy), 12.f, IM_COL32(255, 255, 255, 240));
	dl->AddCircle(ImVec2(ox, oy), 13.5f, IM_COL32(0, 0, 0, 200), 16, 2.4f);
}

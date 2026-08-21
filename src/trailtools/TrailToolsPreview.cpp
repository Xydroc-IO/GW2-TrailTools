#include "TrailToolsPreview.h"

#include "TrailToolsDraftStyle.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"
#include "TrailToolsBinds.h"
#include "Globals.h"
#include "WorldGpsD3d.h"
#include "WorldGpsImgui.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
	bool SlotPreviewVisible(const TrailToolsDetail::TrailEditorSlot& s, int slot)
	{
		if (!s.open || s.trail.points.size() < 2)
			return false;
		if (s.worldShown)
			return true;
		return TrailToolsBinds::Get().trailRecording &&
			TrailToolsDetail::gTrailRecordSlot == slot;
	}

	void CollectPreviewSnippets(uint32_t mapId,
		std::vector<PathingTrails::WorldSnippet>& out)
	{
		using namespace TrailToolsDetail;
		out.clear();
		if (gDraft.previewAllTrails)
		{
			for (int i = 0; i < kMaxTrailEditors; ++i)
			{
				const TrailEditorSlot& s = gTrailEditors[i];
				if (!SlotPreviewVisible(s, i) || s.trail.mapId != mapId)
					continue;
				PathingTrails::WorldSnippet snip =
					TrailToolsDraftStyle::BuildSnippetFor(s.trail);
				if (snip.points.size() >= 2)
					out.push_back(std::move(snip));
			}
			return;
		}
		if (!DraftWorldVisible())
			return;
		/* Prefer focused TrailsN so window switches update GPS without waiting to walk. */
		int slot = gTrailFocusSlot;
		if (slot < 0 || slot >= kMaxTrailEditors || !gTrailEditors[slot].open)
			slot = gTrailRecordSlot;
		if (slot >= 0 && slot < kMaxTrailEditors && gTrailEditors[slot].open)
		{
			const DraftTrail& tr = gTrailEditors[slot].trail;
			if (tr.points.size() >= 2 && tr.mapId == mapId)
			{
				PathingTrails::WorldSnippet snip =
					TrailToolsDraftStyle::BuildSnippetFor(tr);
				if (snip.points.size() >= 2)
					out.push_back(std::move(snip));
				return;
			}
		}
		const DraftTrail& rec = RecordingTrail();
		if (rec.points.size() < 2 || rec.mapId != mapId)
			return;
		PathingTrails::WorldSnippet snip = TrailToolsDraftStyle::BuildActiveSnippet();
		if (snip.points.size() >= 2)
			out.push_back(std::move(snip));
	}
}

void TrailToolsPreview::RenderWorld()
{
	using namespace TrailToolsDetail;
	if (!TrailToolsDetail::AnyAuthoringPadOpen() || !gDraft.previewEnabled)
		return;
	if (!G::Mumble || G::Mumble->uiTick == 0)
		return;
	const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
	if (!ctx || ctx->mapId == 0)
		return;
	if (G::HideWhenMapOpen && (ctx->uiState & static_cast<uint32_t>(UiStateBits::MapOpen)))
		return;
	if (G::HideOutOfGameplay && G::NexusLink && !G::NexusLink->IsGameplay)
		return;

	TrailToolsDraftStyle::BeginFrame();

	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;
	const ImGuiIO& io = ImGui::GetIO();
	const float screenW = io.DisplaySize.x;
	const float screenH = io.DisplaySize.y;
	Mat4 viewProj{};
	Vec3 cam{};
	if (!WorldGpsMath::BuildViewProj(screenW, screenH, viewProj, cam))
		return;

	const Vec3 avatar{
		G::Mumble->fAvatarPosition[0],
		G::Mumble->fAvatarPosition[1],
		G::Mumble->fAvatarPosition[2]
	};
	const float thickness = std::clamp(G::WorldTrailWidth, 0.15f, 4.f);
	const float maxDist = std::max(80.f, G::WorldTrailMaxDist);

	std::vector<PathingTrails::WorldSnippet> snips;
	CollectPreviewSnippets(ctx->mapId, snips);
	if (!snips.empty())
	{
		bool drewD3d = false;
		if (WorldGpsD3d::Available())
			drewD3d = WorldGpsD3d::DrawTrails(
				viewProj, cam, avatar, maxDist, thickness, snips, nullptr);
		if (!drewD3d)
		{
			ImDrawList* dl = ImGui::GetBackgroundDrawList();
			if (dl)
			{
				for (const auto& snip : snips)
				{
					WorldGpsImgui::DrawTrailBillboards(
						dl, viewProj, screenW, screenH, avatar, snip,
						maxDist, thickness, WorldGpsMath::kMaxSegments, true);
				}
			}
		}
	}

	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;
	std::vector<PathingTrails::Marker> marks;
	marks.reserve(gDraft.pois.size());
	for (const DraftPoi& p : gDraft.pois)
	{
		if (p.mapId != ctx->mapId)
			continue;
		marks.push_back(TrailToolsDraftStyle::BuildDraftMarker(p));
	}
	if (!marks.empty())
		WorldGpsImgui::DrawMarkers(dl, viewProj, screenW, screenH, avatar, marks);

	/* Vertex pick handles stay on the active / recording trail only. */
	if (!DraftWorldVisible() && snips.empty())
		return;
	const DraftTrail& rec = RecordingTrail();
	if (rec.mapId != 0 && rec.mapId != ctx->mapId)
		return;
	if (rec.points.size() < 2)
		return;

	int& sel = RecordingSelectedPoint();
	int bestHit = -1;
	float bestD = 22.f * 22.f;
	for (int i = 0; i < static_cast<int>(rec.points.size()); ++i)
	{
		const auto& p = rec.points[static_cast<size_t>(i)];
		if (TrailToolsTrailGeom::IsBreak(p))
			continue;
		float sx = 0.f, sy = 0.f;
		if (!WorldGpsMath::WorldToScreen({ p.x, p.y, p.z }, viewProj, screenW, screenH, sx, sy))
			continue;
		const float r = (sel == i) ? 13.f : 10.f;
		const ImU32 fill = (sel == i) ? IM_COL32(80, 220, 255, 230) : IM_COL32(255, 255, 255, 215);
		dl->AddCircleFilled(ImVec2(sx, sy), r, fill, 20);
		dl->AddCircle(ImVec2(sx, sy), r + 1.4f, IM_COL32(20, 20, 24, 230), 20, 1.8f);
		if (!io.WantCaptureMouse)
		{
			const float dx = sx - io.MousePos.x, dy = sy - io.MousePos.y;
			const float d = dx * dx + dy * dy;
			if (d < bestD)
			{
				bestD = d;
				bestHit = i;
			}
		}
	}

	if (gUberToolEnabled || bestHit < 0 || io.WantCaptureMouse)
		return;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		sel = bestHit;
		SetStatus("Selected #%d — Move to Feet, Delete Nearest, or enable UberTool to drag.",
			bestHit);
	}
}

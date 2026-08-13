#include "TrailToolsPreview.h"

#include "TrailToolsDraftStyle.h"
#include "TrailToolsShared.h"
#include "Globals.h"
#include "WorldGpsD3d.h"
#include "WorldGpsImgui.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <vector>

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
	const float thickness = std::clamp(G::WorldTrailWidth, 0.5f, 4.f);
	const float maxDist = std::max(80.f, G::WorldTrailMaxDist);

	const DraftTrail& rec = RecordingTrail();
	if (rec.points.size() >= 2 && rec.mapId == ctx->mapId)
	{
		PathingTrails::WorldSnippet snip = TrailToolsDraftStyle::BuildActiveSnippet();
		if (snip.points.size() >= 2)
		{
			bool drewD3d = false;
			if (WorldGpsD3d::Available())
			{
				std::vector<PathingTrails::WorldSnippet> one{ snip };
				drewD3d = WorldGpsD3d::DrawTrails(
					viewProj, cam, avatar, maxDist, thickness, one, nullptr);
			}
			if (!drewD3d)
			{
				ImDrawList* dl = ImGui::GetBackgroundDrawList();
				if (dl)
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
}

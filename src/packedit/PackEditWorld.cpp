#include "PackEditInternal.h"

#include "Globals.h"
#include "TrailToolsShared.h"
#include "WorldGpsD3d.h"
#include "WorldGpsImgui.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

void PackEdit::RenderWorld()
{
	if (!gDoc.worldDraw || !G::Mumble || G::Mumble->uiTick == 0)
		return;
	const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
	if (!ctx || ctx->mapId == 0)
		return;
	if (G::HideWhenMapOpen && (ctx->uiState & static_cast<uint32_t>(UiStateBits::MapOpen)))
		return;

	using WorldGpsMath::Mat4;
	using WorldGpsMath::Vec3;
	const ImGuiIO& io = ImGui::GetIO();
	Mat4 vp{};
	Vec3 cam{};
	if (!WorldGpsMath::BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		return;
	const Vec3 avatar{
		G::Mumble->fAvatarPosition[0],
		G::Mumble->fAvatarPosition[1],
		G::Mumble->fAvatarPosition[2]
	};
	const float maxDist = std::max(80.f, G::WorldTrailMaxDist);
	const float thick = std::clamp(G::WorldTrailWidth, 0.5f, 4.f);

	std::vector<PathingTrails::WorldSnippet> snips;
	std::vector<PathingTrails::Marker> marks;
	snips.reserve(32);
	marks.reserve(256);
	int nMark = 0, nTrail = 0;
	for (const auto& it : gDoc.items)
	{
		if (it.tombstone || CategoryHidden(it.type))
			continue;
		if (gDoc.thisMapOnly && it.mapId != 0 && it.mapId != ctx->mapId)
			continue;
		const auto st = EffectiveStyle(it);
		if (it.isTrail)
		{
			if (it.points.size() < 2 || nTrail >= 128)
				continue;
			PathingTrails::WorldSnippet s;
			s.color = st.hasColor ? st.color : 0xFFFFFFFFu;
			s.alpha = st.hasAlpha ? st.alpha : 1.f;
			s.trailScale = st.hasTrailScale ? st.trailScale : 1.f;
			s.points = it.points;
			snips.push_back(std::move(s));
			++nTrail;
		}
		else
		{
			if (nMark >= 1200)
				continue;
			PathingTrails::Marker m;
			m.mapId = it.mapId;
			m.world = { it.x, it.y, it.z };
			m.color = st.hasColor ? st.color : 0xFFFFC828u;
			m.alpha = st.hasAlpha ? st.alpha : 1.f;
			m.iconSize = st.hasIconSize ? st.iconSize : 1.f;
			m.heightOffset = st.hasHeightOffset ? st.heightOffset : 1.5f;
			std::snprintf(m.label, sizeof(m.label), "%s", it.type.c_str());
			marks.push_back(m);
			++nMark;
		}
	}

	if (WorldGpsD3d::Available() && !snips.empty())
		WorldGpsD3d::DrawTrails(vp, cam, avatar, maxDist, thick, snips, nullptr);
	else if (!snips.empty())
	{
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		if (dl)
		{
			for (const auto& s : snips)
			{
				WorldGpsImgui::DrawTrailBillboards(dl, vp, io.DisplaySize.x, io.DisplaySize.y,
					avatar, s, maxDist, thick, WorldGpsMath::kMaxSegments, false);
			}
		}
	}
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (dl && !marks.empty())
		WorldGpsImgui::DrawMarkers(dl, vp, io.DisplaySize.x, io.DisplaySize.y, avatar, marks);
	if (!dl)
		dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;
	for (int i = 0; i < static_cast<int>(gDoc.items.size()); ++i)
	{
		if (!IsSelected(i))
			continue;
		const auto& it = gDoc.items[static_cast<size_t>(i)];
		if (it.tombstone || it.isTrail)
			continue;
		float sx = 0.f, sy = 0.f;
		if (!WorldGpsMath::WorldToScreen({ it.x, it.y, it.z }, vp,
			io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			continue;
		dl->AddCircle(ImVec2(sx, sy), 12.f, IM_COL32(255, 220, 64, 230), 20, 2.2f);
	}
}

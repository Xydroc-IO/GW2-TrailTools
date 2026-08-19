#include "PackEditWorld.h"

#include "PackEditInternal.h"

#include "Globals.h"
#include "TrailToolsShared.h"
#include "WorldGpsD3d.h"
#include "WorldGpsImgui.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
	constexpr int kMaxMarks = 80;
	constexpr int kMaxTrails = 24;

	float Dist2(const WorldGpsMath::Vec3& a, float x, float y, float z)
	{
		const float dx = a.x - x, dy = a.y - y, dz = a.z - z;
		return dx * dx + dy * dy + dz * dz;
	}

	bool IsBreak(const PathingTrails::WorldPoint& p)
	{
		return p.x == 0.f && p.y == 0.f && p.z == 0.f;
	}

	float TrailNear2(const std::vector<PathingTrails::WorldPoint>& pts,
		const WorldGpsMath::Vec3& avatar)
	{
		float best = 1.0e30f;
		const int n = static_cast<int>(pts.size());
		const int step = n > 64 ? n / 32 : 1;
		for (int i = 0; i < n; i += step)
		{
			const auto& p = pts[static_cast<size_t>(i)];
			if (IsBreak(p))
				continue;
			const float d = Dist2(avatar, p.x, p.y, p.z);
			if (d < best)
				best = d;
		}
		return best;
	}

	void ClipNearby(const std::vector<PathingTrails::WorldPoint>& in,
		std::vector<PathingTrails::WorldPoint>& out,
		const WorldGpsMath::Vec3& avatar, float maxDist)
	{
		out.clear();
		const float max2 = maxDist * maxDist;
		bool run = false;
		for (const auto& p : in)
		{
			if (IsBreak(p))
			{
				if (run)
					out.push_back(p);
				run = false;
				continue;
			}
			const bool close = Dist2(avatar, p.x, p.y, p.z) <= max2;
			if (close)
			{
				out.push_back(p);
				run = true;
			}
			else if (run)
			{
				out.push_back(p);
				run = false;
			}
		}
	}
}

PackEdit::WorldGpu& PackEdit::WorldGpu::Get()
{
	static WorldGpu s;
	return s;
}

void PackEdit::WorldGpu::Cull(uint32_t mapId, const WorldGpsMath::Vec3& avatar, float maxDist)
{
	marks_.clear();
	trails_.clear();
	const float keep2 = maxDist * maxDist;
	const bool hide = !gDoc.hidden.empty();
	struct Cand { int i; float d; };
	std::vector<Cand> tc;
	tc.reserve(64);
	for (int i = 0; i < static_cast<int>(gDoc.items.size()); ++i)
	{
		const auto& it = gDoc.items[static_cast<size_t>(i)];
		if (it.tombstone)
			continue;
		if (gDoc.thisMapOnly && it.mapId != 0 && it.mapId != mapId)
			continue;
		if (hide && CategoryHidden(it.type))
			continue;
		if (it.isTrail)
		{
			if (it.points.size() < 2)
				continue;
			const float d = TrailNear2(it.points, avatar);
			if (d <= keep2)
				tc.push_back({ i, d });
		}
		else if (marks_.size() < static_cast<size_t>(kMaxMarks))
		{
			if (Dist2(avatar, it.x, it.y, it.z) <= keep2)
				marks_.push_back(i);
		}
	}
	std::sort(tc.begin(), tc.end(), [](const Cand& a, const Cand& b) { return a.d < b.d; });
	const int n = std::min(static_cast<int>(tc.size()), kMaxTrails);
	for (int i = 0; i < n; ++i)
		trails_.push_back(tc[static_cast<size_t>(i)].i);
}

void PackEdit::WorldGpu::Draw()
{
	if (!gDoc.worldDraw || TrailToolsDetail::gTab != 0)
		return;
	if (!G::Mumble || G::Mumble->uiTick == 0)
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
	const float maxDist = std::max(60.f, G::WorldTrailMaxDist);
	Cull(ctx->mapId, avatar, maxDist);

	std::vector<PathingTrails::WorldSnippet> snips;
	snips.reserve(trails_.size());
	for (int idx : trails_)
	{
		const auto& it = gDoc.items[static_cast<size_t>(idx)];
		PathingTrails::WorldSnippet s;
		s.color = it.style.hasColor ? it.style.color : 0xFFFFFFFFu;
		s.alpha = it.style.hasAlpha ? it.style.alpha : 1.f;
		s.trailScale = it.style.hasTrailScale ? it.style.trailScale : 1.f;
		ClipNearby(it.points, s.points, avatar, maxDist);
		if (s.points.size() >= 2)
			snips.push_back(std::move(s));
	}

	const float thick = std::clamp(G::WorldTrailWidth, 0.15f, 4.f);
	if (!snips.empty())
	{
		if (!WorldGpsD3d::Available() ||
			!WorldGpsD3d::DrawTrails(vp, cam, avatar, maxDist, thick, snips, nullptr))
		{
			ImDrawList* dl = ImGui::GetBackgroundDrawList();
			if (dl)
			{
				for (const auto& s : snips)
				{
					WorldGpsImgui::DrawTrailBillboards(dl, vp, io.DisplaySize.x, io.DisplaySize.y,
						avatar, s, maxDist, thick, 96, false);
				}
			}
		}
	}

	std::vector<PathingTrails::Marker> marks;
	marks.reserve(marks_.size());
	for (int idx : marks_)
	{
		const auto& it = gDoc.items[static_cast<size_t>(idx)];
		PathingTrails::Marker m;
		m.mapId = it.mapId;
		m.world = { it.x, it.y, it.z };
		m.color = it.style.hasColor ? it.style.color : 0xFFFFC828u;
		m.alpha = it.style.hasAlpha ? it.style.alpha : 1.f;
		m.iconSize = it.style.hasIconSize ? it.style.iconSize : 1.f;
		m.heightOffset = it.style.hasHeightOffset ? it.style.heightOffset : 1.5f;
		marks.push_back(m);
	}
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	if (dl && !marks.empty())
		WorldGpsImgui::DrawMarkers(dl, vp, io.DisplaySize.x, io.DisplaySize.y, avatar, marks);
	if (!dl)
		return;
	for (int i : gDoc.selItems)
	{
		if (i < 0 || i >= static_cast<int>(gDoc.items.size()))
			continue;
		const auto& it = gDoc.items[static_cast<size_t>(i)];
		if (it.tombstone || it.isTrail)
			continue;
		float sx = 0.f, sy = 0.f;
		if (!WorldGpsMath::WorldToScreen({ it.x, it.y, it.z }, vp,
			io.DisplaySize.x, io.DisplaySize.y, sx, sy))
			continue;
		dl->AddCircle(ImVec2(sx, sy), 12.f, IM_COL32(255, 220, 64, 230), 16, 2.f);
	}
}

void PackEdit::RenderWorld()
{
	WorldGpu::Get().Draw();
}

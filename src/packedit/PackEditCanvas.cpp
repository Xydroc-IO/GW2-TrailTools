#include "PackEditInternal.h"

#include "Globals.h"
#include "PadNav.h"
#include "TrailToolsGround.h"
#include "TrailToolsShared.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

void PackEdit::DrawCanvas()
{
	PadNav::SectionTitle("2D map");
	PadNav::BeginSection("pe_map");
	ImGui::TextDisabled("Wheel scrolls the pad · Ctrl+wheel zoom · drag pan · Shift+click place.");

	static float sZoom = 4.f;
	static float sPanX = 0.f, sPanZ = 0.f;
	static bool sFollow = true;
	ImGui::Checkbox("Follow player###pe_mfollow", &sFollow);
	ImGui::SameLine();
	ImGui::SliderFloat("###pe_mzoom", &sZoom, 0.4f, 24.f, "zoom %.1f");

	uint32_t mapId = 0;
	float ax = 0.f, ay = 0.f, az = 0.f;
	const bool pose = TrailToolsDetail::ReadMumblePose(mapId, ax, ay, az);

	const ImVec2 size(0.f, 260.f);
	if (!ImGui::BeginChild("###pe_canvas", size, true,
		PadNav::kNestedList | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::EndChild();
		PadNav::EndSection();
		return;
	}
	ImGui::PushTextWrapPos(0.f);
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImVec2 sz = ImGui::GetContentRegionAvail();
	if (sz.x < 8.f)
		sz.x = 8.f;
	if (sz.y < 8.f)
		sz.y = 8.f;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 p1(p0.x + sz.x, p0.y + sz.y);
	dl->AddRectFilled(p0, p1, IM_COL32(18, 22, 28, 255));
	dl->AddRect(p0, p1, IM_COL32(60, 80, 90, 180));

	ImGui::InvisibleButton("###pe_cvhit", sz);
	const bool hover = ImGui::IsItemHovered();
	ImGuiIO& io = ImGui::GetIO();
	const float cx = p0.x + sz.x * 0.5f;
	const float cy = p0.y + sz.y * 0.5f;

	/* Ctrl+wheel zooms; plain wheel is left for the pad (NoScrollWithMouse). */
	if (hover && io.KeyCtrl && io.MouseWheel != 0.f)
	{
		ImGui::SetItemUsingMouseWheel();
		const float oldZ = sZoom;
		sZoom = std::clamp(sZoom * (io.MouseWheel > 0.f ? 1.12f : 0.89f), 0.4f, 24.f);
		if (!sFollow)
		{
			const float wx = sPanX + (io.MousePos.x - cx) / oldZ;
			const float wz = sPanZ - (io.MousePos.y - cy) / oldZ;
			sPanX = wx - (io.MousePos.x - cx) / sZoom;
			sPanZ = wz + (io.MousePos.y - cy) / sZoom;
		}
		io.MouseWheel = 0.f;
		io.MouseWheelH = 0.f;
	}

	const bool panDrag = hover &&
		(ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f) ||
			(ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f) && !io.KeyShift));
	if (panDrag)
	{
		sFollow = false;
		sPanX -= io.MouseDelta.x / sZoom;
		sPanZ += io.MouseDelta.y / sZoom;
	}
	if (sFollow && pose)
	{
		sPanX = ax;
		sPanZ = az;
	}

	auto toScreen = [&](float wx, float wz) -> ImVec2 {
		return ImVec2(cx + (wx - sPanX) * sZoom, cy - (wz - sPanZ) * sZoom);
	};
	auto onMap = [&](const ImVec2& s) {
		return s.x >= p0.x - 8.f && s.y >= p0.y - 8.f &&
			s.x <= p1.x + 8.f && s.y <= p1.y + 8.f;
	};

	for (int g = -8; g <= 8; ++g)
	{
		const float wx = sPanX + static_cast<float>(g) * 25.f;
		const float wz = sPanZ + static_cast<float>(g) * 25.f;
		dl->AddLine(toScreen(wx, sPanZ - 200.f), toScreen(wx, sPanZ + 200.f),
			IM_COL32(40, 50, 58, 120));
		dl->AddLine(toScreen(sPanX - 200.f, wz), toScreen(sPanX + 200.f, wz),
			IM_COL32(40, 50, 58, 120));
	}

	int n = 0;
	int segs = 0;
	for (int i = 0; i < static_cast<int>(gDoc.items.size()) && n < 400; ++i)
	{
		const auto& it = gDoc.items[static_cast<size_t>(i)];
		if (it.tombstone || CategoryHidden(it.type))
			continue;
		if (gDoc.thisMapOnly && mapId && it.mapId && it.mapId != mapId)
			continue;
		const bool sel = IsSelected(i);
		if (it.isTrail)
		{
			ImVec2 prev{};
			bool have = false;
			for (const auto& pt : it.points)
			{
				if (pt.x == 0.f && pt.y == 0.f && pt.z == 0.f)
				{
					have = false;
					continue;
				}
				ImVec2 s = toScreen(pt.x, pt.z);
				if (have && segs < 600 && (onMap(prev) || onMap(s)))
				{
					dl->AddLine(prev, s,
						sel ? IM_COL32(255, 210, 80, 220) : IM_COL32(80, 180, 220, 180), 1.6f);
					++segs;
				}
				prev = s;
				have = true;
			}
			++n;
			continue;
		}
		ImVec2 s = toScreen(it.x, it.z);
		if (!onMap(s))
			continue;
		dl->AddCircleFilled(s, sel ? 5.5f : 3.5f,
			sel ? IM_COL32(255, 210, 64, 240) : IM_COL32(255, 170, 70, 210));
		++n;
	}

	if (pose)
	{
		ImVec2 me = toScreen(ax, az);
		float fx = 0.f, fz = 1.f;
		if (G::Mumble)
		{
			fx = G::Mumble->fAvatarFront[0];
			fz = G::Mumble->fAvatarFront[2];
			const float fl = std::sqrt(fx * fx + fz * fz);
			if (fl > 0.05f)
			{
				fx /= fl;
				fz /= fl;
			}
		}
		const ImVec2 nose = toScreen(ax + fx * (14.f / sZoom), az + fz * (14.f / sZoom));
		dl->AddCircleFilled(me, 5.f, IM_COL32(90, 255, 140, 255));
		dl->AddCircle(me, 8.f, IM_COL32(90, 255, 140, 180));
		dl->AddLine(me, nose, IM_COL32(90, 255, 140, 255), 2.f);
	}
	else
	{
		dl->AddText(ImVec2(p0.x + 10.f, p0.y + 10.f), IM_COL32(200, 180, 80, 220),
			"No Mumble pose — enter the game world.");
	}
	if (gDoc.items.empty())
	{
		dl->AddText(ImVec2(p0.x + 10.f, p0.y + 28.f), IM_COL32(180, 190, 200, 200),
			"No markers in this pack. Open a .taco or Shift+click to place.");
	}

	const bool click = hover && pose &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		!ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f);
	if (click)
	{
		const ImVec2 m = io.MousePos;
		const float wx = sPanX + (m.x - cx) / sZoom;
		const float wz = sPanZ - (m.y - cy) / sZoom;
		int best = -1;
		float bestD = 12.f * 12.f;
		for (int i = 0; i < static_cast<int>(gDoc.items.size()); ++i)
		{
			const auto& it = gDoc.items[static_cast<size_t>(i)];
			if (it.tombstone || it.isTrail)
				continue;
			if (gDoc.thisMapOnly && mapId && it.mapId && it.mapId != mapId)
				continue;
			const float dx = (it.x - wx) * sZoom;
			const float dz = (it.z - wz) * sZoom;
			const float d = dx * dx + dz * dz;
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		if (best >= 0)
		{
			if (io.KeyCtrl)
				SelectToggle(best);
			else
				RevealItem(best);
		}
		else if (io.KeyShift)
		{
			float y = ay;
			if (TrailToolsDetail::gGroundSnap)
				y = TrailToolsGround::EstimateY(wx, wz, ay);
			AddPoiAt(wx, y, wz, mapId);
			std::snprintf(gDoc.status, sizeof(gDoc.status),
				"Placed POI at %.1f, %.1f, %.1f (Y %s).", wx, y, wz,
				TrailToolsDetail::gGroundSnap ? "snapped" : "feet");
		}
	}

	ImGui::PopTextWrapPos();
	ImGui::EndChild();
	if (pose)
		ImGui::TextDisabled("Player  xz %.1f  %.1f  Y %.2f  map %u  ·  %d ground samples",
			ax, az, ay, mapId, TrailToolsGround::PoseSamples());
	if (ImGui::SmallButton("POI at player###pe_mfeet") && pose)
		AddPoiAt(ax, ay, az, mapId);
	PadNav::EndSection();
}

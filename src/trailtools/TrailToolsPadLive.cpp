#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsGround.h"
#include "TrailToolsWorldPick.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>

void TrailToolsDetail::DrawLiveTab()
{
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	const bool ok = ReadMumblePose(mapId, x, y, z);

	PadNav::SectionTitle("Live pose");
	PadNav::BeginSection("live_pose");
	if (!ok)
	{
		ImGui::TextColored(HelperTheme::Warn, "No Mumble pose — enter the game world.");
		PadNav::EndSection();
	}
	else
	{
		ImGui::Text("Map %u", mapId);
		ImGui::Text("XYZ  %.4f  %.4f  %.4f", x, y, z);
		ImGui::TextDisabled("TrlTool XZY  %.4f  %.4f  %.4f", x, z, y);

		char poiLine[320]{};
		std::snprintf(poiLine, sizeof(poiLine),
			"MapID=\"%u\" xpos=\"%.6g\" ypos=\"%.6g\" zpos=\"%.6g\"", mapId, x, y, z);
		if (ImGui::Button("Copy POI attrs###gw2tt_tt_copy_poi"))
		{
			CopyClipboard(poiLine);
			SetStatus("Copied POI attributes.");
		}
		PadNav::WrapSameLine(PadNav::ButtonWidth("Copy XYZ"));
		char vecLine[128]{};
		std::snprintf(vecLine, sizeof(vecLine), "%.6g %.6g %.6g", x, y, z);
		if (ImGui::Button("Copy XYZ###gw2tt_tt_copy_xyz"))
		{
			CopyClipboard(vecLine);
			SetStatus("Copied XYZ vector.");
		}
		PadNav::WrapSameLine(PadNav::ButtonWidth("Copy MapID"));
		char mapLine[32]{};
		std::snprintf(mapLine, sizeof(mapLine), "%u", mapId);
		if (ImGui::Button("Copy MapID###gw2tt_tt_copy_map"))
		{
			CopyClipboard(mapLine);
			SetStatus("Copied Map ID.");
		}
		PadNav::EndSection();
	}

	PadNav::SectionTitle("3D UberTool");
	PadNav::BeginSection("live_uber");
	ImGui::Checkbox("Enable UberTool (click + RGB gizmo)###gw2tt_tt_uber", &gUberToolEnabled);
	ImGui::TextDisabled("Select a marker or trail vertex in the world. Drag an axis to move.");
	ImGui::TextDisabled("Ctrl+click a trail to insert a point. Right-click while dragging cancels.");
	PadNav::EndSection();

	PadNav::SectionTitle("World click");
	PadNav::BeginSection("live_pick");
	ImGui::Checkbox("Enable click place/select###gw2tt_tt_wpick", &gWorldPickEnabled);
	ImGui::Checkbox("Ground snap###gw2tt_tt_gsnap", &gGroundSnap);
	ImGui::TextDisabled("Snap: walk samples + draft + open pack, plane fit. Not the game mesh.");
	ImGui::TextDisabled("%d walk samples. Walk a slope (or open a pack on it) before clicking.",
		TrailToolsGround::PoseSamples());
	static const char* kModes[] = {
		"Place marker",
		"Add trail point",
		"Select nearest",
	};
	if (gWorldPickMode < 0 || gWorldPickMode > 2)
		gWorldPickMode = 0;
	PadNav::SetFullRowWidth();
	ImGui::Combo("###gw2tt_tt_wmode", &gWorldPickMode, kModes, 3);
	float hx = 0.f, hy = 0.f, hz = 0.f;
	if (gWorldPickEnabled && TrailToolsWorldPick::RayFeetPlane(hx, hy, hz))
		ImGui::TextDisabled("Under cursor  %.2f  %.2f  %.2f", hx, hy, hz);
	PadNav::EndSection();

	PadNav::SectionTitle("Preview");
	PadNav::BeginSection("live_prev");
	ImGui::Checkbox("Show draft in world + compass###gw2tt_tt_prev", &gDraft.previewEnabled);
	PadNav::EndSection();

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

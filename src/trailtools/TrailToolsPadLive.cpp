#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsGround.h"
#include "TrailToolsWorldPick.h"

#include "Globals.h"
#include "HelperTheme.h"
#include "PackEdit.h"
#include "PadNav.h"
#include "Settings.h"

#include "imgui/imgui.h"

void TrailToolsDetail::DrawLiveTab()
{
	PadNav::SectionTitle("World click");
	PadNav::BeginSection("live_pick");
	ImGui::Checkbox("Enable click place/select###gw2tt_tt_wpick", &gWorldPickEnabled);
	ImGui::SameLine();
	ImGui::Checkbox("Ground snap###gw2tt_tt_gsnap", &gGroundSnap);
	ImGui::TextDisabled(
		"Snap: walk samples + draft + open pack, plane fit. Not the game mesh.");
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
	if (ImGui::Checkbox("UberTool###gw2tt_tt_uber", &gUberToolEnabled))
		Settings::SetDirty();
	ImGui::SameLine();
	if (ImGui::Checkbox("Draft preview###gw2tt_tt_prev", &gDraft.previewEnabled))
		Settings::SetDirty();
	if (ImGui::Checkbox("Hide trail near me###gw2tt_tt_pclear", &G::WorldTrailPlayerClearOn))
	{
		if (G::WorldTrailPlayerClearOn && G::WorldTrailPlayerClear < 0.05f)
			G::WorldTrailPlayerClear = 1.f;
		Settings::SetDirty();
	}
	ImGui::TextDisabled(
		"UberTool: click a trail/marker, then drag (white dot = slide, RGB = XYZ).");
	if (ImGui::Button("Clear world trail###gw2tt_tt_clrworld", ImVec2(-1.f, 0.f)))
	{
		ClearWorldDraftTrails();
		PackEdit::HideWorldOverlay();
		Settings::SetDirty();
	}
	ImGui::TextDisabled("Removes the GPS that is not in a Trails window, and pack overlay.");
	float hx = 0.f, hy = 0.f, hz = 0.f;
	if (gWorldPickEnabled && TrailToolsWorldPick::RayFeetPlane(hx, hy, hz))
		ImGui::TextDisabled("Under cursor  %.2f  %.2f  %.2f", hx, hy, hz);
	PadNav::EndSection();
}

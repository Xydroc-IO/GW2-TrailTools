#include "UI.h"

#include "CrashTrail.h"
#include "Globals.h"
#include "Gw2Ui.h"
#include "PathingTrails.h"
#include "Settings.h"
#include "TrailToolsBinds.h"
#include "TrailToolsPad.h"
#include "TrailToolsPreview.h"
#include "TrailToolsShared.h"
#include "TrailToolsUberTool.h"
#include "TrailToolsWorldPick.h"
#include "UiScale.h"
#include "PackEdit.h"
#include "WorldClick.h"

#include "imgui/imgui.h"

void UI_Render()
{
	CrashTrail::SetPhase("RT_Render");
	CrashTrail::Tick();
	UiScale::TickAuto();
	Gw2Ui::WarmCommon();

	uint32_t mapId = 0;
	if (G::Mumble && G::Mumble->uiTick != 0)
	{
		const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
		if (ctx)
			mapId = ctx->mapId;
	}
	PathingTrails::Update(mapId);
	PathingTrails::BeginFrame();

	TrailToolsBinds::Poll();
	CrashTrail::SetPhase("pad");
	WorldClick::TickImGui();
	TrailToolsPad::Render();
	CrashTrail::HeartbeatIfHot();
	PackEdit::Tick();
	PackEdit::DrawPopouts();
	const bool uberAte = TrailToolsUberTool::Tick();
	if (!uberAte)
		TrailToolsWorldPick::Tick();
	if (TrailToolsPad::AnyOpen() &&
		(TrailToolsDetail::HasDraftPreview() || TrailToolsDetail::gUberToolEnabled))
	{
		CrashTrail::SetPhase("preview");
		TrailToolsPreview::RenderWorld();
		TrailToolsUberTool::Render();
	}
	if (TrailToolsPad::AnyOpen())
		PackEdit::RenderWorld();

	static int sFrames = 0;
	if (++sFrames >= 600)
	{
		sFrames = 0;
		Settings::Save();
	}
	CrashTrail::SetPhase("idle");
}

void UI_Options()
{
	UI_DrawSettingsControls();
}

void UI_DrawSettingsControls()
{
	ImGui::TextUnformatted("GW2-TrailTools");
	ImGui::Separator();
	ImGui::Checkbox("Show Trail Tools", &G::ShowTrailTools);
	ImGui::SliderFloat("Opacity", &G::Opacity, 0.5f, 1.f, "%.2f");
	ImGui::SliderFloat("Font scale", &G::FontScale, 0.75f, 2.f, "%.2f");
	ImGui::Checkbox("Auto font scale", &G::FontScaleAuto);
	ImGui::Checkbox("Hide preview when map open", &G::HideWhenMapOpen);
	ImGui::Checkbox("Hide preview out of gameplay", &G::HideOutOfGameplay);
	ImGui::SliderFloat("GPS max distance", &G::WorldTrailMaxDist, 40.f, 400.f, "%.0f");
	ImGui::SliderFloat("GPS width", &G::WorldTrailWidth, 0.5f, 4.f, "%.2f");
	ImGui::SliderFloat("Trail player clear", &G::WorldTrailPlayerClear, 0.f, 3.f, "%.2f");
	ImGui::SliderFloat("Marker player clear", &G::WorldMarkerPlayerClear, 0.f, 3.f, "%.2f");
	ImGui::SliderFloat("World marker scale", &G::WorldMarkerScale, 0.5f, 3.f, "%.2f");
	if (ImGui::Button("Open Trail Tools"))
		TrailToolsPad::Open();
	if (ImGui::Button("Reload packs"))
		PathingTrails::ReloadPacks();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextDisabled("Thanks to Lady Elyssa for UI help and Windows testing.");
	Settings::SetDirty();
}

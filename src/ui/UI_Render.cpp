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
#include "TrailToolsGround.h"
#include "UiScale.h"
#include "PackEdit.h"
#include "WorldClick.h"

#include "imgui/imgui.h"

#include <windows.h>
#include <shellapi.h>

namespace
{
	void CenterLine(const char* text)
	{
		const float w = ImGui::CalcTextSize(text).x;
		const float avail = ImGui::GetContentRegionAvail().x;
		if (w < avail)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - w) * 0.5f);
		ImGui::TextUnformatted(text);
	}

	void DrawSettingsCredit()
	{
		const ImVec4 gold(0.82f, 0.68f, 0.28f, 1.f);
		const ImVec4 goldDim(0.72f, 0.58f, 0.24f, 1.f);
		const ImVec4 muted(0.72f, 0.74f, 0.76f, 1.f);
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.50f, 0.38f, 0.16f, 0.80f));
		ImGui::Separator();
		ImGui::PopStyleColor();
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, gold);
		CenterLine("CREATED BY XYDROC");
		ImGui::PopStyleColor();
		const char* donate =
			"If you would like to donate or support Trail Tools, you can do so here — ";
		const char* kofi = "ko-fi.com/xydroc";
		const float avail = ImGui::GetContentRegionAvail().x;
		const float donateW = ImGui::CalcTextSize(donate).x + ImGui::CalcTextSize(kofi).x;
		if (donateW < avail)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - donateW) * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_Text, muted);
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
		ImGui::TextUnformatted(donate);
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
		ImGui::SameLine(0.f, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, goldDim);
		ImGui::TextUnformatted(kofi);
		ImGui::PopStyleColor();
		{
			const ImVec2 min = ImGui::GetItemRectMin();
			const ImVec2 max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddLine(
				ImVec2(min.x, max.y - 1.f), ImVec2(max.x, max.y - 1.f),
				ImGui::GetColorU32(goldDim), 1.f);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				ShellExecuteA(nullptr, "open", "https://ko-fi.com/xydroc",
					nullptr, nullptr, SW_SHOWNORMAL);
		}
	}
}

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
	TrailToolsGround::TickSample();
	TrailToolsPad::Render();
	TrailToolsBinds::PollRecording();
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
	PackEdit::DrawWorldToggles("_opt");
	if (ImGui::Button("Clear world trail###gw2tt_opt_clrworld"))
	{
		TrailToolsDetail::ClearWorldDraftTrails();
		PackEdit::HideWorldOverlay();
	}
	if (ImGui::CollapsingHeader("Pack editor###gw2tt_opt_pe"))
		PackEdit::DrawTab();
	ImGui::TextDisabled("Keybinds work while GW2 is focused (pad can be closed).");
	ImGui::Separator();
	ImGui::Checkbox("Show Trail Tools", &G::ShowTrailTools);
	ImGui::SliderFloat("Opacity", &G::Opacity, 0.5f, 1.f, "%.2f");
	ImGui::SliderFloat("Font scale", &G::FontScale, 0.75f, 2.f, "%.2f");
	ImGui::Checkbox("Auto font scale", &G::FontScaleAuto);
	ImGui::Checkbox("Hide preview when map open", &G::HideWhenMapOpen);
	ImGui::Checkbox("Hide preview out of gameplay", &G::HideOutOfGameplay);
	if (ImGui::CollapsingHeader("World GPS###gw2tt_opt_gps"))
	{
		ImGui::SliderFloat("GPS max distance", &G::WorldTrailMaxDist, 40.f, 400.f, "%.0f");
		ImGui::SliderFloat("GPS width", &G::WorldTrailWidth, 0.15f, 4.f, "%.2f");
		ImGui::Checkbox("Trail texture (arrows)", &G::WorldTrailUseTexture);
		if (ImGui::Checkbox("Hide trail near me", &G::WorldTrailPlayerClearOn) &&
			G::WorldTrailPlayerClearOn && G::WorldTrailPlayerClear < 0.05f)
			G::WorldTrailPlayerClear = 1.f;
		ImGui::SliderFloat("Trail player clear", &G::WorldTrailPlayerClear, 0.f, 3.f, "%.2f");
		ImGui::SliderFloat("Marker player clear", &G::WorldMarkerPlayerClear, 0.f, 3.f, "%.2f");
		ImGui::SliderFloat("World marker scale", &G::WorldMarkerScale, 0.5f, 3.f, "%.2f");
	}
	if (ImGui::Button("Open Trail Tools"))
		TrailToolsPad::Open();
	ImGui::SameLine();
	if (ImGui::Button("Reload packs"))
		PathingTrails::ReloadPacks();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextDisabled("Thanks to Lady Elyssa for UI help, feature selection, and Windows testing.");
	DrawSettingsCredit();
	Settings::SetDirty();
}

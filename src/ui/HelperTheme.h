#pragma once

#include "Globals.h"
#include "Gw2Ui.h"
#include "PadDock.h"
#include "UiScale.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

/* Trail Tools “cartographer” theme — cool slate + teal accents (not wood/gold). */
namespace HelperTheme
{
	inline ImVec4 Gold(0.45f, 0.86f, 0.80f, 1.f);          /* accent (section titles) */
	inline ImVec4 GoldBright(0.62f, 0.95f, 0.90f, 1.f);
	inline ImVec4 GoldDim(0.28f, 0.62f, 0.58f, 1.f);
	inline ImVec4 GoldMuted(0.48f, 0.68f, 0.66f, 1.f);
	inline ImVec4 Ink(0.93f, 0.95f, 0.97f, 1.f);
	inline ImVec4 Muted(0.58f, 0.66f, 0.72f, 1.f);
	inline ImVec4 Bg(0.070f, 0.090f, 0.115f, 0.98f);
	inline ImVec4 Panel(0.110f, 0.140f, 0.175f, 1.f);
	inline ImVec4 Child(0.085f, 0.108f, 0.138f, 0.92f);
	inline ImVec4 Border(0.30f, 0.52f, 0.50f, 0.85f);
	inline ImVec4 BorderSoft(0.22f, 0.32f, 0.38f, 0.55f);
	inline ImVec4 TabActive(0.16f, 0.28f, 0.30f, 1.f);
	inline ImVec4 TabIdle(0.080f, 0.100f, 0.128f, 1.f);
	inline ImVec4 Header(0.14f, 0.22f, 0.26f, 0.96f);
	inline ImVec4 Warn(0.95f, 0.70f, 0.32f, 1.f);
	inline ImVec4 Ok(0.48f, 0.84f, 0.58f, 1.f);
	inline ImVec4 Accent(0.18f, 0.55f, 0.52f, 1.f);
	inline ImVec4 AccentHover(0.24f, 0.68f, 0.64f, 1.f);
	inline ImVec4 AccentActive(0.14f, 0.46f, 0.44f, 1.f);
	inline ImVec4 OnAccent(0.94f, 0.98f, 0.97f, 1.f);
	inline ImVec4 TitleBar(0.055f, 0.072f, 0.095f, 1.f);

	inline void ResetToBuiltin()
	{
		Gold = ImVec4(0.45f, 0.86f, 0.80f, 1.f);
		GoldBright = ImVec4(0.62f, 0.95f, 0.90f, 1.f);
		GoldDim = ImVec4(0.28f, 0.62f, 0.58f, 1.f);
		GoldMuted = ImVec4(0.48f, 0.68f, 0.66f, 1.f);
		Ink = ImVec4(0.93f, 0.95f, 0.97f, 1.f);
		Muted = ImVec4(0.58f, 0.66f, 0.72f, 1.f);
		Bg = ImVec4(0.070f, 0.090f, 0.115f, 0.98f);
		Panel = ImVec4(0.110f, 0.140f, 0.175f, 1.f);
		Child = ImVec4(0.085f, 0.108f, 0.138f, 0.92f);
		Border = ImVec4(0.30f, 0.52f, 0.50f, 0.85f);
		BorderSoft = ImVec4(0.22f, 0.32f, 0.38f, 0.55f);
		TabActive = ImVec4(0.16f, 0.28f, 0.30f, 1.f);
		TabIdle = ImVec4(0.080f, 0.100f, 0.128f, 1.f);
		Header = ImVec4(0.14f, 0.22f, 0.26f, 0.96f);
		Warn = ImVec4(0.95f, 0.70f, 0.32f, 1.f);
		Ok = ImVec4(0.48f, 0.84f, 0.58f, 1.f);
		Accent = ImVec4(0.18f, 0.55f, 0.52f, 1.f);
		AccentHover = ImVec4(0.24f, 0.68f, 0.64f, 1.f);
		AccentActive = ImVec4(0.14f, 0.46f, 0.44f, 1.f);
		OnAccent = ImVec4(0.94f, 0.98f, 0.97f, 1.f);
		TitleBar = ImVec4(0.055f, 0.072f, 0.095f, 1.f);
	}

	inline void Push()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, Ink);
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, Muted);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, Bg);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, Child);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.090f, 0.115f, 0.148f, 0.99f));
		ImGui::PushStyleColor(ImGuiCol_Border, BorderSoft);
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.055f, 0.070f, 0.092f, 0.95f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.090f, 0.130f, 0.155f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.110f, 0.170f, 0.190f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_TitleBg, TitleBar);
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.070f, 0.095f, 0.125f, 0.95f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, TitleBar);
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.080f, 0.100f, 0.128f, 0.80f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, Bg);
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_CheckMark, GoldBright);
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, GoldDim);
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, GoldBright);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.120f, 0.155f, 0.195f, 0.95f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.160f, 0.220f, 0.260f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.200f, 0.300f, 0.320f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_Header, Header);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.18f, 0.30f, 0.34f, 0.95f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, TabActive);
		ImGui::PushStyleColor(ImGuiCol_Separator, BorderSoft);
		ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, Gold);
		ImGui::PushStyleColor(ImGuiCol_SeparatorActive, GoldBright);
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.25f, 0.40f, 0.42f, 0.30f));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, Gold);
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, GoldBright);
		ImGui::PushStyleColor(ImGuiCol_Tab, TabIdle);
		ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.16f, 0.26f, 0.30f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_TabActive, TabActive);
		ImGui::PushStyleColor(ImGuiCol_TabUnfocused, TabIdle);
		ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, TabActive);
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, Header);
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, BorderSoft);
		ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.18f, 0.26f, 0.30f, 0.35f));
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(0.12f, 0.16f, 0.20f, 0.25f));
		ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.20f, 0.45f, 0.44f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_NavHighlight, Gold);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 12.f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 5.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 6.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 12.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.f);
		ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.f);
		ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	}

	inline void Pop()
	{
		ImGuiContext& g = *GImGui;
		const int nVar = g.StyleVarStack.Size;
		const int nCol = g.ColorStack.Size;
		if (nVar > 0)
			ImGui::PopStyleVar(nVar >= 15 ? 15 : nVar);
		if (nCol > 0)
			ImGui::PopStyleColor(nCol >= 45 ? 45 : nCol);
	}

	inline ImGuiWindowFlags PadFlags(ImGuiWindowFlags extra = 0)
	{
		return Gw2Ui::PadWindowFlags(extra);
	}

	inline void EndPad()
	{
		ImGuiWindow* pad = ImGui::GetCurrentWindow();
		ImGuiStyle& st = ImGui::GetStyle();
		st.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.f, 0.f, 0.f, 0.f);
		st.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.f, 0.f, 0.f, 0.f);
		st.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.f, 0.f, 0.f, 0.f);
		st.Colors[ImGuiCol_ScrollbarBg] = Bg;
		ImGui::End();
		Gw2Ui::PaintNativeScrollbars(G::Opacity > 0.05f ? G::Opacity : 1.f, pad);
	}

	struct ScopedWindow
	{
		float opacity = 1.f;
		explicit ScopedWindow(float o)
			: opacity(o)
		{
			Push();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
			ImGui::SetNextWindowBgAlpha(0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.f, 8.f));
		}
		bool AfterBegin(const char* title, bool* pOpen) const
		{
			PadDock::KeepOnScreen();
			const bool collapsed = ImGui::GetStateStorage()->GetBool(
				ImGui::GetID("##gw2tt_pad_collapsed"), false);
			if (!collapsed)
				Gw2Ui::PaintPadChrome(opacity, false, false, /*solidStack=*/true);
			return Gw2Ui::DrawPadTitleBar(title, pOpen, opacity, 0.f, /*solidStack=*/true);
		}
		~ScopedWindow()
		{
			ImGuiContext& g = *GImGui;
			const int n = g.StyleVarStack.Size;
			if (n >= 2)
				ImGui::PopStyleVar(2);
			else if (n > 0)
				ImGui::PopStyleVar(n);
			Pop();
		}
		ScopedWindow(const ScopedWindow&) = delete;
		ScopedWindow& operator=(const ScopedWindow&) = delete;
	};

	struct ScopedOverlay
	{
		explicit ScopedOverlay(float bgAlpha)
		{
			Push();
			ImGui::SetNextWindowBgAlpha(bgAlpha);
		}
		~ScopedOverlay()
		{
			Pop();
		}
		ScopedOverlay(const ScopedOverlay&) = delete;
		ScopedOverlay& operator=(const ScopedOverlay&) = delete;
	};

	constexpr float kPadFontRefW = 560.f;
	constexpr float kPadFontRefH = 600.f;

	struct ScopedFontScale
	{
		explicit ScopedFontScale(float /*refW*/ = kPadFontRefW, float /*refH*/ = kPadFontRefH)
		{
			ImGui::SetWindowFontScale(UiScale::EffectiveFontScale(kPadFontRefW, kPadFontRefH));
		}
		~ScopedFontScale() = default;
		ScopedFontScale(const ScopedFontScale&) = delete;
		ScopedFontScale& operator=(const ScopedFontScale&) = delete;
	};
}

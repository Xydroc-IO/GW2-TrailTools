#pragma once

#include "Globals.h"
#include "Gw2Ui.h"
#include "HelperTheme.h"
#include "UiScale.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <cstdio>
#include <cstring>

/* Pad section navigation. Prefer DrawSideRail for every themed pad —
   a fixed left column instead of wrapping rows or ImGui ◀ ▶ tabs. */
namespace PadNav
{
	/* Breathing room between content / slider labels and the scrollbar gutter. */
	constexpr float kScrollGutterPad = 8.f;

	/* Visible right edge in screen space. */
	inline float WrapEdgeX()
	{
		ImGuiWindow* w = ImGui::GetCurrentWindow();
		if (!w)
			return ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x - kScrollGutterPad;
		return w->WorkRect.Max.x - kScrollGutterPad;
	}

	/* Window-local wrap X — WorkRect / avail, not ContentRegionMax (that overshoots
	   after side-rail SameLine + BeginChild and never wraps inside the clip). */
	inline float WrapLocalX()
	{
		ImGuiWindow* w = ImGui::GetCurrentWindow();
		float localRight = ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - kScrollGutterPad;
		if (w)
		{
			const float workRight = w->WorkRect.Max.x - w->Pos.x - kScrollGutterPad + w->Scroll.x;
			if (workRight < localRight)
				localRight = workRight;
		}
		const float minX = ImGui::GetCursorPos().x + 32.f;
		if (localRight < minX)
			localRight = minX;
		return localRight;
	}

	/* Word-wrap to the live content edge (reflows when the pad is resized).
	   Always PopWrap() before ImGui::End() / EndChild() — do not RAII across End. */
	inline void PushWrap()
	{
		ImGui::PushTextWrapPos(WrapLocalX());
	}

	inline void PopWrap()
	{
		ImGui::PopTextWrapPos();
	}

	/* Fill remaining row width without trusting ContentRegionMax overshoot. */
	inline void SetFullRowWidth()
	{
		float w = ImGui::GetContentRegionAvail().x;
		if (w < 40.f)
			w = 40.f;
		ImGui::SetNextItemWidth(w);
	}

	/* Leave room for right-side labels + gutter (SliderFloat / DragFloat). */
	inline void PushLabeledItemWidth()
	{
		const float reserve = ImGui::GetFontSize() * 12.f + kScrollGutterPad;
		ImGui::PushItemWidth(-reserve);
	}

	inline void PopLabeledItemWidth()
	{
		ImGui::PopItemWidth();
	}

	/* Label text before ### id (ImGui right-side labels). */
	inline float VisibleLabelWidth(const char* label)
	{
		if (!label || !label[0])
			return 0.f;
		const char* end = std::strstr(label, "###");
		return end
			? ImGui::CalcTextSize(label, end, true).x
			: ImGui::CalcTextSize(label, nullptr, true).x;
	}

	/* Field + label span for WrapSameLine before InputText / DragFloat / etc. */
	inline float LabeledSpan(const char* label, float fieldW)
	{
		return fieldW + ImGui::GetStyle().ItemInnerSpacing.x + VisibleLabelWidth(label);
	}

	/* SameLine only when next item still fits (Account-style flow). */
	inline void WrapSameLine(float nextItemWidth)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float lastX2 = ImGui::GetItemRectMax().x;
		const float nextX2 = lastX2 + style.ItemSpacing.x + nextItemWidth;
		if (nextX2 < WrapEdgeX() - 1.f)
			ImGui::SameLine(0.f, style.ItemSpacing.x);
	}

	/* Wrap if needed, then SetNextItemWidth for a right-labeled widget. */
	inline void PrepLabeled(const char* label, float fieldW, bool first = false)
	{
		if (!first)
			WrapSameLine(LabeledSpan(label, fieldW));
		ImGui::SetNextItemWidth(fieldW);
	}

	/* Full-row labeled input: shrink field so the right-side label never clips. */
	inline void PushWidthForLabel(const char* label)
	{
		const float reserve = VisibleLabelWidth(label) +
			ImGui::GetStyle().ItemInnerSpacing.x + kScrollGutterPad;
		ImGui::PushItemWidth(-(reserve > 24.f ? reserve : 24.f));
	}

	inline void PopWidthForLabel()
	{
		ImGui::PopItemWidth();
	}

	/* Caption above, full-width control — avoids right-side label clipping. */
	inline bool SliderFloatRow(const char* caption, const char* id, float* v,
		float vMin, float vMax, const char* fmt = "%.2f")
	{
		ImGui::TextUnformatted(caption);
		ImGui::SetNextItemWidth(-1.f);
		char buf[96];
		std::snprintf(buf, sizeof(buf), "##%s", id);
		return ImGui::SliderFloat(buf, v, vMin, vMax, fmt);
	}

	inline float CheckboxWidth(const char* label)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float box = ImGui::GetFrameHeight();
		return box + style.ItemInnerSpacing.x + ImGui::CalcTextSize(label, nullptr, true).x;
	}

	inline float ButtonWidth(const char* label)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		return ImGui::CalcTextSize(label, nullptr, true).x + style.FramePadding.x * 2.f;
	}

	/* Pack chips onto wrapping rows (imgui_demo pattern: SameLine only when the
	   next chip still fits on the current row). Pass first=true for the first
	   chip in a group (or after Spacing / headers). */
	inline bool WrapButton(const char* label, bool selected = false, bool first = false)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float btnW = ButtonWidth(label);
		if (!first)
		{
			const float lastX2 = ImGui::GetItemRectMax().x;
			const float nextX2 = lastX2 + style.ItemSpacing.x + btnW;
			if (nextX2 < WrapEdgeX() - 1.f)
				ImGui::SameLine(0.f, style.ItemSpacing.x);
		}

		if (selected)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::TabActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::TabActive);
			ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::GoldBright);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabIdle);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::Header);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::TabActive);
			ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::Muted);
		}
		const bool clicked = ImGui::SmallButton(label);
		ImGui::PopStyleColor(4);
		return clicked;
	}

	inline void WrapSlash()
	{
		const float slashW = ImGui::CalcTextSize("/").x;
		const float lastX2 = ImGui::GetItemRectMax().x;
		const float nextX2 = lastX2 + 4.f + slashW;
		if (nextX2 < WrapEdgeX() - 1.f)
			ImGui::SameLine(0.f, 4.f);
		ImGui::TextDisabled("/");
	}

	/* Wrapping chip rows (Unlocks kinds, tight toolbars). */
	inline int DrawTabs(const char* id, const char* const* labels, int count, int current)
	{
		if (!labels || count <= 0)
			return 0;
		if (current < 0)
			current = 0;
		if (current >= count)
			current = count - 1;

		ImGui::PushID(id);
		for (int i = 0; i < count; ++i)
		{
			ImGui::PushID(i);
			if (WrapButton(labels[i], i == current, /*first=*/i == 0))
				current = i;
			ImGui::PopID();
		}
		ImGui::PopID();
		ImGui::Spacing();
		ImGui::Separator();
		return current;
	}

	/* Horizontal top bar with optional DAT icons (Log Manager style). */
	inline int DrawTopBar(const char* id, const char* const* labels, int count, int current,
		const int* icons = nullptr)
	{
		if (!labels || count <= 0)
			return 0;
		if (current < 0)
			current = 0;
		if (current >= count)
			current = count - 1;

		ImGui::PushID(id);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.f, 4.f));
		for (int i = 0; i < count; ++i)
		{
			ImGui::PushID(i);
			char buf[96];
			std::snprintf(buf, sizeof(buf), "%s###top_%d", labels[i], i);
			const bool on = (i == current);
			if (on)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabActive);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::TabActive);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::TabActive);
				ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::GoldBright);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabIdle);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::Header);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::TabActive);
				ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::Muted);
			}
			bool clicked = false;
			const int asset = (icons && icons[i] > 0) ? icons[i] : 0;
			if (asset > 0)
				clicked = Gw2Ui::IconLabelButton(buf, asset, 16.f);
			else
				clicked = ImGui::SmallButton(buf);
			ImGui::PopStyleColor(4);
			if (clicked)
				current = i;
			if (i + 1 < count)
				ImGui::SameLine(0.f, 4.f);
			ImGui::PopID();
		}
		ImGui::PopStyleVar();
		ImGui::PopID();
		ImGui::Spacing();
		ImGui::Separator();
		return current;
	}

	/* Left rail: icon dock (tooltips from labels). Optional DAT icons (icons[i] > 0).
	   Caller draws content after — rail ends with SameLine.
	   Pad body order: DrawSideRail → BeginChild body → Blurb → controls → list.
	   Do not echo the window title in the body (chrome title bar owns it). */
	inline int DrawSideRail(const char* id, const char* const* labels, int count, int current,
		float width = 0.f, const int* icons = nullptr)
	{
		/* Auto-size from live font + pad footprint (call after ScopedFontScale). */
		const float iconSz = icons ? UiScale::RailIconSize(40.f) : 16.f;
		const float cell = icons ? UiScale::RailCellSize(iconSz) : 22.f;
		if (width <= 1.f)
		{
			if (icons)
				width = UiScale::IconRailWidth(cell);
			else
				width = UiScale::FitSideRailWidth(labels, count, 80.f, 260.f, 0.f);
		}
		else
			width = UiScale::SideRailWidth(width);
		if (!labels || count <= 0)
			return 0;
		if (current < 0)
			current = 0;
		if (current >= count)
			current = count - 1;

		const float padY = UiScale::Clampf(cell * 0.12f, 4.f, 10.f);
		const float gapY = UiScale::Clampf(cell * 0.12f, 4.f, 8.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.f, padY));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, gapY));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
		const float railH = -HelperTheme::ResizeGripClearance();
		ImGui::BeginChild(id, ImVec2(width, railH), true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NavFlattened);

		for (int i = 0; i < count; ++i)
		{
			ImGui::PushID(i);
			char buf[96];
			std::snprintf(buf, sizeof(buf), "%s###side_%d", labels[i], i);
			const int asset = (icons && icons[i] > 0) ? icons[i] : 0;
			if (Gw2Ui::RailToggle(buf, i == current, asset, iconSz, false))
				current = i;
			ImGui::PopID();
		}

		ImGui::EndChild();
		ImGui::PopStyleVar(3);
		ImGui::SameLine(0.f, 8.f);
		return current;
	}

	/* Toggle-style rail entry for helper chrome (open pads / flags).
	   Icon dock only (hover for names).
	   iconSzOverride < 0 → default 52. */
	inline bool SideToggle(const char* label, bool on, int assetId = 0, float iconSzOverride = -1.f)
	{
		const float iconSz = (iconSzOverride > 0.f) ? iconSzOverride : 52.f;
		return Gw2Ui::RailToggle(label, on, assetId, iconSz, false);
	}

	/* Shared pad copy hierarchy — use on every companion / Account tab.
	   SectionTitle = real subsections only (never the window name).
	   Blurb = one muted lead line under chrome / after rail. */
	inline void SectionTitle(const char* title)
	{
		if (!title || !title[0])
			return;
		ImGui::Spacing();
		PushWrap();
		ImGui::TextColored(HelperTheme::Gold, "%s", title);
		PopWrap();
		const ImVec2 a = ImGui::GetItemRectMin();
		const ImVec2 b = ImGui::GetItemRectMax();
		const float lineW = ImMin(ImGui::CalcTextSize(title).x, b.x - a.x);
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(a.x, b.y + 2.f),
			ImVec2(a.x + lineW, b.y + 2.f),
			ImGui::ColorConvertFloat4ToU32(HelperTheme::Accent), 1.5f);
		ImGui::Dummy(ImVec2(0.f, 4.f));
	}

	/* Card-like section: content drawn on channel 1, panel behind on merge. */
	inline void BeginSection(const char* id)
	{
		ImGui::Spacing();
		ImGui::PushID(id);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->ChannelsSplit(2);
		dl->ChannelsSetCurrent(1);
		ImGui::BeginGroup();
		ImGui::Indent(10.f);
		ImGui::Dummy(ImVec2(0.f, 6.f));
	}

	inline void EndSection()
	{
		ImGui::Dummy(ImVec2(0.f, 6.f));
		ImGui::Unindent(10.f);
		ImGui::EndGroup();
		const ImVec2 a = ImGui::GetItemRectMin();
		const ImVec2 b = ImGui::GetItemRectMax();
		ImVec2 lo(a.x - 6.f, a.y - 2.f);
		ImVec2 hi(b.x + 6.f, b.y + 2.f);
		const float right = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;
		if (hi.x < right - 4.f)
			hi.x = right - 4.f;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->ChannelsSetCurrent(0);
		dl->AddRectFilled(lo, hi,
			ImGui::ColorConvertFloat4ToU32(HelperTheme::Panel), 5.f);
		dl->AddRect(lo, hi,
			ImGui::ColorConvertFloat4ToU32(HelperTheme::BorderSoft), 5.f, 0, 1.f);
		dl->ChannelsMerge();
		ImGui::PopID();
		ImGui::Spacing();
	}

	/* Caption above full-width control — readable left-to-right flow. */
	inline bool InputCaption(const char* caption, const char* id, char* buf, size_t n)
	{
		PushWrap();
		ImGui::TextColored(HelperTheme::Muted, "%s", caption);
		PopWrap();
		SetFullRowWidth();
		char hid[128]{};
		std::snprintf(hid, sizeof(hid), "##%s", id);
		return ImGui::InputText(hid, buf, n);
	}

	inline bool PrimaryButton(const char* label)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::Accent);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::AccentHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::AccentActive);
		ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::OnAccent);
		const bool c = ImGui::Button(label);
		ImGui::PopStyleColor(4);
		return c;
	}

	inline void Blurb(const char* text)
	{
		if (!text || !text[0])
			return;
		PushWrap();
		ImGui::TextColored(HelperTheme::Muted, "%s", text);
		PopWrap();
	}

	inline void Meta(const char* text)
	{
		if (!text || !text[0])
			return;
		PushWrap();
		ImGui::TextColored(HelperTheme::GoldMuted, "%s", text);
		PopWrap();
	}

	inline void StatusBusy(const char* text = "Updating...")
	{
		PushWrap();
		ImGui::TextColored(HelperTheme::Warn, "%s", text ? text : "Updating...");
		PopWrap();
	}

	inline void StatusOk(const char* text)
	{
		if (!text || !text[0])
			return;
		PushWrap();
		ImGui::TextColored(HelperTheme::Ok, "%s", text);
		PopWrap();
	}

	inline void StatusWarn(const char* text)
	{
		if (!text || !text[0])
			return;
		PushWrap();
		ImGui::TextColored(HelperTheme::Warn, "%s", text);
		PopWrap();
	}

	/* Primary Refresh control — icon when DAT chrome is ready. */
	inline bool RefreshButton(const char* id = "###gw2tt_refresh")
	{
		char buf[64];
		std::snprintf(buf, sizeof(buf), "Refresh%s", id && id[0] ? id : "###gw2tt_refresh");
		return Gw2Ui::IconLabelButton(buf, Gw2Ui::Icon::Bag, 16.f);
	}
}

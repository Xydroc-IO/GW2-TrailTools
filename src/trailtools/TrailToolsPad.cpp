#include "TrailToolsPad.h"
#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"

#include "CrashTrail.h"
#include "EiRuntime.h"
#include "Globals.h"
#include "Gw2Ui.h"
#include "HelperTheme.h"
#include "PadDock.h"
#include "PadNav.h"
#include "Settings.h"
#include "TrailToolsBinds.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdio>
#include <functional>

namespace
{
	constexpr float kHubW = PadDock::kWorkbenchW;
	constexpr float kHubH = PadDock::kWorkbenchH;
	constexpr float kDeskW = PadDock::kWorkbenchW;
	constexpr float kDeskH = PadDock::kWorkbenchH;
	constexpr float kEditW = PadDock::kWorkbenchW;
	constexpr float kEditH = PadDock::kWorkbenchH - 80.f;
	constexpr float kEditMinW = 520.f;

	G::PadGeom GeomFrom(float x, float y, float w, float h)
	{
		G::PadGeom g{};
		g.x = x;
		g.y = y;
		g.w = w;
		g.h = h;
		return g;
	}

	void GeomTo(const G::PadGeom& g, float& x, float& y, float& w, float& h)
	{
		x = g.x;
		y = g.y;
		w = g.w;
		h = g.h;
	}

	bool RenderCollapsiblePad(
		const char* title,
		bool& showFlag,
		G::PadGeom& geom,
		bool& placeOnce,
		bool& focus,
		float defW,
		float defH,
		ImVec2 fallbackPos,
		const std::function<void()>& body,
		bool* outFocused = nullptr,
		float minW = 320.f,
		bool bodyAlwaysScroll = true) /* false: caller owns rail + body (MarkersN) */
	{
		if (!showFlag)
			return false;

		if (geom.w > 1.f && geom.w < minW)
			geom.w = minW;

		const float maxH = PadDock::MaxH(320.f);
		PadDock::SetSizeConstraints(title, minW, 160.f, PadDock::MaxW(900.f), maxH);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);
		PadDock::Place(geom, placeOnce, defW, defH, fallbackPos);
		PadDock::ApplyCollapsedSize(title, geom.w >= 80.f ? geom.w : defW);
		if (!placeOnce && geom.w < 80.f)
			ImGui::SetNextWindowSize(ImVec2(defW, defH), ImGuiCond_FirstUseEver);
		if (focus)
		{
			/* Wine: skip focus steal (same as PadDock / WinePadOpen). */
			if (!EiRuntime::IsWine())
				ImGui::SetNextWindowFocus();
			focus = false;
		}

		bool open = showFlag;
		HelperTheme::ScopedWindow theme(G::Opacity);
		/* Scroll lives in a body child (same as hub) — keeps the root chrome
		   flush and stops a root scrollbar from fighting the title controls. */
		ImGuiWindowFlags padFlags = HelperTheme::PadFlags(
			ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoScrollbar);
		if (PadDock::IsCustomCollapsed(title))
			padFlags |= ImGuiWindowFlags_NoResize;
		const bool padBody = ImGui::Begin(title, &open, padFlags);
		if (!theme.AfterBegin(title, &open) || !padBody || !open)
		{
			const ImVec2 p = ImGui::GetWindowPos();
			if (std::fabs(p.x - geom.x) > 0.5f || std::fabs(p.y - geom.y) > 0.5f)
			{
				geom.x = p.x;
				geom.y = p.y;
				Settings::SetDirty();
			}
			const bool hovered = ImGui::IsWindowHovered(
				ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
			if (outFocused)
				*outFocused = focused;
			HelperTheme::EndPad();
			if (!open)
			{
				showFlag = false;
				Settings::SetDirty();
			}
			return hovered || (focused && ImGui::GetIO().WantTextInput);
		}

		if (!ImGui::IsWindowCollapsed() && PadDock::Capture(geom))
			Settings::SetDirty();

		HelperTheme::ScopedFontScale fontScale(defW, defH);
		if (bodyAlwaysScroll)
		{
			const float bodyH = -HelperTheme::ResizeGripClearance();
			ImGui::BeginChild("###gw2tt_pad_body", ImVec2(0.f, bodyH), false,
				ImGuiWindowFlags_AlwaysVerticalScrollbar);
			body();
			ImGui::EndChild();
		}
		else
			body();

		const bool hovered = ImGui::IsWindowHovered(
			ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		const bool focusedWin = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		if (outFocused)
			*outFocused = focusedWin;
		const bool typingHere = focusedWin && ImGui::GetIO().WantTextInput;
		HelperTheme::EndPad();
		return hovered || typingHere;
	}

	ImVec2 DeskFallback(bool markers)
	{
		const ImVec2 base = PadDock::ForTrailPopout(kDeskW, kDeskH);
		if (markers)
			return ImVec2(base.x, base.y + 72.f);
		return base;
	}

	ImVec2 TrailEditorFallback(int index)
	{
		/* Cascade far enough that windows never look like a single stacked pad. */
		constexpr float kStepX = 56.f;
		constexpr float kStepY = 64.f;
		ImVec2 base = PadDock::ForTrailPopout(kEditW, kEditH);
		if (TrailToolsDetail::gShowTrailsDesk && TrailToolsDetail::gTrailsDeskX >= 0.f)
		{
			base.x = TrailToolsDetail::gTrailsDeskX + TrailToolsDetail::gTrailsDeskW + 12.f;
			base.y = TrailToolsDetail::gTrailsDeskY + static_cast<float>(index) * kStepY;
		}
		else
		{
			base.x += static_cast<float>(index) * kStepX;
			base.y += static_cast<float>(index) * kStepY;
		}
		return PadDock::ClampPos(base.x, base.y, kEditW);
	}

	ImVec2 MarkerEditorFallback(int index)
	{
		constexpr float kStepX = 56.f;
		constexpr float kStepY = 64.f;
		ImVec2 base = PadDock::ForMarkerPopout(kEditW, kEditH);
		if (TrailToolsDetail::gShowMarkersDesk && TrailToolsDetail::gMarkersDeskX >= 0.f)
		{
			base.x = TrailToolsDetail::gMarkersDeskX + TrailToolsDetail::gMarkersDeskW + 12.f;
			base.y = TrailToolsDetail::gMarkersDeskY + static_cast<float>(index) * kStepY;
		}
		else
		{
			base.x += static_cast<float>(index) * kStepX + 24.f;
			base.y += static_cast<float>(index) * kStepY + 48.f;
		}
		return PadDock::ClampPos(base.x, base.y, kEditW);
	}
}

bool TrailToolsPad::AnyOpen()
{
	return TrailToolsDetail::AnyAuthoringPadOpen();
}

void TrailToolsPad::Open()
{
	G::ShowTrailTools = true;
	TrailToolsDetail::gPlaceOnce = true;
	TrailToolsDetail::gFocus = true;
	Settings::SetDirty();
}

void TrailToolsPad::OpenTrailsDesk()
{
	TrailToolsDetail::OpenTrailsDesk();
	Settings::SetDirty();
}

void TrailToolsPad::OpenMarkersDesk()
{
	TrailToolsDetail::OpenMarkersDesk();
	Settings::SetDirty();
}

void TrailToolsPad::OpenTrailsWindow()
{
	using namespace TrailToolsDetail;
	OpenNewTrailEditor();
	gTab = 0; /* Content */
	Settings::SetDirty();
}

void TrailToolsPad::OpenMarkersWindow()
{
	using namespace TrailToolsDetail;
	OpenNewMarkerEditor();
	gTab = 0; /* Content */
	Settings::SetDirty();
}

bool TrailToolsPad::Render()
{
	using namespace TrailToolsDetail;
	bool hover = false;

	if (G::ShowTrailTools)
	{
		char title[280]{};
		std::snprintf(title, sizeof(title), "Trail Tools%s###GW2TrailToolsHub",
			gDraft.xmlDirty || gDraft.trailDirty ? " *" : "");

		const float maxH = PadDock::MaxH(320.f);
		PadDock::SetSizeConstraints(title, 440.f, 280.f, PadDock::MaxW(820.f), maxH);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);
		PadDock::Place(G::PadTrailTools, gPlaceOnce, kHubW, kHubH, PadDock::BesideHelper(kHubW));
		PadDock::ApplyCollapsedSize(title,
			G::PadTrailTools.w >= 80.f ? G::PadTrailTools.w : kHubW);
		if (!gPlaceOnce && G::PadTrailTools.w < 80.f)
			ImGui::SetNextWindowSize(ImVec2(kHubW, kHubH), ImGuiCond_FirstUseEver);
		if (gFocus)
		{
			if (!EiRuntime::IsWine())
				ImGui::SetNextWindowFocus();
			gFocus = false;
		}

		bool open = G::ShowTrailTools;
		HelperTheme::ScopedWindow theme(G::Opacity);
		/* Body child owns scrolling — root scrollbar leaves a gutter stub when minimized. */
		ImGuiWindowFlags padFlags = HelperTheme::PadFlags(
			ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoScrollbar);
		if (PadDock::IsCustomCollapsed(title))
			padFlags |= ImGuiWindowFlags_NoResize;
		const bool padBody = ImGui::Begin(title, &open, padFlags);
		if (!theme.AfterBegin(title, &open) || !padBody || !open)
		{
			const ImVec2 p = ImGui::GetWindowPos();
			if (std::fabs(p.x - G::PadTrailTools.x) > 0.5f ||
				std::fabs(p.y - G::PadTrailTools.y) > 0.5f)
			{
				G::PadTrailTools.x = p.x;
				G::PadTrailTools.y = p.y;
				Settings::SetDirty();
			}
			hover = ImGui::IsWindowHovered(
				ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
				(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
					ImGui::GetIO().WantTextInput);
			HelperTheme::EndPad();
			if (!open)
			{
				G::ShowTrailTools = false;
				Settings::SetDirty();
			}
		}
		else
		{
			if (!ImGui::IsWindowCollapsed() && PadDock::Capture(G::PadTrailTools))
				Settings::SetDirty();

			HelperTheme::ScopedFontScale fontScale(kHubW, kHubH);

			/* Recover from a saved/narrow geom that clips the body. */
			{
				const ImVec2 sz = ImGui::GetWindowSize();
				if (sz.x < 440.f)
					ImGui::SetWindowSize(ImVec2(kHubW, sz.y < 280.f ? kHubH : sz.y));
			}

			static const char* kTabs[] = { "Content", "Keybinds" };
			static const int kTabIcons[] = {
				static_cast<int>(Gw2Ui::Icon::TrailAnvil),
				static_cast<int>(Gw2Ui::Icon::Options),
			};
			CrashTrail::SetPhase("pad.rail");
			/* After a rail tab change, ignore Pop out / New window for this frame
			   (Windows can deliver the same click onto the newly shown body). */
			static int sPrevRailTab = -1;
			gHubSkipOpenClicks = false;
			if (gTab < 0 || gTab > 1)
				gTab = 0;
			const int railTab = PadNav::DrawSideRail(
				"###gw2tt_tt_nav", kTabs, 2, gTab, 0.f, kTabIcons);
			if (sPrevRailTab >= 0 && railTab != sPrevRailTab)
				gHubSkipOpenClicks = true;
			sPrevRailTab = railTab;
			gTab = railTab;

			const float bodyH = -HelperTheme::ResizeGripClearance();
			CrashTrail::SetPhase("pad.body");
			ImGui::BeginChild("###gw2tt_tt_body", ImVec2(0.f, bodyH), false,
				ImGuiWindowFlags_AlwaysVerticalScrollbar);
			PadNav::PushWrap();
			if (gTab == 0)
			{
				CrashTrail::SetPhase("pad.tab.content");
				DrawContentTab();
			}
			else
			{
				CrashTrail::SetPhase("pad.tab.keybinds");
				DrawKeybindsTab();
			}
			PadNav::PopWrap();
			ImGui::EndChild();

			hover = ImGui::IsWindowHovered(
				ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
				(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
					ImGui::GetIO().WantTextInput);
			CrashTrail::SetPhase("pad.end");
			HelperTheme::EndPad();
		}
	}

	/* Trails desk (optional pop-out) */
	if (gShowTrailsDesk)
	{
		G::PadGeom geom = GeomFrom(gTrailsDeskX, gTrailsDeskY, gTrailsDeskW, gTrailsDeskH);
		hover = RenderCollapsiblePad(
			"Trail Tools — Trails###GW2TrailToolsTrailsDesk",
			gShowTrailsDesk,
			geom,
			gPlaceOnceTrailsDesk,
			gFocusTrailsDesk,
			kDeskW,
			kDeskH,
			DeskFallback(false),
			[]() {
				DrawTrailDesk(true);
			}) || hover;
		GeomTo(geom, gTrailsDeskX, gTrailsDeskY, gTrailsDeskW, gTrailsDeskH);
		if (!gShowTrailsDesk)
			gPopoutTrails = false;
		else
			gPopoutTrails = true;
	}

	/* Markers desk (optional pop-out) */
	if (gShowMarkersDesk)
	{
		G::PadGeom geom = GeomFrom(gMarkersDeskX, gMarkersDeskY, gMarkersDeskW, gMarkersDeskH);
		hover = RenderCollapsiblePad(
			"Trail Tools — Markers###GW2TrailToolsMarkersDesk",
			gShowMarkersDesk,
			geom,
			gPlaceOnceMarkersDesk,
			gFocusMarkersDesk,
			kDeskW,
			kDeskH,
			DeskFallback(true),
			[]() {
				DrawMarkersDesk(true);
			}) || hover;
		GeomTo(geom, gMarkersDeskX, gMarkersDeskY, gMarkersDeskW, gMarkersDeskH);
		if (!gShowMarkersDesk)
			gPopoutMarkers = false;
		else
			gPopoutMarkers = true;
	}

	hover = RenderXmlEditorPad() || hover;

	/* Trails1 … TrailsN */
	for (int i = 0; i < kMaxTrailEditors; ++i)
	{
		TrailEditorSlot& slot = gTrailEditors[i];
		if (!slot.open)
			continue;
		G::PadGeom geom = GeomFrom(slot.geomX, slot.geomY, slot.geomW, slot.geomH);
		if (i == 0 && slot.geomX < 0.f && PadDock::HasSavedPos(G::PadTrailEditor))
			geom = G::PadTrailEditor;

		char title[280]{};
		const char* rel = slot.trail.fileRel.empty()
			? "Data/example.trl"
			: slot.trail.fileRel.c_str();
		std::snprintf(title, sizeof(title), "Trails%d - %s%s###GW2TrailToolsTrailEd%d",
			i + 1, rel, slot.dirty ? " *" : "", i);

		bool focused = false;
		hover = RenderCollapsiblePad(
			title,
			slot.open,
			geom,
			slot.placeOnce,
			slot.focus,
			kEditW,
			kEditH,
			TrailEditorFallback(i),
			[i]() {
				PushTrailEditorToActive(i);
				DrawTrailRawEditor();
				PopTrailEditorFromActive(i);
			},
			&focused,
			kEditMinW,
			false) || hover;

		GeomTo(geom, slot.geomX, slot.geomY, slot.geomW, slot.geomH);
		if (i == 0)
			G::PadTrailEditor = geom;
		if (focused)
			gTrailFocusSlot = i;
		if (!slot.open && gTrailRecordSlot == i)
			gTrailRecordSlot = -1;
		if (!slot.open && gTrailFocusSlot == i)
			gTrailFocusSlot = -1;
	}
	auto slotOpen = [](int s) {
		return s >= 0 && s < kMaxTrailEditors && gTrailEditors[s].open;
	};
	if (!slotOpen(gTrailRecordSlot))
		gTrailRecordSlot = -1;
	if (!slotOpen(gTrailFocusSlot))
	{
		gTrailFocusSlot = -1;
		for (int i = 0; i < kMaxTrailEditors; ++i)
		{
			if (gTrailEditors[i].open) { gTrailFocusSlot = i; break; }
		}
	}
	if (gTrailRecordSlot < 0 && TrailToolsBinds::Get().trailRecording)
	{
		TrailToolsBinds::Get().trailRecording = false;
		TrailToolsBinds::Get().trailPaused = false;
	}
	SyncTrailWindowFocus();

	/* Markers1 … MarkersN */
	for (int i = 0; i < kMaxMarkerEditors; ++i)
	{
		MarkerEditorSlot& slot = gMarkerEditors[i];
		if (!slot.open)
			continue;
		G::PadGeom geom = GeomFrom(slot.geomX, slot.geomY, slot.geomW, slot.geomH);
		if (i == 0 && slot.geomX < 0.f && PadDock::HasSavedPos(G::PadMarkerEditor))
			geom = G::PadMarkerEditor;

		char title[96]{};
		if (slot.tab == 1)
			std::snprintf(title, sizeof(title),
				"Markers%d Settings###GW2TrailToolsMarkerEd%d", i + 1, i);
		else
			std::snprintf(title, sizeof(title),
				"Markers%d###GW2TrailToolsMarkerEd%d", i + 1, i);

		hover = RenderCollapsiblePad(
			title,
			slot.open,
			geom,
			slot.placeOnce,
			slot.focus,
			kEditW,
			kEditH,
			MarkerEditorFallback(i),
			[i]() {
				DrawMarkerRawEditorForSlot(i);
			},
			nullptr,
			kEditMinW,
			false) || hover;

		GeomTo(geom, slot.geomX, slot.geomY, slot.geomW, slot.geomH);
		if (i == 0)
			G::PadMarkerEditor = geom;
	}

	gPadPointerOver = hover;
	return hover;
}

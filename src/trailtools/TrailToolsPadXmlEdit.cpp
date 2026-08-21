#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsXml.h"

#include "EiRuntime.h"
#include "Globals.h"
#include "HelperTheme.h"
#include "PadDock.h"
#include "PadNav.h"
#include "Settings.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstring>
#include <string>

#include <windows.h>

namespace
{
	bool gXmlTabIndentPending = false;
	bool gXmlTabHeld = false;

	int XmlEditCb(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			auto* s = static_cast<std::string*>(data->UserData);
			s->resize(static_cast<size_t>(data->BufTextLen));
			data->Buf = s->data();
			return 0;
		}
		if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
		{
			/* Buf is unset here — only flag indent; insert in CallbackAlways. */
			if (data->EventChar == '\t')
			{
				gXmlTabIndentPending = true;
				return 1;
			}
			return 0;
		}
		if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
		{
			/* Nexus often does not map ImGuiKey_Tab; also catch VK_TAB edges. */
			const bool down = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;
			const bool edge = down && !gXmlTabHeld;
			gXmlTabHeld = down;
			if (edge &&
				(GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0 &&
				(GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0 &&
				(GetAsyncKeyState(VK_MENU) & 0x8000) == 0)
				gXmlTabIndentPending = true;

			if (gXmlTabIndentPending)
			{
				gXmlTabIndentPending = false;
				if (data->SelectionStart != data->SelectionEnd)
					data->DeleteChars(data->SelectionStart,
						data->SelectionEnd - data->SelectionStart);
				data->InsertChars(data->CursorPos, "   ");
			}
			TrailToolsDetail::gXmlEditCursor = data->CursorPos;
			return 0;
		}
		return 0;
	}

	void FillFromDraft()
	{
		using namespace TrailToolsDetail;
		gXmlEdit = TrailToolsXml::EmitOverlayData(gDraft);
		gXmlEditDirty = false;
		gXmlEditCursor = 0;
		SetStatus("Filled OverlayData from full draft (all trails + markers).");
	}
}

void TrailToolsDetail::InsertAtXmlCaret(const char* text)
{
	if (!text || !text[0])
		return;
	if (gXmlEdit.empty())
		gXmlEdit = TrailToolsXml::EmitOverlayData(gDraft);
	size_t pos = static_cast<size_t>(gXmlEditCursor);
	if (pos > gXmlEdit.size())
		pos = gXmlEdit.size();
	const size_t len = std::strlen(text);
	gXmlEdit.insert(pos, text, len);
	gXmlEditCursor = static_cast<int>(pos + len);
	gXmlEditDirty = true;
	CopyClipboard(text);
}

void TrailToolsDetail::DrawXmlEditorBody()
{
	if (gXmlEditDirty)
		ImGui::TextColored(HelperTheme::Warn, "Unsaved XML edits — Save writes this text as-is.");
	else
		ImGui::TextDisabled("Edit OverlayData freely. Extra attrs survive Save; Apply maps known fields.");

	if (ImGui::Button("Fill from draft###gw2tt_tt_xmlfill"))
		FillFromDraft();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Apply to editors"));
	if (ImGui::Button("Apply to editors###gw2tt_tt_xmlapply"))
	{
		if (!ApplyOverlayXml(gXmlEdit))
			SetStatus("Apply failed.");
		else
		{
			gDraft.xmlDirty = true;
			SetStatus("Applied XML to editors (%zu trails, %zu markers).",
				gDraft.trails.size(), gDraft.pois.size());
		}
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Copy"));
	if (ImGui::Button("Copy###gw2tt_tt_xmlcopyed"))
	{
		CopyClipboard(gXmlEdit.c_str());
		SetStatus("Copied XML.");
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Save"));
	if (PadNav::PrimaryButton("Save###gw2tt_tt_xmlsaveed"))
		SaveProjectXml(false);

	if (gXmlEdit.empty())
		gXmlEdit = TrailToolsXml::EmitOverlayData(gDraft);
	if (gXmlEdit.capacity() < gXmlEdit.size() + 256)
		gXmlEdit.reserve(gXmlEdit.size() + 1024);

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const float h = avail.y > 80.f ? avail.y : 80.f;
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
		ImGuiInputTextFlags_CallbackResize |
		ImGuiInputTextFlags_CallbackAlways |
		ImGuiInputTextFlags_CallbackCharFilter;
	if (ImGui::InputTextMultiline("###gw2tt_tt_xmlbuf", gXmlEdit.data(),
		gXmlEdit.capacity() + 1, ImVec2(avail.x > 8.f ? avail.x : 8.f, h),
		flags, XmlEditCb, &gXmlEdit))
		gXmlEditDirty = true;
}

void TrailToolsDetail::DrawXmlEditorPane(float height)
{
	if (gXmlEdit.empty())
		gXmlEdit = TrailToolsXml::EmitOverlayData(gDraft);
	if (gXmlEdit.capacity() < gXmlEdit.size() + 256)
		gXmlEdit.reserve(gXmlEdit.size() + 1024);
	const float h = height > 40.f ? height : 40.f;
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
		ImGuiInputTextFlags_CallbackResize |
		ImGuiInputTextFlags_CallbackAlways |
		ImGuiInputTextFlags_CallbackCharFilter;
	const float w = ImGui::GetContentRegionAvail().x;
	if (ImGui::InputTextMultiline("###gw2tt_tt_xmlbuf_hub", gXmlEdit.data(),
		gXmlEdit.capacity() + 1, ImVec2(w > 8.f ? w : 8.f, h),
		flags, XmlEditCb, &gXmlEdit))
		gXmlEditDirty = true;
}

bool TrailToolsDetail::RenderXmlEditorPad()
{
	if (!gShowXmlEdit)
		return false;

	G::PadGeom geom{};
	geom.x = gXmlEditX;
	geom.y = gXmlEditY;
	geom.w = gXmlEditW;
	geom.h = gXmlEditH;
	constexpr float kW = 640.f;
	constexpr float kH = 520.f;
	constexpr float kMinW = 420.f;
	if (geom.w > 1.f && geom.w < kMinW)
		geom.w = kMinW;

	const char* title = "OverlayData XML###GW2TrailToolsXmlEdit";
	const float maxH = PadDock::MaxH(320.f);
	PadDock::SetSizeConstraints(title, kMinW, 200.f, PadDock::MaxW(1100.f), maxH);
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);
	const ImVec2 fallback = PadDock::ClampPos(
		PadDock::BesideHelper(kW).x, PadDock::BesideHelper(kW).y + 48.f, kW);
	PadDock::Place(geom, gPlaceOnceXmlEdit, kW, kH, fallback);
	PadDock::ApplyCollapsedSize(title, geom.w >= 80.f ? geom.w : kW);
	if (!gPlaceOnceXmlEdit && geom.w < 80.f)
		ImGui::SetNextWindowSize(ImVec2(kW, kH), ImGuiCond_FirstUseEver);
	if (gFocusXmlEdit)
	{
		if (!EiRuntime::IsWine())
			ImGui::SetNextWindowFocus();
		gFocusXmlEdit = false;
	}

	bool open = gShowXmlEdit;
	HelperTheme::ScopedWindow theme(G::Opacity);
	ImGuiWindowFlags padFlags = HelperTheme::PadFlags(
		ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoScrollbar);
	if (PadDock::IsCustomCollapsed(title))
		padFlags |= ImGuiWindowFlags_NoResize;
	const bool padBody = ImGui::Begin(title, &open, padFlags);
	auto finishGeom = [&]() {
		gXmlEditX = geom.x;
		gXmlEditY = geom.y;
		gXmlEditW = geom.w;
		gXmlEditH = geom.h;
	};
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
		HelperTheme::EndPad();
		if (!open)
		{
			gShowXmlEdit = false;
			Settings::SetDirty();
		}
		finishGeom();
		return hovered || (focused && ImGui::GetIO().WantTextInput);
	}
	if (!ImGui::IsWindowCollapsed() && PadDock::Capture(geom))
		Settings::SetDirty();

	HelperTheme::ScopedFontScale fontScale(kW, kH);
	ImGui::BeginChild("###gw2tt_xml_body", ImVec2(0.f, -HelperTheme::ResizeGripClearance()),
		false, ImGuiWindowFlags_None);
	DrawXmlEditorBody();
	ImGui::EndChild();

	const bool hovered = ImGui::IsWindowHovered(
		ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
	const bool focusedWin = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	HelperTheme::EndPad();
	finishGeom();
	return hovered || (focusedWin && ImGui::GetIO().WantTextInput);
}

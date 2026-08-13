#include "Gw2Ui.h"

#include "Globals.h"
#include "HelperTheme.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <cstdio>
#include <cstring>
#include <unordered_set>

namespace
{
	std::unordered_set<int> gRequested;

	Texture_t* TexFor(int assetId)
	{
		if (!G::API || assetId <= 0)
			return nullptr;
		char id[48]{};
		std::snprintf(id, sizeof(id), "GW2TT_UI_%d", assetId);
		if (G::API->Textures_Get)
		{
			Texture_t* t = G::API->Textures_Get(id);
			if (t && t->Resource)
				return t;
		}
		return nullptr;
	}

	void EnsureRequest(int assetId)
	{
		if (!G::API || !G::API->Textures_GetOrCreateFromURL || assetId <= 0)
			return;
		if (gRequested.count(assetId))
			return;
		char id[48]{};
		char endpoint[48]{};
		std::snprintf(id, sizeof(id), "GW2TT_UI_%d", assetId);
		std::snprintf(endpoint, sizeof(endpoint), "/%d.png", assetId);
		G::API->Textures_GetOrCreateFromURL(id, "https://assets.gw2dat.com", endpoint);
		gRequested.insert(assetId);
	}

	bool ImageButtonTex(ImTextureID tex, float size)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 1.f, 1.f, 0.12f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 1.f, 0.22f));
		const bool clicked = ImGui::ImageButton(
			tex,
			ImVec2(size, size),
			ImVec2(0, 0), ImVec2(1, 1),
			0,
			ImVec4(0, 0, 0, 0),
			ImVec4(1, 1, 1, 1));
		ImGui::PopStyleColor(3);
		return clicked;
	}

	void VisibleTitle(const char* title, char* out, size_t outLen)
	{
		if (!out || outLen == 0)
			return;
		out[0] = 0;
		if (!title || !title[0])
		{
			std::snprintf(out, outLen, "Trail Tools");
			return;
		}
		const char* end = std::strstr(title, "###");
		if (!end)
			end = std::strstr(title, "##");
		if (!end)
		{
			std::snprintf(out, outLen, "%s", title);
			return;
		}
		const size_t n = static_cast<size_t>(end - title);
		if (n >= outLen)
		{
			std::memcpy(out, title, outLen - 1);
			out[outLen - 1] = 0;
			return;
		}
		std::memcpy(out, title, n);
		out[n] = 0;
	}
}

void Gw2Ui::Request(int assetId)
{
	EnsureRequest(assetId);
}

void Gw2Ui::WarmCommon()
{
	Request(Icon::Map);
	Request(Icon::Inventory);
	Request(Icon::Alert);
	Request(Icon::Bag);
	Request(Icon::Options);
	Request(Icon::TrailAnvil);
	Request(Icon::Close);
}

bool Gw2Ui::Image(int assetId, float size)
{
	EnsureRequest(assetId);
	Texture_t* t = TexFor(assetId);
	if (!t || !t->Resource)
		return false;
	ImGui::Image(reinterpret_cast<ImTextureID>(t->Resource), ImVec2(size, size));
	return true;
}

bool Gw2Ui::Image(Icon icon, float size)
{
	return Image(static_cast<int>(icon), size);
}

bool Gw2Ui::PaintPadChrome(float opacity, bool, bool, bool)
{
	const ImVec2 p0 = ImGui::GetWindowPos();
	const ImVec2 sz = ImGui::GetWindowSize();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec4 fill4 = HelperTheme::Bg;
	fill4.w = opacity;
	ImVec4 border4 = HelperTheme::Border;
	border4.w *= opacity;
	dl->AddRectFilled(p0, ImVec2(p0.x + sz.x, p0.y + sz.y),
		ImGui::ColorConvertFloat4ToU32(fill4), 6.f);
	dl->AddRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y),
		ImGui::ColorConvertFloat4ToU32(border4), 6.f, 0, 1.25f);
	return true;
}

void Gw2Ui::PaintNativeScrollbars(float, ImGuiWindow*)
{
}

bool Gw2Ui::DrawPadTitleBar(const char* title, bool* pOpen, float opacity, float, bool)
{
	ImGuiStorage* st = ImGui::GetStateStorage();
	const ImGuiID colId = ImGui::GetID("##gw2tt_pad_collapsed");
	const ImGuiID expWId = ImGui::GetID("##gw2tt_pad_exp_w");
	const ImGuiID expHId = ImGui::GetID("##gw2tt_pad_exp_h");
	bool collapsed = st->GetBool(colId, false);

	ImGuiWindow* win = ImGui::GetCurrentWindow();
	const ImVec2 win0 = win->Pos;
	const float winW = win->Size.x;
	/* WorkRect = content column (excludes WindowPadding + scrollbar gutter). */
	const float contentRight = win->WorkRect.Max.x;
	const float barH = collapsed ? 28.f : 36.f;
	const float btn = collapsed ? 18.f : 20.f;

	if (!collapsed)
	{
		const ImVec2 live = win->Size;
		if (live.x >= 80.f && live.y >= 80.f)
		{
			st->SetFloat(expWId, live.x);
			st->SetFloat(expHId, live.y);
		}
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec4 bar4 = HelperTheme::TitleBar;
	bar4.w = opacity;
	/* Title fill spans the full window (incl. gutter) so the bar stays solid. */
	dl->AddRectFilled(win0, ImVec2(win0.x + winW, win0.y + barH),
		ImGui::ColorConvertFloat4ToU32(bar4));
	ImVec4 line4 = HelperTheme::Accent;
	line4.w = opacity * 0.75f;
	dl->AddLine(
		ImVec2(win0.x + 1.f, win0.y + barH - 1.f),
		ImVec2(contentRight - 1.f, win0.y + barH - 1.f),
		ImGui::ColorConvertFloat4ToU32(line4), 1.5f);

	char vis[96]{};
	VisibleTitle(title, vis, sizeof(vis));

	ImGui::PushClipRect(win0, ImVec2(contentRight, win0.y + barH), false);

	const float edgePad = 8.f;
	const float btnGap = 4.f;
	const float clusterW = btn * 2.f + btnGap;
	const float clusterLeft = contentRight - edgePad - clusterW;

	ImGui::SetCursorScreenPos(win0);
	ImGui::InvisibleButton("##gw2tt_title_drag",
		ImVec2(ImMax(40.f, clusterLeft - win0.x - 4.f), barH));
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		const ImVec2 delta = ImGui::GetIO().MouseDelta;
		ImGui::SetWindowPos(ImVec2(win0.x + delta.x, win0.y + delta.y));
	}

	const ImVec2 titlePos(win0.x + 12.f, win0.y + (barH - ImGui::GetFontSize()) * 0.5f);
	dl->AddText(titlePos,
		ImGui::ColorConvertFloat4ToU32(HelperTheme::GoldBright), vis);

	const float btnY = win0.y + (barH - btn) * 0.5f;
	ImGui::SetCursorScreenPos(ImVec2(clusterLeft, btnY));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(btnGap, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::Panel);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HelperTheme::AccentHover);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, HelperTheme::AccentActive);
	ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::Ink);

	if (ImGui::Button(collapsed ? "+##ttmin" : "-##ttmin", ImVec2(btn, btn)))
	{
		if (!collapsed)
		{
			const ImVec2 live = win->Size;
			if (live.x >= 80.f && live.y >= 80.f)
			{
				st->SetFloat(expWId, live.x);
				st->SetFloat(expHId, live.y);
			}
			collapsed = true;
			st->SetBool(colId, true);
			ImGui::SetWindowSize(ImVec2(winW, barH));
		}
		else
		{
			collapsed = false;
			st->SetBool(colId, false);
			float w = st->GetFloat(expWId, 560.f);
			float h = st->GetFloat(expHId, 640.f);
			if (w < 200.f)
				w = 560.f;
			if (h < 120.f)
				h = 640.f;
			ImGui::SetWindowSize(ImVec2(w, h));
		}
	}
	ImGui::SameLine();
	if (pOpen && ImGui::Button("X##ttclose", ImVec2(btn, btn)))
		*pOpen = false;

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(2);
	ImGui::PopClipRect();

	if (!collapsed)
		ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, barH + 6.f));
	return !collapsed;
}

bool Gw2Ui::IconButton(const char* id, int assetId, float size)
{
	EnsureRequest(assetId);
	Texture_t* t = TexFor(assetId);
	ImGui::PushID(id);
	bool clicked = false;
	if (t && t->Resource)
		clicked = ImageButtonTex(reinterpret_cast<ImTextureID>(t->Resource), size);
	else
		clicked = ImGui::Button("?", ImVec2(size, size));
	ImGui::PopID();
	return clicked;
}

bool Gw2Ui::IconButton(const char* id, Icon icon, float size)
{
	return IconButton(id, static_cast<int>(icon), size);
}

bool Gw2Ui::IconLabelButton(const char* label, int assetId, float iconSize)
{
	EnsureRequest(assetId);
	ImGui::BeginGroup();
	Texture_t* t = TexFor(assetId);
	if (t && t->Resource)
	{
		ImGui::Image(reinterpret_cast<ImTextureID>(t->Resource), ImVec2(iconSize, iconSize));
		ImGui::SameLine();
	}
	const bool clicked = ImGui::Button(label);
	ImGui::EndGroup();
	return clicked;
}

bool Gw2Ui::IconLabelButton(const char* label, Icon icon, float iconSize)
{
	return IconLabelButton(label, static_cast<int>(icon), iconSize);
}

bool Gw2Ui::RailToggle(const char* label, bool on, int assetId, float iconSize, bool showLabel)
{
	EnsureRequest(assetId);
	ImGui::PushID(label);
	if (on)
		ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabActive);
	else
		ImGui::PushStyleColor(ImGuiCol_Button, HelperTheme::TabIdle);
	Texture_t* t = TexFor(assetId);
	bool clicked = false;
	const float pad = 6.f;
	if (t && t->Resource)
		clicked = ImageButtonTex(reinterpret_cast<ImTextureID>(t->Resource), iconSize);
	else
		clicked = ImGui::Button(showLabel ? label : "##r", ImVec2(iconSize + pad, iconSize + pad));
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered() && label && label[0])
	{
		const char* tip = label;
		const char* hash = std::strstr(label, "###");
		char buf[96]{};
		if (hash)
		{
			const size_t n = static_cast<size_t>(hash - label);
			if (n < sizeof(buf))
			{
				std::memcpy(buf, label, n);
				buf[n] = 0;
				tip = buf;
			}
		}
		ImGui::SetTooltip("%s", tip);
	}
	ImGui::PopID();
	return clicked;
}

bool Gw2Ui::RailToggle(const char* label, bool on, Icon icon, float iconSize, bool showLabel)
{
	return RailToggle(label, on, static_cast<int>(icon), iconSize, showLabel);
}

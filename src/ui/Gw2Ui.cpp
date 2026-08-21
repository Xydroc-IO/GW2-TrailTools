#include "Gw2Ui.h"

#include "Globals.h"
#include "HelperTheme.h"
#include "PadDock.h"
#include "UiScale.h"

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
	Request(Icon::Achievement);
	Request(Icon::SettingsGear);
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
	ImGuiWindow* win = ImGui::GetCurrentWindow();
	const ImVec2 p0 = ImGui::GetWindowPos();
	const ImVec2 sz = ImGui::GetWindowSize();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec4 fill4 = HelperTheme::Bg;
	fill4.w = opacity;
	ImVec4 border4 = HelperTheme::Border;
	border4.w *= opacity;
	const ImU32 fillU = ImGui::ColorConvertFloat4ToU32(fill4);
	const ImVec2 p1(p0.x + sz.x, p0.y + sz.y);
	dl->AddRectFilled(p0, p1, fillU, 6.f);
	/* Rounded fill AA can leave a clear slit before the Y scrollbar — seal it. */
	if (win && win->ScrollbarY)
	{
		const float gx0 = win->InnerRect.Max.x - 2.f;
		dl->AddRectFilled(ImVec2(gx0, p0.y), p1, fillU, 0.f);
	}
	dl->AddRect(p0, p1, ImGui::ColorConvertFloat4ToU32(border4), 6.f, 0, 1.25f);
	return true;
}

bool Gw2Ui::DrawPadTitleBar(const char* title, bool* pOpen, float opacity, float, bool)
{
	ImGuiStorage* st = ImGui::GetStateStorage();
	const ImGuiID colId = ImGui::GetID("##gw2tt_pad_collapsed");
	const ImGuiID expWId = ImGui::GetID("##gw2tt_pad_exp_w");
	const ImGuiID expHId = ImGui::GetID("##gw2tt_pad_exp_h");
	bool collapsed = st->GetBool(colId, false);

	ImGuiWindow* win = ImGui::GetCurrentWindow();
	PadDock::SetCustomCollapsedId(win->ID, collapsed);
	const ImVec2 win0 = win->Pos;
	const float winW = win->Size.x;
	const float winRight = win0.x + winW;
	const float barH = PadDock::TitleBarH(collapsed);
	/* Same faces / gap on Win + Wine — collapsed must not shrink the chrome. */
	const float btn = collapsed ? 22.f : 24.f;
	const float btnGap = 8.f;

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
	/* Full-width title wash (covers scrollbar gutter so it isn't a clear slit). */
	dl->AddRectFilled(win0, ImVec2(winRight, win0.y + barH),
		ImGui::ColorConvertFloat4ToU32(bar4));
	/* Expanded: sit left of a root scrollbar if present. Collapsed: always use
	   the outer edge — a leftover ScrollbarY gutter becomes the floating stub
	   after X on Windows. */
	const float contentRight = (!collapsed && win->ScrollbarY)
		? win->InnerRect.Max.x
		: winRight;
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

	const float btnY = win0.y + IM_FLOOR((barH - btn) * 0.5f + 0.5f);

	/* Geometry (not glyphs) — pixel-centered inside the face, inset from border. */
	auto chromeBtn = [&](const char* id, float x, int kind) -> bool {
		ImGui::SetCursorScreenPos(ImVec2(x, btnY));
		const bool hit = ImGui::InvisibleButton(id, ImVec2(btn, btn));
		const ImVec2 a = ImGui::GetItemRectMin();
		const ImVec2 b = ImGui::GetItemRectMax();
		const bool hov = ImGui::IsItemHovered();
		const bool act = ImGui::IsItemActive();
		ImVec4 fill = act ? HelperTheme::AccentActive
			: (hov ? HelperTheme::AccentHover : HelperTheme::Panel);
		fill.w *= opacity;
		ImVec4 stroke = HelperTheme::BorderSoft;
		stroke.w *= opacity;
		dl->AddRectFilled(a, b, ImGui::ColorConvertFloat4ToU32(fill), 3.f);
		dl->AddRect(a, b, ImGui::ColorConvertFloat4ToU32(stroke), 3.f, 0, 1.f);
		/* Inset so AA stroke doesn't eat the glyph; snap to pixel centers. */
		const float cx = IM_FLOOR((a.x + b.x) * 0.5f) + 0.5f;
		const float cy = IM_FLOOR((a.y + b.y) * 0.5f) + 0.5f;
		const float arm = IM_FLOOR(btn * 0.34f) + 0.5f;
		ImVec4 ink = HelperTheme::Ink;
		ink.w *= opacity;
		const ImU32 ic = ImGui::ColorConvertFloat4ToU32(ink);
		const float th = 2.f;
		if (kind == 0) /* minus */
			dl->AddLine(ImVec2(cx - arm, cy), ImVec2(cx + arm, cy), ic, th);
		else if (kind == 1) /* plus */
		{
			dl->AddLine(ImVec2(cx - arm, cy), ImVec2(cx + arm, cy), ic, th);
			dl->AddLine(ImVec2(cx, cy - arm), ImVec2(cx, cy + arm), ic, th);
		}
		else /* X */
		{
			dl->AddLine(ImVec2(cx - arm, cy - arm), ImVec2(cx + arm, cy + arm), ic, th);
			dl->AddLine(ImVec2(cx + arm, cy - arm), ImVec2(cx - arm, cy + arm), ic, th);
		}
		return hit;
	};

	if (chromeBtn("##ttmin", clusterLeft, collapsed ? 1 : 0))
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
			PadDock::SetCustomCollapsedId(win->ID, true);
			win->Scroll = ImVec2(0.f, 0.f);
			win->ScrollMax = ImVec2(0.f, 0.f);
			win->ScrollbarSizes = ImVec2(0.f, 0.f);
			win->SizeFull = ImVec2(winW, barH);
			win->Size = win->SizeFull;
		}
		else
		{
			collapsed = false;
			st->SetBool(colId, false);
			PadDock::SetCustomCollapsedId(win->ID, false);
			float w = st->GetFloat(expWId, 560.f);
			float h = st->GetFloat(expHId, 640.f);
			if (w < 200.f)
				w = 560.f;
			if (h < 120.f)
				h = 640.f;
			win->SizeFull = ImVec2(w, h);
			win->Size = win->SizeFull;
		}
	}
	if (pOpen && chromeBtn("##ttclose", clusterLeft + btn + btnGap, 2))
		*pOpen = false;

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

	if (iconSize < 12.f)
		iconSize = UiScale::RailIconSize(40.f);
	const float pad = (iconSize * 0.12f < 4.f) ? 4.f : (iconSize * 0.12f > 10.f ? 10.f : iconSize * 0.12f);
	const float cell = iconSize + pad * 2.f;
	const float avail = ImGui::GetContentRegionAvail().x;
	if (avail > cell + 1.f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - cell) * 0.5f);

	const ImVec2 btnSz(cell, cell);
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const bool clicked = ImGui::InvisibleButton("##rail", btnSz);
	const bool hovered = ImGui::IsItemHovered();
	const bool held = ImGui::IsItemActive();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec4 bg4 = on ? HelperTheme::TabActive : HelperTheme::TabIdle;
	if (held)
		bg4 = HelperTheme::AccentActive;
	else if (hovered && !on)
		bg4 = HelperTheme::TabActive;
	dl->AddRectFilled(p0, ImVec2(p0.x + cell, p0.y + cell),
		ImGui::ColorConvertFloat4ToU32(bg4), 4.f);
	dl->AddRect(p0, ImVec2(p0.x + cell, p0.y + cell),
		ImGui::ColorConvertFloat4ToU32(HelperTheme::BorderSoft), 4.f, 0, 1.f);

	const float img = iconSize;
	const float ox = (cell - img) * 0.5f;
	const float oy = (cell - img) * 0.5f;
	const ImVec2 i0(p0.x + ox, p0.y + oy);
	const ImVec2 i1(i0.x + img, i0.y + img);
	Texture_t* t = TexFor(assetId);
	if (t && t->Resource)
	{
		/* Selected tab: full-bright icon; idle: slightly dimmer for contrast. */
		const ImU32 tint = on ? IM_COL32(255, 255, 255, 255)
			: IM_COL32(180, 190, 200, 210);
		dl->AddImage(reinterpret_cast<ImTextureID>(t->Resource), i0, i1,
			ImVec2(0, 0), ImVec2(1, 1), tint);
	}
	else if (showLabel && label && label[0])
	{
		const char* tip = label;
		const char* hash = std::strstr(label, "###");
		char shortLab[32]{};
		if (hash)
		{
			const size_t n = static_cast<size_t>(hash - label);
			const size_t m = n < sizeof(shortLab) - 1 ? n : sizeof(shortLab) - 1;
			std::memcpy(shortLab, label, m);
			shortLab[m] = 0;
			tip = shortLab;
		}
		const ImVec2 ts = ImGui::CalcTextSize(tip);
		dl->AddText(ImVec2(p0.x + (cell - ts.x) * 0.5f, p0.y + (cell - ts.y) * 0.5f),
			ImGui::ColorConvertFloat4ToU32(HelperTheme::Ink), tip);
	}
	else
	{
		dl->AddText(ImVec2(p0.x + cell * 0.35f, p0.y + cell * 0.28f),
			ImGui::ColorConvertFloat4ToU32(HelperTheme::Muted), "?");
	}

	if (hovered && label && label[0])
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

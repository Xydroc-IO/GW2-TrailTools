#include "Gw2Ui.h"
#include "HelperTheme.h"
#include "PadDock.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

void Gw2Ui::PaintNativeScrollbars(float opacity, ImGuiWindow* root)
{
	if (!root)
		root = ImGui::GetCurrentWindow();
	if (!root || root->Collapsed)
		return;
	/* Custom minimize is title-only — never paint a gutter stub beside +/X. */
	if (root->StateStorage.GetBool(root->GetID("##gw2tt_pad_collapsed"), false))
		return;

	float a = opacity;
	if (a < 0.05f)
		a = 0.05f;
	if (a > 1.f)
		a = 1.f;

	auto paintY = [&](ImGuiWindow* window) {
		if (!window || window->Collapsed || !window->ScrollbarY)
			return;
		if (!(window->Active || window->WasActive))
			return;
		ImDrawList* dl = window->DrawList;
		if (!dl)
			return;

		const ImRect outer = window->Rect();
		ImRect bb = ImGui::GetWindowScrollbarRect(window, ImGuiAxis_Y);
		const bool col = window->StateStorage.GetBool(
			window->GetID("##gw2tt_pad_collapsed"), false);
		const float titleH = PadDock::TitleBarH(col);
		/* Title wash covers gutter under min/close; paint body gutter below. */
		const float top = window->Pos.y + titleH;
		bb.Min.y = ImMax(bb.Min.y, top);
		if (bb.GetHeight() < 2.f)
			return;

		/* Opaque column content→outer (NoBackground otherwise shows through). */
		const float sealL = ImMin(window->InnerRect.Max.x, bb.Min.x) - 3.f;
		const ImU32 sealU = IM_COL32(
			(int)(HelperTheme::Bg.x * 255.f + 0.5f),
			(int)(HelperTheme::Bg.y * 255.f + 0.5f),
			(int)(HelperTheme::Bg.z * 255.f + 0.5f),
			(int)(ImClamp(a, 0.95f, 1.f) * 255.f + 0.5f));
		dl->PushClipRect(ImVec2(sealL, top), ImVec2(outer.Max.x, outer.Max.y), false);
		dl->AddRectFilled(ImVec2(sealL, top), ImVec2(outer.Max.x, bb.Max.y), sealU, 0.f);

		const float trackH = bb.Max.y - top;
		if (trackH < 2.f)
		{
			dl->PopClipRect();
			return;
		}
		const float winH = window->InnerRect.GetHeight();
		const float contentH = winH + window->ScrollMax.y;
		float grabH = (contentH > 1.f) ? (winH / contentH) * trackH : trackH;
		grabH = ImClamp(grabH, 18.f, trackH);
		const float travel = ImMax(0.f, trackH - grabH);
		const float t = (window->ScrollMax.y > 0.f)
			? (window->Scroll.y / window->ScrollMax.y)
			: 0.f;
		const float gy0 = top + travel * t;
		/* Match track presence on Win/Linux — 3px reads as a hairline on many
		   Windows DPI setups while Linux/Wine still looks fine at ~6. */
		const float trackW = ImMax(bb.GetWidth(), ImGui::GetStyle().ScrollbarSize);
		const float thumbW = ImClamp(trackW * 0.45f, 5.f, 8.f);
		const float gx1 = outer.Max.x - 2.f;
		const float gx0 = gx1 - thumbW;
		ImVec4 thumb = HelperTheme::Gold;
		thumb.w *= a;
		dl->AddRectFilled(ImVec2(gx0, gy0), ImVec2(gx1, gy0 + grabH),
			ImGui::ColorConvertFloat4ToU32(thumb), 1.5f);
		dl->PopClipRect();
	};

	paintY(root);
	ImGuiContext& g = *GImGui;
	for (int i = 0; i < g.Windows.Size; ++i)
	{
		ImGuiWindow* w = g.Windows[i];
		if (!w || w == root)
			continue;
		bool under = false;
		for (ImGuiWindow* p = w; p; p = p->ParentWindow)
		{
			if (p == root)
			{
				under = true;
				break;
			}
		}
		if (under)
			paintY(w);
	}
}

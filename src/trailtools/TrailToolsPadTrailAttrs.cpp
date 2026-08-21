#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsAssets.h"
#include "TrailToolsDraftStyle.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

void TrailToolsDetail::DrawTrailAttrsSection()
{
	PadNav::SectionTitle("Trail attrs");
	PadNav::BeginSection("trl_attrs");
	ImGui::TextDisabled("Per-trail texture for this Trails window (category Looks if unset).");

	static std::vector<TrailToolsAssets::Entry> sTex;
	static bool sTexLoaded = false;
	if (!sTexLoaded)
	{
		TrailToolsAssets::RefreshAuthoringList(sTex);
		sTexLoaded = true;
	}

	const auto sty = TrailToolsDraftStyle::ResolveTrail();
	const char* effective = !gDraft.active.texture.empty()
		? gDraft.active.texture.c_str()
		: (!sty.textureRel.empty() ? sty.textureRel.c_str() : "(category default)");
	ImGui::TextColored(HelperTheme::Muted, "In use: %s", effective);

	const char* preview = gDraft.active.texture.empty()
		? "(category default)" : gDraft.active.texture.c_str();
	PadNav::SetFullRowWidth();
	if (ImGui::BeginCombo("###gw2tt_tt_ttexpick", preview))
	{
		if (ImGui::Selectable("(category default)", gDraft.active.texture.empty()))
		{
			gDraft.active.texture.clear();
			MarkDirty();
		}
		for (size_t i = 0; i < sTex.size(); ++i)
		{
			const bool sel = gDraft.active.texture == sTex[i].relPath;
			if (ImGui::Selectable(sTex[i].label.c_str(), sel))
			{
				gDraft.active.texture = sTex[i].relPath;
				MarkDirty();
			}
			if (sel)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Refresh"));
	if (ImGui::SmallButton("Refresh###gw2tt_tt_ttexref"))
	{
		TrailToolsAssets::RefreshAuthoringList(sTex);
		sTexLoaded = true;
	}

	char tex[256]{};
	std::snprintf(tex, sizeof(tex), "%s", gDraft.active.texture.c_str());
	PadNav::PushWidthForLabel("texture###gw2tt_tt_ttex");
	if (ImGui::InputText("texture###gw2tt_tt_ttex", tex, sizeof(tex)))
	{
		gDraft.active.texture = tex;
		MarkDirty();
	}
	PadNav::PopWidthForLabel();
	PadNav::PrepLabeled("trailScale###gw2tt_tt_ats", 100.f, true);
	if (ImGui::DragFloat("trailScale###gw2tt_tt_ats", &gDraft.active.trailScale, 0.05f, 0.25f, 4.f))
		MarkDirty();
	PadNav::PrepLabeled("animSpeed###gw2tt_tt_aanim", 100.f);
	if (ImGui::DragFloat("animSpeed###gw2tt_tt_aanim", &gDraft.active.animSpeed, 0.05f, 0.1f, 8.f))
		MarkDirty();
	PadNav::PrepLabeled("alpha###gw2tt_tt_aalpha", 100.f);
	if (ImGui::DragFloat("alpha###gw2tt_tt_aalpha", &gDraft.active.alpha, 0.01f, 0.05f, 1.f))
		MarkDirty();
	PadNav::PrepLabeled("fadeNear###gw2tt_tt_afn", 100.f);
	if (ImGui::DragFloat("fadeNear###gw2tt_tt_afn", &gDraft.active.fadeNear, 10.f, -1.f, 20000.f))
		MarkDirty();
	PadNav::PrepLabeled("fadeFar###gw2tt_tt_aff", 100.f);
	if (ImGui::DragFloat("fadeFar###gw2tt_tt_aff", &gDraft.active.fadeFar, 10.f, -1.f, 20000.f))
		MarkDirty();
	PadNav::EndSection();
}

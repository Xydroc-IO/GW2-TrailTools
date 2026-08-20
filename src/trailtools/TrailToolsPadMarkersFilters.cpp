#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace
{
	const char* BehaviorName(int b)
	{
		switch (b)
		{
		case 0: return "Always visible";
		case 1: return "Reappear on map change";
		case 2: return "Reappear on daily reset";
		case 3: return "Only before activation";
		case 4: return "Only after activation";
		case 5: return "Reappear after timer";
		case 6: return "Reappear on map reset";
		case 7: return "Daily reset or timer";
		default: return "Custom / other";
		}
	}
}

void TrailToolsDetail::DrawPoiBehaviorAndFilters(DraftPoi& p)
{
	PadNav::PrepLabeled("behavior###gw2tt_tt_pbeh", 80.f, true);
	ImGui::InputInt("behavior###gw2tt_tt_pbeh", &p.behavior);
	const char* curName = BehaviorName(p.behavior);
	PadNav::SetFullRowWidth();
	if (ImGui::BeginCombo("###gw2tt_tt_pbehcombo", curName))
	{
		for (int i = 0; i <= 7; ++i)
		{
			const bool sel = p.behavior == i;
			if (ImGui::Selectable(BehaviorName(i), sel))
				p.behavior = i;
			if (sel)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	PadNav::WrapSameLine(PadNav::CheckboxWidth("autoTrigger###gw2tt_tt_patr"));
	ImGui::Checkbox("autoTrigger###gw2tt_tt_patr", &p.autoTrigger);
	PadNav::PrepLabeled("triggerRange###gw2tt_tt_ptr", 100.f, true);
	ImGui::DragFloat("triggerRange###gw2tt_tt_ptr", &p.triggerRange, 0.1f, 0.f, 50.f);

	if (ImGui::CollapsingHeader("Display###gw2tt_tt_pdisp"))
	{
		PadNav::PrepLabeled("mapDisplaySize###gw2tt_tt_pmds", 110.f, true);
		ImGui::DragFloat("mapDisplaySize###gw2tt_tt_pmds", &p.mapDisplaySize, 0.5f, 1.f, 256.f);
		PadNav::PrepLabeled("minSize###gw2tt_tt_pmins", 100.f);
		ImGui::DragFloat("minSize###gw2tt_tt_pmins", &p.minSize, 0.5f, 1.f, 512.f);
		PadNav::PrepLabeled("maxSize###gw2tt_tt_pmaxs", 100.f);
		ImGui::DragFloat("maxSize###gw2tt_tt_pmaxs", &p.maxSize, 1.f, 1.f, 4096.f);
		PadNav::PrepLabeled("fadeNear###gw2tt_tt_pfn", 100.f);
		ImGui::DragFloat("fadeNear###gw2tt_tt_pfn", &p.fadeNear, 10.f, -1.f, 20000.f);
		PadNav::PrepLabeled("fadeFar###gw2tt_tt_pff", 100.f);
		ImGui::DragFloat("fadeFar###gw2tt_tt_pff", &p.fadeFar, 10.f, -1.f, 20000.f);
	}

	if (ImGui::CollapsingHeader("Filters###gw2tt_tt_pfilt"))
	{
		ImGui::TextDisabled("Omit filters with −1 / 0 / empty.");
		PadNav::PrepLabeled("achievementId###gw2tt_tt_paid", 110.f, true);
		ImGui::InputInt("achievementId###gw2tt_tt_paid", &p.achievementId);
		PadNav::PrepLabeled("achievementBit###gw2tt_tt_pabit", 110.f);
		ImGui::InputInt("achievementBit###gw2tt_tt_pabit", &p.achievementBit);
		char fest[96]{}, tog[160]{};
		std::snprintf(fest, sizeof(fest), "%s", p.festival.c_str());
		std::snprintf(tog, sizeof(tog), "%s", p.toggleCategory.c_str());
		PadNav::PushWidthForLabel("festival###gw2tt_tt_pfest");
		if (ImGui::InputText("festival###gw2tt_tt_pfest", fest, sizeof(fest)))
			p.festival = fest;
		PadNav::PopWidthForLabel();
		PadNav::PrepLabeled("profession###gw2tt_tt_pprof", 100.f, true);
		ImGui::InputInt("profession###gw2tt_tt_pprof", &p.profession);
		PadNav::PrepLabeled("race###gw2tt_tt_prace", 100.f);
		ImGui::InputInt("race###gw2tt_tt_prace", &p.race);
		PadNav::PrepLabeled("mount###gw2tt_tt_pmount", 100.f);
		ImGui::InputInt("mount###gw2tt_tt_pmount", &p.mount);
		PadNav::PushWidthForLabel("toggleCategory###gw2tt_tt_ptog");
		if (ImGui::InputText("toggleCategory###gw2tt_tt_ptog", tog, sizeof(tog)))
			p.toggleCategory = tog;
		PadNav::PopWidthForLabel();

		char tip[96]{}, tipd[384]{}, info[384]{}, copy[256]{}, cmsg[128]{};
		char sched[96]{}, icon[256]{};
		std::snprintf(tip, sizeof(tip), "%s", p.tipName.c_str());
		std::snprintf(tipd, sizeof(tipd), "%s", p.tipDescription.c_str());
		std::snprintf(info, sizeof(info), "%s", p.info.c_str());
		std::snprintf(copy, sizeof(copy), "%s", p.copy.c_str());
		std::snprintf(cmsg, sizeof(cmsg), "%s", p.copyMessage.c_str());
		std::snprintf(sched, sizeof(sched), "%s", p.schedule.c_str());
		std::snprintf(icon, sizeof(icon), "%s", p.iconFile.c_str());
		PadNav::PushWidthForLabel("tip-name###gw2tt_tt_ptn");
		if (ImGui::InputText("tip-name###gw2tt_tt_ptn", tip, sizeof(tip)))
			p.tipName = tip;
		PadNav::PopWidthForLabel();
		PadNav::PushWidthForLabel("tip-description###gw2tt_tt_ptd");
		if (ImGui::InputText("tip-description###gw2tt_tt_ptd", tipd, sizeof(tipd)))
			p.tipDescription = tipd;
		PadNav::PopWidthForLabel();
		PadNav::PushWidthForLabel("info###gw2tt_tt_pinfo");
		if (ImGui::InputText("info###gw2tt_tt_pinfo", info, sizeof(info)))
			p.info = info;
		PadNav::PopWidthForLabel();
		PadNav::PushWidthForLabel("copy###gw2tt_tt_pcopy");
		if (ImGui::InputText("copy###gw2tt_tt_pcopy", copy, sizeof(copy)))
			p.copy = copy;
		PadNav::PopWidthForLabel();
		PadNav::PushWidthForLabel("copy-message###gw2tt_tt_pcmsg");
		if (ImGui::InputText("copy-message###gw2tt_tt_pcmsg", cmsg, sizeof(cmsg)))
			p.copyMessage = cmsg;
		PadNav::PopWidthForLabel();
		PadNav::PushWidthForLabel("schedule###gw2tt_tt_psched");
		if (ImGui::InputText("schedule###gw2tt_tt_psched", sched, sizeof(sched)))
			p.schedule = sched;
		PadNav::PopWidthForLabel();
		PadNav::PrepLabeled("schedule-duration###gw2tt_tt_psd", 120.f, true);
		ImGui::DragFloat("schedule-duration###gw2tt_tt_psd", &p.scheduleDuration,
			1.f, 0.f, 10080.f);
		PadNav::PushWidthForLabel("iconFile###gw2tt_tt_picon");
		if (ImGui::InputText("iconFile###gw2tt_tt_picon", icon, sizeof(icon)))
			p.iconFile = icon;
		PadNav::PopWidthForLabel();
	}
}

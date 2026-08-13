#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <string>
#include <vector>

/* Combined Trails + Markers hub tab (project lists → open TrailsN / MarkersN). */
void TrailToolsDetail::DrawContentTab()
{
	EnsureWorkspace();
	SyncActiveType();
	if (gDraft.active.fileRel.empty())
		SyncActiveFileRelFromStem();

	PadNav::SectionTitle("Trail default");
	PadNav::BeginSection("content_trl_cat");
	{
		std::vector<std::string> leaves;
		CollectLeafPaths(gDraft.root, "", leaves, true);
		if (leaves.empty() && gDraft.trailType[0])
			leaves.push_back(gDraft.trailType);
		int cur = 0;
		for (size_t i = 0; i < leaves.size(); ++i)
		{
			if (leaves[i] == gDraft.trailType)
			{
				cur = static_cast<int>(i);
				break;
			}
		}
		const char* preview = leaves.empty() ? (gDraft.trailType[0] ? gDraft.trailType : "(none)")
			: leaves[static_cast<size_t>(cur)].c_str();
		PadNav::SetFullRowWidth();
		if (ImGui::BeginCombo("###gw2tt_tt_trltype", preview))
		{
			for (size_t i = 0; i < leaves.size(); ++i)
			{
				const bool sel = static_cast<int>(i) == cur;
				if (ImGui::Selectable(leaves[i].c_str(), sel))
				{
					std::snprintf(gDraft.trailType, sizeof(gDraft.trailType), "%s",
						leaves[i].c_str());
					gDraft.active.type = gDraft.trailType;
					MarkDirty();
				}
				if (sel)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		PadNav::InputCaption("Or type path", "gw2tt_tt_trltype_edit",
			gDraft.trailType, sizeof(gDraft.trailType));
	}
	PadNav::EndSection();

	PadNav::SectionTitle("Trails in project");
	PadNav::BeginSection("content_trails");
	if (ImGui::BeginChild("###gw2tt_tt_tlist", ImVec2(0.f, 110.f), true))
	{
		for (int i = 0; i < static_cast<int>(gDraft.trails.size()); ++i)
		{
			const DraftTrail& t = gDraft.trails[static_cast<size_t>(i)];
			ImGui::PushID(i);
			char lab[200]{};
			std::snprintf(lab, sizeof(lab), "%s  map %u  %zu pts",
				t.fileRel.c_str(), t.mapId, t.points.size());
			if (ImGui::Selectable(lab, gDraft.selectedTrail == i))
			{
				gDraft.selectedTrail = i;
				gDraft.active = t;
				std::snprintf(gDraft.trailType, sizeof(gDraft.trailType), "%s", t.type.c_str());
				ApplyStemFromFileRel();
				gDraft.selectedPoint = -1;
				gDraft.trailDirty = false;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Win") && !gHubSkipOpenClicks)
			{
				const int slot = OpenNewTrailEditor();
				if (slot >= 0)
				{
					TrailEditorSlot& s = gTrailEditors[slot];
					s.trail = t;
					const size_t slash = t.fileRel.find_last_of('/');
					std::string stem = slash == std::string::npos ? t.fileRel
						: t.fileRel.substr(slash + 1);
					if (stem.size() > 4 && stem.compare(stem.size() - 4, 4, ".trl") == 0)
						stem.resize(stem.size() - 4);
					std::snprintf(s.stem, sizeof(s.stem), "%s", stem.c_str());
					s.dirty = false;
					s.selectedPoint = -1;
				}
			}
			ImGui::PopID();
		}
		if (gDraft.trails.empty())
			ImGui::TextDisabled("No trails in XML yet.");
	}
	ImGui::EndChild();
	if (ImGui::Button("New trail window###gw2tt_tt_open_trn") && !gHubSkipOpenClicks)
		OpenNewTrailEditor();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add to project"));
	if (PadNav::PrimaryButton("Add to project###gw2tt_tt_ins_trxml"))
		UpsertActiveTrailInPack();
	ImGui::Dummy(ImVec2(0.f, 2.f));
	for (int i = 0; i < kMaxTrailEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sTrails%d###gw2tt_tt_trslot%d",
			gTrailEditors[i].open ? "*" : "", i + 1, i);
		if (i > 0)
			PadNav::WrapSameLine(PadNav::ButtonWidth(lab));
		if (ImGui::SmallButton(lab) && !gHubSkipOpenClicks)
			OpenTrailEditorSlot(i);
	}
	PadNav::EndSection();

	PadNav::SectionTitle("Marker default");
	PadNav::BeginSection("content_mk_cat");
	{
		std::vector<std::string> leaves;
		CollectLeafPaths(gDraft.root, "", leaves, false);
		if (leaves.empty())
			leaves.push_back(gDraft.markerType[0] ? gDraft.markerType : "examplepack.circle");
		int cur = 0;
		for (size_t i = 0; i < leaves.size(); ++i)
		{
			if (leaves[i] == gDraft.markerType)
			{
				cur = static_cast<int>(i);
				break;
			}
		}
		PadNav::SetFullRowWidth();
		if (ImGui::BeginCombo("###gw2tt_tt_mtype", leaves[static_cast<size_t>(cur)].c_str()))
		{
			for (size_t i = 0; i < leaves.size(); ++i)
			{
				const bool sel = static_cast<int>(i) == cur;
				if (ImGui::Selectable(leaves[i].c_str(), sel))
					std::snprintf(gDraft.markerType, sizeof(gDraft.markerType), "%s",
						leaves[i].c_str());
				if (sel)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		PadNav::InputCaption("Or type path", "gw2tt_tt_mtype_edit",
			gDraft.markerType, sizeof(gDraft.markerType));
	}
	PadNav::EndSection();

	PadNav::SectionTitle("Markers in project");
	PadNav::BeginSection("content_markers");
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	const bool pose = ReadMumblePose(mapId, x, y, z);
	static bool sThisMapOnly = true;
	ImGui::Checkbox("This map only###gw2tt_tt_mmap", &sThisMapOnly);
	if (ImGui::BeginChild("###gw2tt_tt_mlist", ImVec2(0.f, 110.f), true))
	{
		for (int i = 0; i < static_cast<int>(gDraft.pois.size()); ++i)
		{
			const DraftPoi& p = gDraft.pois[static_cast<size_t>(i)];
			if (sThisMapOnly && pose && p.mapId != mapId)
				continue;
			ImGui::PushID(1000 + i);
			char label[256]{};
			std::snprintf(label, sizeof(label), "%d  map %u  %s", i, p.mapId, p.type.c_str());
			if (ImGui::Selectable(label, gDraft.selectedPoi == i))
				gDraft.selectedPoi = i;
			ImGui::SameLine();
			if (ImGui::SmallButton("Win") && !gHubSkipOpenClicks)
				OpenMarkerEditor(i, true);
			ImGui::PopID();
		}
		if (gDraft.pois.empty())
			ImGui::TextDisabled("No markers in XML yet.");
	}
	ImGui::EndChild();
	if (ImGui::Button("New marker window###gw2tt_tt_open_mkn") && !gHubSkipOpenClicks)
		OpenNewMarkerEditor();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add to project"));
	if (PadNav::PrimaryButton("Add to project###gw2tt_tt_ins_mkxml"))
		UpsertSelectedPoiInPack();
	for (int i = 0; i < kMaxMarkerEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sMarkers%d###gw2tt_tt_mkslot%d",
			gMarkerEditors[i].open ? "*" : "", i + 1, i);
		if (i == 0)
			ImGui::Dummy(ImVec2(0.f, 2.f));
		else
			PadNav::WrapSameLine(PadNav::ButtonWidth(lab));
		if (ImGui::SmallButton(lab) && !gHubSkipOpenClicks)
		{
			if (gMarkerEditors[i].open)
				gMarkerEditors[i].focus = true;
			else if (gDraft.selectedPoi >= 0)
				OpenMarkerEditor(gDraft.selectedPoi, true);
			else
				OpenNewMarkerEditor();
		}
	}
	PadNav::EndSection();

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

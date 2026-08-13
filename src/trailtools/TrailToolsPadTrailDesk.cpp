#include "TrailToolsInternal.h"
#include "TrailToolsPad.h"
#include "TrailToolsShared.h"
#include "TrailToolsXml.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{
	void DrawTrailList()
	{
		using namespace TrailToolsDetail;
		if (ImGui::BeginChild("###gw2tt_tt_tlist", ImVec2(0.f, 120.f), true))
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
					SetStatus("Editing trail %s", t.fileRel.c_str());
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Win"))
				{
					const int slot = OpenNewTrailEditor();
					if (slot >= 0)
					{
						TrailEditorSlot& s = gTrailEditors[slot];
						s.trail = t;
						ApplyStemFromFileRel(); /* uses active — sync stem from t */
						{
							const size_t slash = t.fileRel.find_last_of('/');
							std::string stem = slash == std::string::npos ? t.fileRel
								: t.fileRel.substr(slash + 1);
							if (stem.size() > 4 && stem.compare(stem.size() - 4, 4, ".trl") == 0)
								stem.resize(stem.size() - 4);
							std::snprintf(s.stem, sizeof(s.stem), "%s", stem.c_str());
						}
						s.dirty = false;
						s.selectedPoint = -1;
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Del"))
				{
					gDraft.trails.erase(gDraft.trails.begin() + i);
					if (gDraft.selectedTrail == i)
						gDraft.selectedTrail = -1;
					ImGui::PopID();
					break;
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}

	void DrawDefaultCategory()
	{
		using namespace TrailToolsDetail;
		ImGui::TextColored(HelperTheme::Muted, "Category");
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
		}
		PadNav::InputCaption("Or type path", "gw2tt_tt_trltype_edit",
			gDraft.trailType, sizeof(gDraft.trailType));
		if (ImGui::IsItemDeactivatedAfterEdit() && gDraft.trailType[0])
		{
			gDraft.active.type = gDraft.trailType;
			MarkDirty();
		}
	}

}

void TrailToolsDetail::SyncActiveType()
{
	if (gDraft.trailType[0])
		gDraft.active.type = gDraft.trailType;
}

void TrailToolsDetail::SyncActiveFileRelFromStem()
{
	const char* stem = gDraft.trailFileStem[0] ? gDraft.trailFileStem : "Trail";
	gDraft.active.fileRel = std::string("Data/") + gDraft.packName + "/Trails/" + stem + ".trl";
}

void TrailToolsDetail::ApplyStemFromFileRel()
{
	const size_t slash = gDraft.active.fileRel.find_last_of('/');
	std::string stem = slash == std::string::npos ? gDraft.active.fileRel
		: gDraft.active.fileRel.substr(slash + 1);
	if (stem.size() > 4 && stem.compare(stem.size() - 4, 4, ".trl") == 0)
		stem.resize(stem.size() - 4);
	std::snprintf(gDraft.trailFileStem, sizeof(gDraft.trailFileStem), "%s", stem.c_str());
}

void TrailToolsDetail::MarkDirty()
{
	gDraft.trailDirty = true;
}

void TrailToolsDetail::DrawTrailDesk(bool asPopout)
{
	EnsureWorkspace();
	SyncActiveType();
	if (gDraft.active.fileRel.empty())
		SyncActiveFileRelFromStem();

	PadNav::SectionTitle("Category");
	PadNav::BeginSection("trails_cat");
	DrawDefaultCategory();
	PadNav::EndSection();

	PadNav::SectionTitle("In pack");
	PadNav::BeginSection("trails_list");
	DrawTrailList();
	ImGui::Dummy(ImVec2(0.f, 6.f));
	if (ImGui::Button("New trail window###gw2tt_tt_open_trn"))
		OpenNewTrailEditor();
	if (!asPopout)
	{
		PadNav::WrapSameLine(PadNav::ButtonWidth("Pop out"));
		if (ImGui::Button("Pop out###gw2tt_tt_open_trdesk"))
			TrailToolsPad::OpenTrailsDesk();
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Copy XML"));
	{
		const std::string line = TrailToolsXml::EmitTrailElement(gDraft.active);
		if (!line.empty() && ImGui::Button("Copy XML###gw2tt_tt_copytrxml2"))
		{
			CopyClipboard(line.c_str());
			SetStatus("Copied Trail XML line.");
		}
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add to project"));
	if (PadNav::PrimaryButton("Add to project###gw2tt_tt_ins_trxml"))
		UpsertActiveTrailInPack();
	ImGui::Dummy(ImVec2(0.f, 4.f));
	PadNav::PushWrap();
	ImGui::TextDisabled("%s%s · %zu pts · map %u",
		gDraft.active.fileRel.empty() ? "(no trail)" : gDraft.active.fileRel.c_str(),
		gDraft.trailDirty ? " *" : "",
		gDraft.active.points.size(), gDraft.active.mapId);
	PadNav::PopWrap();
	PadNav::EndSection();

	PadNav::SectionTitle("Editors");
	PadNav::BeginSection("trails_slots");
	for (int i = 0; i < kMaxTrailEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sTrails%d###gw2tt_tt_trslot%d",
			gTrailEditors[i].open ? "*" : "", i + 1, i);
		if (i > 0)
			PadNav::WrapSameLine(PadNav::ButtonWidth(lab));
		if (ImGui::SmallButton(lab))
			OpenTrailEditorSlot(i);
	}
	PadNav::EndSection();

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

void TrailToolsDetail::DrawTrailTab()
{
	DrawTrailDesk();
	ImGui::Dummy(ImVec2(0.f, 10.f));
	DrawTrailRawEditor();
}

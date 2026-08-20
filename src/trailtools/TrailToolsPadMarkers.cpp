#include "TrailToolsInternal.h"
#include "TrailToolsPad.h"
#include "TrailToolsShared.h"
#include "TrailToolsBinds.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsUberTool.h"

#include "Gw2Ui.h"
#include "HelperTheme.h"
#include "PadNav.h"
#include "PathingTrails.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
	bool RowBtn(const char* label, bool first)
	{
		if (!first)
			PadNav::WrapSameLine(PadNav::ButtonWidth(label));
		return ImGui::SmallButton(label);
	}

	void SyncSlotToActivePoi(TrailToolsDetail::MarkerEditorSlot& ed)
	{
		using namespace TrailToolsDetail;
		const int poi = TrailToolsUberTool::ActivePoiIndex();
		if (poi < 0)
			return;
		ed.poiIndex = poi;
		gDraft.selectedPoi = poi;
	}

	void DrawMarkerToolbar(TrailToolsDetail::MarkerEditorSlot& ed)
	{
		using namespace TrailToolsDetail;
		PadNav::PushWrap();
		if (RowBtn("Insert Marker###gw2tt_tt_mk_ins", true))
		{
			TrailToolsBinds::ActionPlaceMarker(-1);
			ed.poiIndex = gDraft.selectedPoi;
			if (gDraft.selectedPoi >= 0)
				TrailToolsUberTool::SelectPoi(gDraft.selectedPoi);
		}
		if (RowBtn("Select Nearest###gw2tt_tt_mk_near", false))
		{
			TrailToolsBinds::ActionMarkerSelectNearest();
			if (gDraft.selectedPoi >= 0)
				ed.poiIndex = gDraft.selectedPoi;
		}
		if (RowBtn("Delete Marker###gw2tt_tt_mk_del", false))
		{
			SyncSlotToActivePoi(ed);
			TrailToolsBinds::ActionDeleteMarker();
			ed.poiIndex = gDraft.selectedPoi;
		}
		if (RowBtn("Move to Feet###gw2tt_tt_mk_feet", false))
		{
			SyncSlotToActivePoi(ed);
			TrailToolsBinds::ActionMarkerMoveToFeet();
		}
		if (RowBtn("Undo###gw2tt_tt_mk_undo2", false))
			TrailToolsEditUndo::UndoPois();
		PadNav::PopWrap();
	}

	void SelectMarkerRow(TrailToolsDetail::MarkerEditorSlot& ed, int i)
	{
		using namespace TrailToolsDetail;
		if (i < 0 || i >= static_cast<int>(gDraft.pois.size()))
			return;
		ed.poiIndex = i;
		gDraft.selectedPoi = i;
		TrailToolsUberTool::SelectPoi(i);
	}

	void DrawMarkerRawList(TrailToolsDetail::MarkerEditorSlot& ed)
	{
		using namespace TrailToolsDetail;
		ImGui::TextDisabled("Raw marker data  ·  %zu in project", gDraft.pois.size());
		ImGui::Dummy(ImVec2(0.f, 4.f));
		for (int i = 0; i < static_cast<int>(gDraft.pois.size()); ++i)
		{
			DraftPoi& p = gDraft.pois[static_cast<size_t>(i)];
			ImGui::PushID(i);
			const bool sel = ed.poiIndex == i;
			char idx[16]{};
			std::snprintf(idx, sizeof(idx), "%d", i);
			if (ImGui::Selectable(idx, sel, ImGuiSelectableFlags_AllowItemOverlap,
					ImVec2(28.f, 0.f)))
				SelectMarkerRow(ed, i);
			ImGui::SameLine();
			float v[3] = { p.x, p.y, p.z };
			const float xyzW = ImGui::GetContentRegionAvail().x * 0.58f;
			ImGui::SetNextItemWidth(xyzW < 168.f ? 168.f : xyzW);
			if (ImGui::InputFloat3("###xyz", v, "%.4f"))
			{
				p.x = v[0];
				p.y = v[1];
				p.z = v[2];
				SelectMarkerRow(ed, i);
			}
			if (ImGui::IsItemClicked() || ImGui::IsItemActivated())
				SelectMarkerRow(ed, i);
			ImGui::SameLine();
			{
				const char* typ = p.type.empty() ? "(no type)" : p.type.c_str();
				if (sel)
					ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::GoldMuted);
				else
					ImGui::PushStyleColor(ImGuiCol_Text, HelperTheme::Muted);
				if (ImGui::Selectable(typ, sel, ImGuiSelectableFlags_AllowItemOverlap))
					SelectMarkerRow(ed, i);
				ImGui::PopStyleColor();
			}
			ImGui::PopID();
		}
		if (gDraft.pois.empty())
			ImGui::TextDisabled("Insert Marker or Drop here on Content.");
	}

	void DrawCopyFromLoaded()
	{
		using namespace TrailToolsDetail;
		if (!ImGui::CollapsingHeader("Copy from loaded Pathing###gw2tt_tt_copyload"))
			return;
		const auto marks = PathingTrails::CurrentMarkers();
		ImGui::TextDisabled("%zu markers on current map (enabled)", marks.size());
		if (ImGui::BeginChild("###gw2tt_tt_copylist", ImVec2(0.f, 100.f), true))
		{
			for (size_t i = 0; i < marks.size() && i < 80; ++i)
			{
				const auto& m = marks[i];
				ImGui::PushID(static_cast<int>(i));
				char lab[160]{};
				std::snprintf(lab, sizeof(lab), "%s", m.label);
				if (ImGui::Selectable(lab))
				{
					TrailToolsEditUndo::PushPois();
					DraftPoi p;
					p.mapId = m.mapId;
					p.x = m.world.x;
					p.y = m.world.y;
					p.z = m.world.z;
					p.type = m.label;
					p.guid = m.guid[0] ? m.guid : MakeGuidBase64();
					p.behavior = m.behavior;
					p.autoTrigger = m.autoTrigger;
					p.triggerRange = m.triggerRange;
					p.tipName = m.tipName;
					p.tipDescription = m.tipDescription;
					p.info = m.info;
					p.copy = m.copy;
					p.copyMessage = m.copyMessage;
					p.schedule = m.schedule;
					p.scheduleDuration = m.scheduleDuration;
					p.scriptOnce = m.scriptOnce;
					p.scriptTrigger = m.scriptTrigger;
					p.scriptFilter = m.scriptFilter;
					p.scriptTick = m.scriptTick;
					p.scriptFocus = m.scriptFocus;
					p.hide = m.hide;
					p.show = m.show;
					p.resetLength = m.resetLength;
					p.invertBehavior = m.invertBehavior;
					p.alpha = m.alpha;
					p.iconSize = m.iconSize;
					p.heightOffset = m.heightOffset;
					p.mapDisplaySize = m.mapDisplaySize;
					p.minSize = m.minSize;
					p.maxSize = m.maxSize;
					gDraft.pois.push_back(std::move(p));
					gDraft.selectedPoi = static_cast<int>(gDraft.pois.size()) - 1;
					SetStatus("Cloned marker into draft.");
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}

void TrailToolsDetail::DrawMarkersDesk(bool asPopout)
{
	PadNav::SectionTitle("Markers");
	PadNav::BeginSection("markers_main");
	ImGui::TextColored(HelperTheme::Muted, "Category");
	std::vector<std::string> leaves;
	CollectLeafPaths(gDraft.root, "", leaves, false);
	if (leaves.empty())
		leaves.push_back(gDraft.markerType[0] ? gDraft.markerType : "examplepack.circle");
	int cur = 0;
	for (size_t i = 0; i < leaves.size(); ++i)
	{
		if (leaves[i] == gDraft.markerType)
		{ cur = static_cast<int>(i); break; }
	}
	PadNav::SetFullRowWidth();
	if (ImGui::BeginCombo("###gw2tt_tt_mtype", leaves[static_cast<size_t>(cur)].c_str()))
	{
		for (size_t i = 0; i < leaves.size(); ++i)
		{
			const bool sel = static_cast<int>(i) == cur;
			if (ImGui::Selectable(leaves[i].c_str(), sel))
				std::snprintf(gDraft.markerType, sizeof(gDraft.markerType), "%s", leaves[i].c_str());
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	PadNav::InputCaption("Or type path", "gw2tt_tt_mtype_edit",
		gDraft.markerType, sizeof(gDraft.markerType));

	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	const bool pose = ReadMumblePose(mapId, x, y, z);
	static bool sThisMapOnly = true;
	ImGui::Checkbox("This map only###gw2tt_tt_mmap", &sThisMapOnly);

	if (PadNav::PrimaryButton("Drop here###gw2tt_tt_drop"))
		TrailToolsBinds::ActionPlaceMarker(-1);
	PadNav::WrapSameLine(PadNav::ButtonWidth("Delete"));
	if (ImGui::Button("Delete###gw2tt_tt_mdel"))
		TrailToolsBinds::ActionDeleteMarker();

	ImGui::Spacing();
	ImGui::TextColored(HelperTheme::Muted, "In project (%zu)", gDraft.pois.size());
	if (ImGui::BeginChild("###gw2tt_tt_mlist", ImVec2(0.f, 160.f), true))
	{
		for (int i = 0; i < static_cast<int>(gDraft.pois.size()); ++i)
		{
			const DraftPoi& p = gDraft.pois[static_cast<size_t>(i)];
			if (sThisMapOnly && pose && p.mapId != mapId)
				continue;
			ImGui::PushID(i);
			char label[256]{};
			std::snprintf(label, sizeof(label), "%d  map %u  %s###gw2tt_tt_mi",
				i, p.mapId, p.type.c_str());
			if (ImGui::Selectable(label, gDraft.selectedPoi == i))
				gDraft.selectedPoi = i;
			ImGui::SameLine();
			if (ImGui::SmallButton("Win"))
				OpenMarkerEditor(i, true);
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	if (ImGui::Button("New marker window###gw2tt_tt_open_mkn") && !gHubSkipOpenClicks)
		OpenNewMarkerEditor();
	if (!asPopout)
	{
		PadNav::WrapSameLine(PadNav::ButtonWidth("Pop out"));
		if (ImGui::Button("Pop out###gw2tt_tt_open_mkdesk") && !gHubSkipOpenClicks)
			TrailToolsPad::OpenMarkersDesk();
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add to project"));
	if (PadNav::PrimaryButton("Add to project###gw2tt_tt_ins_mkxml"))
		UpsertSelectedPoiInPack();

	ImGui::Spacing();
	ImGui::TextColored(HelperTheme::Muted, "Editors");
	for (int i = 0; i < kMaxMarkerEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sMarkers%d###gw2tt_tt_mkslot%d",
			gMarkerEditors[i].open ? "*" : "", i + 1, i);
		if (i > 0)
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

	DrawCopyFromLoaded();

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

void TrailToolsDetail::DrawMarkerRawEditor()
{
	const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	TrailToolsEditUndo::PollPoisHotkey(focused);

	if (gDraft.selectedPoi < 0 || gDraft.selectedPoi >= static_cast<int>(gDraft.pois.size()))
	{
		ImGui::TextDisabled("Select a marker, or Drop here.");
		if (ImGui::Button("Drop here###gw2tt_tt_drop_raw"))
			TrailToolsBinds::ActionPlaceMarker(-1);
		if (ImGui::Button("Undo###gw2tt_tt_mk_undo0"))
			TrailToolsEditUndo::UndoPois();
		return;
	}

	DraftPoi& p = gDraft.pois[static_cast<size_t>(gDraft.selectedPoi)];
	ImGui::Text("Marker %d", gDraft.selectedPoi);
	if (ImGui::Button("Undo###gw2tt_tt_mk_undo1"))
		TrailToolsEditUndo::UndoPois();
	DrawSelectedPoiEditor(p);
	if (ImGui::Button("Add to project###gw2tt_tt_mk_raw_ins"))
		UpsertSelectedPoiInPack();

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

void TrailToolsDetail::DrawMarkerRawEditorForSlot(int slot)
{
	const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	TrailToolsEditUndo::PollPoisHotkey(focused);

	if (slot < 0 || slot >= kMaxMarkerEditors || !gMarkerEditors[slot].open)
	{
		ImGui::TextDisabled("Editor closed.");
		return;
	}
	MarkerEditorSlot& ed = gMarkerEditors[slot];

	static const char* kTabs[] = { "Data", "Settings" };
	static const int kIcons[] = {
		static_cast<int>(Gw2Ui::Icon::Achievement),
		static_cast<int>(Gw2Ui::Icon::SettingsGear),
	};
	if (ed.tab < 0 || ed.tab > 1)
		ed.tab = 0;
	ed.tab = PadNav::DrawSideRail("###gw2tt_tt_mknav", kTabs, 2, ed.tab, 0.f, kIcons);

	const float bodyH = -HelperTheme::ResizeGripClearance();
	ImGui::BeginChild("###gw2tt_tt_mkbody", ImVec2(0.f, bodyH), false,
		ImGuiWindowFlags_AlwaysVerticalScrollbar);
	PadNav::PushWrap();

	if (ed.tab == 0)
	{
		/* Keep slot index aligned with gizmo/list lock before toolbar actions. */
		SyncSlotToActivePoi(ed);
		DrawMarkerToolbar(ed);
		ImGui::Dummy(ImVec2(0.f, 6.f));
		SyncSlotToActivePoi(ed);
		if (ImGui::BeginChild("###gw2tt_tt_mk_raw", ImVec2(0.f, 0.f), true,
				PadNav::kNestedList))
			DrawMarkerRawList(ed);
		ImGui::EndChild();
	}
	else
	{
		SyncSlotToActivePoi(ed);
		if (ed.poiIndex < 0 || ed.poiIndex >= static_cast<int>(gDraft.pois.size()))
			ImGui::TextDisabled("No marker bound — Insert Marker or pick one on Content.");
		else
		{
			DraftPoi& p = gDraft.pois[static_cast<size_t>(ed.poiIndex)];
			ImGui::TextDisabled("Markers%d — POI %d", slot + 1, ed.poiIndex);
			DrawSelectedPoiEditor(p);
		}
	}

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
	PadNav::PopWrap();
	ImGui::EndChild();
}

void TrailToolsDetail::DrawMarkersTab()
{
	DrawMarkersDesk();
	ImGui::Separator();
	DrawMarkerRawEditor();
}

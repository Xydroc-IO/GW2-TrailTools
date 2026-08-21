#include "TrailToolsInternal.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
	float gDensifySpacing = 1.25f;
	int gSmoothPasses = 1;
	int gLastClickedPoint = -1;
}

void TrailToolsDetail::DrawTrailGeomSection()
{
	PadNav::SectionTitle("Geometry");
	PadNav::BeginSection("trl_geom");
	ImGui::TextDisabled("Reverse / densify / smooth · Ctrl+click multi-select.");

	if (ImGui::Button("Reverse###gw2tt_tt_grev"))
	{
		TrailToolsEditUndo::PushTrail();
		TrailToolsTrailGeom::Reverse(gDraft.active);
		MarkDirty();
		SetStatus("Reversed trail points.");
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Densify"));
	if (ImGui::Button("Densify###gw2tt_tt_gden"))
	{
		TrailToolsEditUndo::PushTrail();
		TrailToolsTrailGeom::Densify(gDraft.active, gDensifySpacing);
		MarkDirty();
		SetStatus("Densified (max %.2fm).", gDensifySpacing);
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Smooth"));
	if (ImGui::Button("Smooth###gw2tt_tt_gsmooth"))
	{
		TrailToolsEditUndo::PushTrail();
		TrailToolsTrailGeom::Smooth(gDraft.active, gSmoothPasses);
		MarkDirty();
		SetStatus("Smoothed (%d pass).", gSmoothPasses);
	}
	PadNav::PrepLabeled("max spacing###gw2tt_tt_gsp", 90.f, true);
	ImGui::DragFloat("max spacing###gw2tt_tt_gsp", &gDensifySpacing, 0.05f, 0.25f, 10.f);
	PadNav::PrepLabeled("smooth passes###gw2tt_tt_gpass", 80.f);
	ImGui::SliderInt("smooth passes###gw2tt_tt_gpass", &gSmoothPasses, 1, 2);

	std::vector<int>* sel = nullptr;
	static std::vector<int> sActiveSel;
	sel = &sActiveSel;
	int geomSlot = -1;
	if (gTrailEditorDrawActive)
		geomSlot = gTrailEditorDrawSlot;
	else if (gTrailFocusSlot >= 0)
		geomSlot = gTrailFocusSlot;
	else
		geomSlot = gTrailRecordSlot;
	if (geomSlot >= 0 && geomSlot < kMaxTrailEditors)
		sel = &gTrailEditors[geomSlot].selectedPoints;

	if (ImGui::Button("Delete selected###gw2tt_tt_gdelsel"))
	{
		if (!sel || sel->empty())
			SetStatus("No multi-selection.");
		else
		{
			TrailToolsEditUndo::PushTrail();
			TrailToolsTrailGeom::DeleteIndices(gDraft.active, *sel, gDraft.selectedPoint);
			MarkDirty();
			SetStatus("Deleted selection.");
		}
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Clear sel"));
	if (ImGui::Button("Clear sel###gw2tt_tt_gclrsel") && sel)
	{
		TrailToolsTrailGeom::ClearSelection(*sel);
		SetStatus("Cleared multi-selection.");
	}
	if (sel && !sel->empty())
		ImGui::TextDisabled("%zu selected", sel->size());

	ImGui::Dummy(ImVec2(0.f, 4.f));
	ImGui::Text("Active: map %u | %zu points%s", gDraft.active.mapId, gDraft.active.points.size(),
		gDraft.trailDirty ? " | modified" : "");
	if (ImGui::BeginChild("###gw2tt_tt_pts", ImVec2(0.f, 120.f), true))
	{
		const ImGuiIO& io = ImGui::GetIO();
		for (int i = 0; i < static_cast<int>(gDraft.active.points.size()); ++i)
		{
			const auto& p = gDraft.active.points[static_cast<size_t>(i)];
			const bool brk = TrailToolsTrailGeom::IsBreak(p);
			const bool multi = sel && TrailToolsTrailGeom::IsSelected(*sel, i);
			char lab[112]{};
			if (brk)
				std::snprintf(lab, sizeof(lab), "%s%4d  [section break]", multi ? "*" : " ", i);
			else
				std::snprintf(lab, sizeof(lab), "%s%4d  %.3f  %.3f  %.3f",
					multi ? "*" : " ", i, p.x, p.y, p.z);
			const bool primary = gDraft.selectedPoint == i;
			if (ImGui::Selectable(lab, primary || multi))
			{
				if (io.KeyCtrl && sel)
					TrailToolsTrailGeom::ToggleSelect(*sel, i);
				else if (io.KeyShift && sel && gLastClickedPoint >= 0)
					TrailToolsTrailGeom::SelectRange(*sel, gLastClickedPoint, i);
				else if (sel)
				{
					TrailToolsTrailGeom::ClearSelection(*sel);
					sel->push_back(i);
				}
				gDraft.selectedPoint = i;
				gLastClickedPoint = i;
			}
		}
	}
	ImGui::EndChild();

	if (gDraft.selectedPoint >= 0 &&
		gDraft.selectedPoint < static_cast<int>(gDraft.active.points.size()))
	{
		ImGui::Dummy(ImVec2(0.f, 4.f));
		auto& pt = gDraft.active.points[static_cast<size_t>(gDraft.selectedPoint)];
		if (ImGui::DragFloat3("Edit point XYZ###gw2tt_tt_ptedit", &pt.x, 0.05f))
			MarkDirty();
		if (ImGui::IsItemActivated())
			TrailToolsEditUndo::PushTrail();
		if (ImGui::SmallButton("Delete point###gw2tt_tt_ptdel"))
		{
			TrailToolsEditUndo::PushTrail();
			gDraft.active.points.erase(gDraft.active.points.begin() + gDraft.selectedPoint);
			gDraft.selectedPoint = -1;
			MarkDirty();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Insert after###gw2tt_tt_ptins"))
		{
			TrailToolsEditUndo::PushTrail();
			gDraft.active.points.insert(
				gDraft.active.points.begin() + gDraft.selectedPoint + 1, pt);
			++gDraft.selectedPoint;
			MarkDirty();
		}
	}
	PadNav::EndSection();
}

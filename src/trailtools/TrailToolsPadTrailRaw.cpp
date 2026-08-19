#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrl.h"
#include "TrailToolsBinds.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsXml.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <windows.h>

namespace
{
	bool RowBtn(const char* label, bool first)
	{
		if (!first)
			PadNav::WrapSameLine(PadNav::ButtonWidth(label));
		return ImGui::SmallButton(label);
	}

	void DrawFileTools()
	{
		using namespace TrailToolsDetail;
		PadNav::PushWrap();
		if (RowBtn("New###gw2tt_tt_newtrl", true))
		{
			TrailToolsEditUndo::PushTrail();
			SyncActiveFileRelFromStem();
			gDraft.active = {};
			gDraft.active.type = gDraft.trailType[0] ? gDraft.trailType
				: (RootCategoryName() + ".example");
			SyncActiveFileRelFromStem();
			gDraft.selectedTrail = -1;
			gDraft.selectedPoint = -1;
			gDraft.trailDirty = false;
			RecordingWorldShown() = false;
			SetStatus("New empty trail.");
		}
		if (RowBtn("Load###gw2tt_tt_load", false))
		{
			std::wstring path;
			if (!DialogPickTrl(false, path))
				SetStatus("Load cancelled.");
			else
			{
				uint32_t mid = 0;
				std::vector<PathingTrails::WorldPoint> pts;
				if (!TrailToolsTrl::Read(path, mid, pts))
					SetStatus("Load failed.");
				else
				{
					TrailToolsEditUndo::PushTrail();
					RememberDirFromPath(path);
					gDraft.active.mapId = mid;
					gDraft.active.points = std::move(pts);
					gDraft.selectedPoint = -1;
					std::string under;
					if (TryAbsUnderPack(path, under))
					{
						gDraft.active.fileRel = under;
						ApplyStemFromFileRel();
					}
					else
					{
						const size_t slash = path.find_last_of(L"\\/");
						std::wstring name = slash == std::wstring::npos ? path
							: path.substr(slash + 1);
						std::string stem = WideToUtf8(name);
						if (stem.size() > 4 && stem.compare(stem.size() - 4, 4, ".trl") == 0)
							stem.resize(stem.size() - 4);
						std::snprintf(gDraft.trailFileStem, sizeof(gDraft.trailFileStem), "%s",
							stem.c_str());
						SyncActiveFileRelFromStem();
					}
					SyncActiveType();
					gDraft.trailDirty = false;
					gDraft.selectedTrail = -1;
					SetStatus("Loaded map %u, %zu points.", mid, gDraft.active.points.size());
				}
			}
		}
		if (RowBtn("Save###gw2tt_tt_save", false))
		{
			SyncActiveFileRelFromStem();
			SaveActiveToPath(ActiveTrlPath());
		}
		if (RowBtn("Save As###gw2tt_tt_saveas", false))
		{
			std::wstring path;
			if (!DialogPickTrl(true, path))
				SetStatus("Save As cancelled.");
			else
				SaveActiveToPath(path);
		}
		if (RowBtn("Copy XML###gw2tt_tt_copytrxmln", false))
		{
			const std::string line = TrailToolsXml::EmitTrailElementBasic(gDraft.active);
			if (line.empty())
				SetStatus("Set type and trailData (Save .trl) first.");
			else
			{
				CopyClipboard(line.c_str());
				SetStatus("Copied %s", line.c_str());
			}
		}
		PadNav::PopWrap();
	}

	void DrawTrailTools()
	{
		PadNav::PushWrap();
		if (RowBtn("New Segment###gw2tt_tt_sec", true))
			TrailToolsBinds::ActionTrailSection();
		if (RowBtn("Insert Vector###gw2tt_tt_insvec", false))
			TrailToolsBinds::ActionTrailInsertVector();
		if (RowBtn("Select Nearest###gw2tt_tt_near", false))
			TrailToolsBinds::ActionTrailSelectNearest();
		if (RowBtn("Move to Feet###gw2tt_tt_movefoot", false))
			TrailToolsBinds::ActionTrailMoveToFeet();
		if (RowBtn("Delete Nearest###gw2tt_tt_delnear", false))
			TrailToolsBinds::ActionTrailDeleteNearest();
		if (TrailToolsEditUndo::CanUndoTrail())
		{
			if (RowBtn("Undo###gw2tt_tt_tr_undo", false))
				TrailToolsEditUndo::UndoTrail();
		}
		PadNav::PopWrap();
	}

	void ClampSampleSpacing(float& v)
	{
		if (!std::isfinite(v) || v < 0.05f || v > 5.f)
			v = 0.3f;
	}

	void DrawRecordingRail()
	{
		using namespace TrailToolsDetail;
		auto& kb = TrailToolsBinds::Get();
		ClampSampleSpacing(kb.trailSampleSpacing);
		ImGui::TextUnformatted("Recording");
		ImGui::Dummy(ImVec2(0.f, 6.f));
		if (kb.trailRecording)
		{
			if (ImGui::Button("Stop###gw2tt_tt_recstop", ImVec2(-1.f, 0.f)))
				TrailToolsBinds::ActionTrailStop();
		}
		else if (ImGui::Button("Start###gw2tt_tt_recstart", ImVec2(-1.f, 0.f)))
			TrailToolsBinds::ActionTrailStart();
		const char* pauseLab = (kb.trailRecording && kb.trailPaused)
			? "Resume###gw2tt_tt_recpause"
			: "Pause###gw2tt_tt_recpause";
		if (ImGui::Button(pauseLab, ImVec2(-1.f, 0.f)))
			TrailToolsBinds::ActionTrailPause();
		ImGui::Dummy(ImVec2(0.f, 10.f));
		ImGui::TextUnformatted("Spacing");
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::InputFloat("###gw2tt_tt_vspace", &kb.trailSampleSpacing, 0.f, 0.f, "%.2f"))
			ClampSampleSpacing(kb.trailSampleSpacing);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Seconds between samples while moving (0.3 = 1/3 s).\n"
				"Standing still does not add duplicate vectors.");
		if (kb.trailRecording)
			ImGui::TextColored(kb.trailPaused ? HelperTheme::Muted : HelperTheme::Ok,
				kb.trailPaused ? "Paused" : "Recording");
		else
			ImGui::TextDisabled("Idle");
	}

	void DrawRawPointList(float height)
	{
		using namespace TrailToolsDetail;
		ImGui::TextDisabled("Raw trail data  ·  map %u  ·  %zu pts%s", gDraft.active.mapId,
			gDraft.active.points.size(), gDraft.trailDirty ? " *" : "");
		ImGui::BeginChild("###gw2tt_tt_rawpts", ImVec2(0.f, height), true);
		ImGui::Dummy(ImVec2(0.f, 4.f));
		for (int i = 0; i < static_cast<int>(gDraft.active.points.size()); ++i)
		{
			auto& p = gDraft.active.points[static_cast<size_t>(i)];
			ImGui::PushID(i);
			const bool sel = gDraft.selectedPoint == i;
			if (IsSectionBreak(p))
			{
				if (ImGui::Selectable("[segment]", sel))
					gDraft.selectedPoint = i;
			}
			else
			{
				char idx[16]{};
				std::snprintf(idx, sizeof(idx), "%d", i);
				if (ImGui::Selectable(idx, sel, 0, ImVec2(28.f, 0.f)))
					gDraft.selectedPoint = i;
				ImGui::SameLine();
				float v[3] = { p.x, p.y, p.z };
				const float xyzW = ImGui::GetContentRegionAvail().x * 0.58f;
				ImGui::SetNextItemWidth(xyzW < 168.f ? 168.f : xyzW);
				if (ImGui::InputFloat3("###xyz", v, "%.4f"))
				{
					p.x = v[0];
					p.y = v[1];
					p.z = v[2];
					gDraft.selectedPoint = i;
					gDraft.trailDirty = true;
				}
			}
			ImGui::PopID();
		}
		if (gDraft.active.points.empty())
			ImGui::TextDisabled("Insert Vector (feet) or Start recording.");
		ImGui::EndChild();
	}
}

void TrailToolsDetail::DrawTrailRawEditor()
{
	EnsureWorkspace();
	SyncActiveType();
	if (gDraft.active.fileRel.empty())
		SyncActiveFileRelFromStem();

	const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	TrailToolsEditUndo::PollTrailHotkey(focused);

	ImGui::Dummy(ImVec2(0.f, 10.f));
	DrawFileTools();
	ImGui::Dummy(ImVec2(0.f, 10.f));
	DrawTrailTools();
	ImGui::Dummy(ImVec2(0.f, 10.f));

	const float pad = ImGui::GetStyle().WindowPadding.x * 2.f + 4.f;
	const float leftW = PadNav::ButtonWidth("Recording") + pad;
	const float rowH = ImGui::GetContentRegionAvail().y;
	ImGui::BeginChild("###gw2tt_tt_recrail", ImVec2(leftW, rowH), true,
		ImGuiWindowFlags_NoScrollbar);
	DrawRecordingRail();
	ImGui::EndChild();
	ImGui::SameLine(0.f, 12.f);
	ImGui::BeginChild("###gw2tt_tt_rawbody", ImVec2(0.f, rowH), false);
	{
		const float moreH = ImGui::GetFrameHeightWithSpacing();
		float listH = ImGui::GetContentRegionAvail().y - moreH - 4.f;
		if (listH < 64.f)
			listH = 64.f;
		DrawRawPointList(listH);
	}
	if (ImGui::CollapsingHeader("Attrs / geometry###gw2tt_tt_more"))
	{
		DrawTrailAttrsSection();
		DrawTrailGeomSection();
	}
	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
	ImGui::EndChild();
}

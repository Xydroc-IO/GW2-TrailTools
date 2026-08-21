#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsEditUndo.h"

#include "HelperTheme.h"
#include "PadNav.h"
#include "TrailToolsXml.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{
	void CopyRow(const char* id, const char* clip, const char* label)
	{
		char bid[64]{};
		std::snprintf(bid, sizeof(bid), "COPY###%s", id);
		if (PadNav::ActionSmallButton(bid))
		{
			TrailToolsDetail::InsertAtXmlCaret(clip);
			TrailToolsDetail::SetStatus("Inserted into OverlayData at caret.");
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(label);
	}

	void TypeCombo(const char* comboId, const char* editId, char* buf, size_t n, bool trails)
	{
		using namespace TrailToolsDetail;
		std::vector<std::string> leaves;
		CollectLeafPaths(gDraft.root, "", leaves, trails);
		if (leaves.empty() && buf[0])
			leaves.push_back(buf);
		if (leaves.empty())
			leaves.push_back(trails ? "examplepack.example" : "examplepack.circle");
		int cur = 0;
		for (size_t i = 0; i < leaves.size(); ++i)
		{
			if (leaves[i] == buf)
			{
				cur = static_cast<int>(i);
				break;
			}
		}
		PadNav::SetFullRowWidth();
		if (ImGui::BeginCombo(comboId, leaves[static_cast<size_t>(cur)].c_str()))
		{
			for (size_t i = 0; i < leaves.size(); ++i)
			{
				const bool sel = static_cast<int>(i) == cur;
				if (ImGui::Selectable(leaves[i].c_str(), sel))
				{
					std::snprintf(buf, n, "%s", leaves[i].c_str());
					if (trails)
					{
						gDraft.active.type = buf;
						MarkDirty();
					}
				}
				if (sel)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		PadNav::InputCaption("Or type path", editId, buf, n);
	}
}

void TrailToolsDetail::DrawContentTab()
{
	EnsureWorkspace();
	SyncActiveType();
	if (gDraft.active.fileRel.empty())
		SyncActiveFileRelFromStem();

	DrawLiveTab();

	PadNav::SectionTitle("Project");
	PadNav::BeginSection("content_xml");
	DrawXmlProjectStrip();
	PadNav::EndSection();

	PadNav::SectionTitle("Pose");
	PadNav::BeginSection("content_pose");
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	const bool pose = ReadMumblePose(mapId, x, y, z);
	if (!pose)
		ImGui::TextColored(HelperTheme::Warn, "No Mumble pose — enter the game world.");
	else
	{
		char mapLine[32]{};
		char vecLine[128]{};
		char poiLine[384]{};
		char mapLab[48]{};
		char xyzLab[96]{};
		std::snprintf(mapLine, sizeof(mapLine), "%u", mapId);
		std::snprintf(vecLine, sizeof(vecLine), "%.4f %.4f %.4f", x, y, z);
		std::snprintf(mapLab, sizeof(mapLab), "Map ID:  %u", mapId);
		std::snprintf(xyzLab, sizeof(xyzLab), "XYZ:  %.4f  %.4f  %.4f", x, y, z);
		const std::string guid = MakeGuidBase64();
		std::snprintf(poiLine, sizeof(poiLine),
			"<POI MapID=\"%u\" xpos=\"%.6g\" ypos=\"%.6g\" zpos=\"%.6g\" type=\"%s\" GUID=\"%s\"/>",
			mapId, x, y, z, gDraft.markerType, guid.c_str());
		if (ImGui::BeginTable("###pose_copy", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableNextColumn();
			CopyRow("cmap", mapLine, mapLab);
			ImGui::TableNextColumn();
			CopyRow("cxyz", vecLine, xyzLab);
			ImGui::TableNextColumn();
			if (PadNav::ActionSmallButton("COPY###cpoi"))
			{
				TrailToolsEditUndo::PushPois();
				DraftPoi p;
				p.mapId = mapId;
				p.x = x;
				p.y = y;
				p.z = z;
				p.type = gDraft.markerType[0] ? gDraft.markerType : "examplepack.circle";
				p.guid = guid;
				gDraft.pois.push_back(std::move(p));
				gDraft.selectedPoi = static_cast<int>(gDraft.pois.size()) - 1;
				gDraft.xmlDirty = true;
				InsertAtXmlCaret(poiLine);
				SetStatus("Created POI and inserted tag at caret.");
			}
			ImGui::SameLine();
			ImGui::TextUnformatted("Create POI (MapID, XYZ, GUID)");
			ImGui::TableNextColumn();
			if (PadNav::ActionSmallButton("COPY###cguid"))
			{
				InsertAtXmlCaret(guid.c_str());
				SetStatus("Inserted into OverlayData at caret.");
			}
			ImGui::SameLine();
			ImGui::TextUnformatted("Create Random GUID (Base64)");
			ImGui::EndTable();
		}
	}
	PadNav::EndSection();

	PadNav::SectionTitle("OverlayData");
	PadNav::BeginSection("content_xmlbuf");
	if (PadNav::ActionButton("XML editor###gw2tt_tt_xmledit_c") && !gHubSkipOpenClicks)
		OpenXmlEditor();
	if (ImGui::BeginChild("###gw2tt_tt_xmlpane", ImVec2(0.f, 200.f), true, PadNav::kNestedList))
		DrawXmlEditorPane(ImGui::GetContentRegionAvail().y);
	ImGui::EndChild();
	PadNav::EndSection();

	PadNav::SectionTitle("Trail default");
	PadNav::BeginSection("content_trl_cat");
	TypeCombo("###gw2tt_tt_trltype", "gw2tt_tt_trltype_edit",
		gDraft.trailType, sizeof(gDraft.trailType), true);
	ImGui::TextColored(HelperTheme::Muted,
		"Write into OverlayData adds the active TrailsN / hub trail to the draft (then Save or Fill regenerates XML).");
	if (PadNav::ActionButton("New trail window###gw2tt_tt_open_trn") && !gHubSkipOpenClicks)
		OpenNewTrailEditor();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Write into OverlayData"));
	if (PadNav::PrimaryButton("Write into OverlayData###gw2tt_tt_ins_trxml"))
		UpsertActiveTrailInPack();
	ImGui::Dummy(ImVec2(0.f, 2.f));
	for (int i = 0; i < kMaxTrailEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sTrails%d###gw2tt_tt_trslot%d",
			gTrailEditors[i].open ? "*" : "", i + 1, i);
		if (i > 0)
			PadNav::WrapSameLine(PadNav::ButtonWidth(lab));
		if (PadNav::ActionSmallButton(lab) && !gHubSkipOpenClicks)
			OpenTrailEditorSlot(i);
	}
	PadNav::EndSection();

	PadNav::SectionTitle("Marker default");
	PadNav::BeginSection("content_mk_cat");
	TypeCombo("###gw2tt_tt_mtype", "gw2tt_tt_mtype_edit",
		gDraft.markerType, sizeof(gDraft.markerType), false);
	ImGui::TextColored(HelperTheme::Muted,
		"Write into OverlayData marks the selected POI dirty in the draft (POIs already live in OverlayData).");
	if (PadNav::ActionButton("New marker window###gw2tt_tt_open_mkn") && !gHubSkipOpenClicks)
		OpenNewMarkerEditor();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Write into OverlayData"));
	if (PadNav::PrimaryButton("Write into OverlayData###gw2tt_tt_ins_mkxml"))
		UpsertSelectedPoiInPack();
	ImGui::Dummy(ImVec2(0.f, 2.f));
	for (int i = 0; i < kMaxMarkerEditors; ++i)
	{
		char lab[48]{};
		std::snprintf(lab, sizeof(lab), "%sMarkers%d###gw2tt_tt_mkslot%d",
			gMarkerEditors[i].open ? "*" : "", i + 1, i);
		if (i > 0)
			PadNav::WrapSameLine(PadNav::ButtonWidth(lab));
		if (PadNav::ActionSmallButton(lab) && !gHubSkipOpenClicks)
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

	if (ImGui::CollapsingHeader("Pack###gw2tt_tt_packmore"))
		DrawPackTab();
}

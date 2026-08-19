#include "PackEditInternal.h"

#include "AddonPaths.h"
#include "HelperTheme.h"
#include "PadNav.h"
#include "TrailToolsShared.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <string>

#include <windows.h>
#include <commdlg.h>

namespace
{
	bool PickPath(bool save, bool folderHint, std::wstring& out)
	{
		(void)folderHint;
		wchar_t file[MAX_PATH]{};
		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = L"Marker pack (*.taco;*.zip)\0*.taco;*.zip\0All\0*.*\0";
		ofn.lpstrFile = file;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
		ofn.lpstrDefExt = L"taco";
		const BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
		if (!ok)
			return false;
		out = file;
		return true;
	}

	bool PickFolder(std::wstring& out)
	{
		/* Simple: user picks any file inside the pack folder; we take the directory. */
		std::wstring file;
		if (!PickPath(false, true, file))
			return false;
		const size_t sl = file.find_last_of(L"\\/");
		if (sl == std::wstring::npos)
			return false;
		out = file.substr(0, sl);
		return true;
	}
}

void PackEdit::DrawTab()
{
	PadNav::SectionTitle("Pack file");
	PadNav::BeginSection("pe_file");
	ImGui::TextDisabled("Edit an installed .taco in place (our editor — zip in / zip out).");
	if (ImGui::Button("Open pack...###pe_open"))
	{
		std::wstring path;
		std::string err;
		if (PickPath(false, false, path) && !OpenZip(path, err))
			std::snprintf(gDoc.status, sizeof(gDoc.status), "%s", err.c_str());
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Open folder..."));
	if (ImGui::Button("Open folder...###pe_folder"))
	{
		std::wstring dir;
		std::string err;
		if (PickFolder(dir) && !OpenFolder(dir, err))
			std::snprintf(gDoc.status, sizeof(gDoc.status), "%s", err.c_str());
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("New pack"));
	if (ImGui::Button("New pack###pe_new"))
		NewEmpty();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Close pack"));
	if (ImGui::Button("Close pack###pe_close"))
	{
		const bool hasPack = !gDoc.roots.empty() || !gDoc.items.empty() ||
			!gDoc.entries.empty() || !gDoc.path.empty();
		if (hasPack)
			ImGui::OpenPopup("pe_close_ask");
		else
			std::snprintf(gDoc.status, sizeof(gDoc.status), "Editor is already empty.");
	}
	if (ImGui::BeginPopup("pe_close_ask"))
	{
		ImGui::TextUnformatted("Remove this pack from the editor?");
		ImGui::TextDisabled("Unsaved work is discarded. Files on disk are not deleted.");
		if (ImGui::Button("Close###pe_close_ok"))
		{
			ClosePack();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel###pe_close_no"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Save"));
	if (PadNav::PrimaryButton("Save###pe_save"))
	{
		std::wstring path = gDoc.path;
		std::string err;
		if (path.empty() || !gDoc.fromZip)
		{
			if (!PickPath(true, false, path))
				std::snprintf(gDoc.status, sizeof(gDoc.status), "Save cancelled.");
			else if (!SaveZip(path, err))
				std::snprintf(gDoc.status, sizeof(gDoc.status), "%s", err.c_str());
		}
		else if (!SaveZip(path, err))
			std::snprintf(gDoc.status, sizeof(gDoc.status), "%s", err.c_str());
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Save As..."));
	if (ImGui::Button("Save As...###pe_saveas"))
	{
		std::wstring path;
		std::string err;
		if (PickPath(true, false, path) && !SaveZip(path, err))
			std::snprintf(gDoc.status, sizeof(gDoc.status), "%s", err.c_str());
	}
	if (gDoc.dirty)
		ImGui::TextColored(HelperTheme::Warn, "Unsaved edits");
	DrawWorldToggles("_tab");
	if (ImGui::Button("Undo###pe_undo"))
		Undo();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Redo"));
	if (ImGui::Button("Redo###pe_redo"))
		Redo();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add POI"));
	if (ImGui::Button("Add POI###pe_addpoi"))
	{
		AddPoiAtFeet();
		uint32_t mapId = 0;
		float x = 0.f, y = 0.f, z = 0.f;
		if (TrailToolsDetail::ReadMumblePose(mapId, x, y, z))
		{
			PePathable* p = Selected();
			if (p)
			{
				p->mapId = mapId;
				p->x = x;
				p->y = y;
				p->z = z;
			}
		}
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add Trail"));
	if (ImGui::Button("Add Trail###pe_addtr"))
		AddTrailEmpty();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Add category"));
	if (ImGui::Button("Add category###pe_addcat"))
		AddCategory();
	PadNav::EndSection();

	if (!gDoc.popTree)
		DrawTree();
	if (!gDoc.popDet)
		DrawDetails();
	if (!gDoc.popRes)
		DrawResources();
	if (!gDoc.popMap)
		DrawCanvas();

	if (gDoc.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDoc.status);
	const int lint = LintIssues();
	if (lint > 0)
		ImGui::TextColored(HelperTheme::Warn, "%d lint issues (empty type, MapID 0, or missing trailData).", lint);
	ImGui::TextDisabled("%zu markers/trails · %zu selected · Save keeps original XML files.",
		gDoc.items.size(), gDoc.selItems.size());
}

void PackEdit::HideWorldOverlay()
{
	gDoc.worldDraw = false;
}

void PackEdit::DrawWorldToggles(const char* idSuf)
{
	if (!idSuf)
		idSuf = "";
	char id[48];
	std::snprintf(id, sizeof(id), "Draw pack in world###pe_wdraw%s", idSuf);
	ImGui::Checkbox(id, &gDoc.worldDraw);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "World gizmo###pe_giz%s", idSuf);
	ImGui::Checkbox(id, &gDoc.gizmoOn);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "Rotate drag###pe_rotmode%s", idSuf);
	ImGui::Checkbox(id, &gDoc.rotateMode);
	std::snprintf(id, sizeof(id), "This map only###pe_maponly%s", idSuf);
	ImGui::Checkbox(id, &gDoc.thisMapOnly);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "Pop tree###pe_popt%s", idSuf);
	ImGui::Checkbox(id, &gDoc.popTree);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "Pop details###pe_popd%s", idSuf);
	ImGui::Checkbox(id, &gDoc.popDet);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "Pop resources###pe_popr%s", idSuf);
	ImGui::Checkbox(id, &gDoc.popRes);
	ImGui::SameLine();
	std::snprintf(id, sizeof(id), "Pop 2D map###pe_popm%s", idSuf);
	ImGui::Checkbox(id, &gDoc.popMap);
	std::snprintf(id, sizeof(id), "Ground snap###pe_gsnap%s", idSuf);
	ImGui::Checkbox(id, &TrailToolsDetail::gGroundSnap);
	ImGui::TextDisabled("Ground snap: walk + pack points, plane fit (not the game mesh).");
}

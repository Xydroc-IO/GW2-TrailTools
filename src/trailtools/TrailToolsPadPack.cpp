#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsBuild.h"

#include "AddonPaths.h"
#include "Globals.h"
#include "HelperTheme.h"
#include "PadNav.h"
#include "PathingTrails.h"
#include "Settings.h"

#include "imgui/imgui.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

void TrailToolsDetail::DrawLooksDefaultsUi()
{
	ImGui::TextDisabled("Presets and paths for the active trail / marker category (TacO defaults).");

	int trailN = 0, markN = 0;
	const char* const* trailNames = TrailLookPresetNames(&trailN);
	const char* const* markNames = MarkerLookPresetNames(&markN);
	static int sTrailPreset = 0;
	static int sMarkPreset = 0;

	ImGui::SetNextItemWidth(200.f);
	if (ImGui::Combo("Trail look###gw2tt_tt_tlook", &sTrailPreset, trailNames, trailN))
		ApplyTrailLookPreset(sTrailPreset);
	PadNav::WrapSameLine(PadNav::ButtonWidth("Apply trail"));
	if (ImGui::Button("Apply trail###gw2tt_tt_tlook_go"))
		ApplyTrailLookPreset(sTrailPreset);

	ImGui::SetNextItemWidth(200.f);
	if (ImGui::Combo("Marker look###gw2tt_tt_mlook", &sMarkPreset, markNames, markN))
		ApplyMarkerLookPreset(sMarkPreset);
	PadNav::WrapSameLine(PadNav::ButtonWidth("Apply marker"));
	if (ImGui::Button("Apply marker###gw2tt_tt_mlook_go"))
		ApplyMarkerLookPreset(sMarkPreset);

	CategoryNode* trailLeaf = FindCategoryByPath(gDraft.root,
		gDraft.trailType[0] ? std::string(gDraft.trailType) : RootCategoryName() + ".example");
	CategoryNode* markLeaf = FindCategoryByPath(gDraft.root,
		gDraft.markerType[0] ? std::string(gDraft.markerType) : RootCategoryName() + ".circle");

	if (trailLeaf)
	{
		ImGui::Separator();
		ImGui::TextDisabled("Trail category: %s", gDraft.trailType);
		char tex[256]{};
		std::snprintf(tex, sizeof(tex), "%s", trailLeaf->texture.c_str());
		PadNav::PushWidthForLabel("Default trail texture###gw2tt_tt_ttex");
		if (ImGui::InputText("Default trail texture###gw2tt_tt_ttex", tex, sizeof(tex)))
			trailLeaf->texture = tex;
		PadNav::PopWidthForLabel();
		PadNav::PrepLabeled("trailScale###gw2tt_tt_tscale", 120.f, true);
		ImGui::DragFloat("trailScale###gw2tt_tt_tscale", &trailLeaf->trailScale, 0.05f, 0.25f, 4.f);
		PadNav::PrepLabeled("fadeNear###gw2tt_tt_tfn", 100.f);
		ImGui::DragFloat("fadeNear###gw2tt_tt_tfn", &trailLeaf->fadeNear, 10.f, -1.f, 20000.f);
		PadNav::PrepLabeled("fadeFar###gw2tt_tt_tff", 100.f);
		ImGui::DragFloat("fadeFar###gw2tt_tt_tff", &trailLeaf->fadeFar, 10.f, -1.f, 20000.f);
		float rgba[4] = {
			((trailLeaf->color >> 16) & 0xFFu) / 255.f,
			((trailLeaf->color >> 8) & 0xFFu) / 255.f,
			(trailLeaf->color & 0xFFu) / 255.f,
			((trailLeaf->color >> 24) & 0xFFu) / 255.f,
		};
		if (trailLeaf->color == 0)
		{
			rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1.f;
		}
		if (ImGui::ColorEdit4("Trail tint###gw2tt_tt_tcol", rgba,
			ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex))
		{
			const auto ch = [](float f) -> uint32_t {
				return static_cast<uint32_t>(std::clamp(f, 0.f, 1.f) * 255.f + 0.5f);
			};
			trailLeaf->color = (ch(rgba[3]) << 24) | (ch(rgba[0]) << 16) |
				(ch(rgba[1]) << 8) | ch(rgba[2]);
		}
	}

	if (markLeaf)
	{
		ImGui::Separator();
		ImGui::TextDisabled("Marker category: %s", gDraft.markerType);
		char icon[256]{};
		std::snprintf(icon, sizeof(icon), "%s", markLeaf->iconFile.c_str());
		PadNav::PushWidthForLabel("Default marker icon###gw2tt_tt_micon");
		if (ImGui::InputText("Default marker icon###gw2tt_tt_micon", icon, sizeof(icon)))
			markLeaf->iconFile = icon;
		PadNav::PopWidthForLabel();
		PadNav::PrepLabeled("iconSize###gw2tt_tt_misz", 120.f, true);
		ImGui::DragFloat("iconSize###gw2tt_tt_misz", &markLeaf->iconSize, 0.05f, 0.25f, 4.f);
		float rgba[4] = {
			((markLeaf->color >> 16) & 0xFFu) / 255.f,
			((markLeaf->color >> 8) & 0xFFu) / 255.f,
			(markLeaf->color & 0xFFu) / 255.f,
			((markLeaf->color >> 24) & 0xFFu) / 255.f,
		};
		if (markLeaf->color == 0)
		{
			rgba[0] = 1.f; rgba[1] = 0.8f; rgba[2] = 0.16f; rgba[3] = 1.f;
		}
		if (ImGui::ColorEdit4("Marker tint###gw2tt_tt_mcol", rgba,
			ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHex))
		{
			const auto ch = [](float f) -> uint32_t {
				return static_cast<uint32_t>(std::clamp(f, 0.f, 1.f) * 255.f + 0.5f);
			};
			markLeaf->color = (ch(rgba[3]) << 24) | (ch(rgba[0]) << 16) |
				(ch(rgba[1]) << 8) | ch(rgba[2]);
		}
	}
}

void TrailToolsDetail::DrawPackTab()
{
	PadNav::SectionTitle("Pack");
	PadNav::BeginSection("pack_id");
	ImGui::TextDisabled("Identity + build. OverlayData New/Load/Save stays under Project above.");
	PadNav::PushWrap();
	ImGui::TextDisabled("pathing/authoring/%s/", gDraft.packName);
	PadNav::PopWrap();

	static char sPrevPack[64] = {};
	if (!sPrevPack[0])
		std::snprintf(sPrevPack, sizeof(sPrevPack), "%s", gDraft.packName);
	PadNav::InputCaption("Pack name", "gw2tt_tt_pname", gDraft.packName, sizeof(gDraft.packName));
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		const std::string oldPack = sPrevPack[0] ? sPrevPack : "ExamplePack";
		std::string oldRoot = oldPack;
		for (char& ch : oldRoot)
			ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
		SanitizePackName(gDraft.packName, sizeof(gDraft.packName));
		RemapDraftAfterPackRename(oldPack, oldRoot);
		gDraft.root.name = RootCategoryName();
		if (gDraft.active.fileRel.empty())
		{
			gDraft.active.fileRel = std::string("Data/") +
				(gDraft.trailFileStem[0] ? gDraft.trailFileStem : "Trail") + ".trl";
		}
		std::snprintf(sPrevPack, sizeof(sPrevPack), "%s", gDraft.packName);
		SetStatus("Pack renamed - category paths remapped.");
	}
	else if (!ImGui::IsItemActive())
		std::snprintf(sPrevPack, sizeof(sPrevPack), "%s", gDraft.packName);

	PadNav::InputCaption("Display name", "gw2tt_tt_pdname",
		gDraft.displayName, sizeof(gDraft.displayName));

	if (ImGui::Button("Open folder###gw2tt_tt_folder"))
	{
		if (OpenAuthoringFolder())
			SetStatus("Opened authoring folder.");
		else
			SetStatus("Could not open folder.");
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Save draft"));
	if (ImGui::Button("Save draft###gw2tt_tt_savedraft"))
		SaveDraftSession();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Load draft"));
	if (ImGui::Button("Load draft###gw2tt_tt_loaddraft"))
		LoadDraftSession();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Reseed"));
	if (ImGui::Button("Reseed###gw2tt_tt_reseed"))
	{
		SeedDefaultCategories();
		SetStatus("Reseeded TacO-style categories (example / circle / heart / square / triangle).");
	}

	PadNav::PushWrap();
	ImGui::TextDisabled("Active trail · %s%s · map %u · %zu pts",
		gDraft.active.fileRel.empty() ? "(none)" : gDraft.active.fileRel.c_str(),
		gDraft.trailDirty ? " *" : "",
		gDraft.active.mapId,
		gDraft.active.points.size());
	PadNav::PopWrap();
	PadNav::EndSection();

	PadNav::SectionTitle("Import / build");
	PadNav::BeginSection("pack_build");
	static char sImportName[96] = "Hero.Blish.Pack.taco";
	PadNav::InputCaption("Installed .taco filename", "gw2tt_tt_impname",
		sImportName, sizeof(sImportName));
	if (ImGui::Button("Import###gw2tt_tt_import"))
	{
		std::wstring path = AddonPaths::EnsureUnder(AddonPaths::DataDir(), L"pathing");
		path += L"\\";
		for (const char* c = sImportName; *c; ++c)
			path.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*c)));
		std::string err;
		if (!ImportTacoToDraft(path, err))
			SetStatus("%s", err.c_str());
		else
			SetStatus("Imported %s into draft.", sImportName);
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Reload Pathing"));
	if (ImGui::Button("Reload Pathing###gw2tt_tt_reload"))
	{
		PathingTrails::ReloadPacks();
		SetStatus("Pathing packs reloaded.");
	}
	PadNav::WrapSameLine(PadNav::ButtonWidth("Build .taco"));
	if (PadNav::PrimaryButton("Build .taco###gw2tt_tt_build"))
	{
		std::string err;
		if (!TrailToolsBuild::BuildTaco(err))
			SetStatus("%s", err.c_str());
		else
		{
			const std::string root = RootCategoryName();
			PathingTrails::SetCategoryEnabled(root, true);
			PathingTrails::SerializeEnabledPaths(G::PathingEnabled, sizeof(G::PathingEnabled));
			Settings::SaveNow();
			PathingTrails::ReloadPacks();
			SetStatus("Built %s.taco, enabled \"%s\", reloaded Pathing.",
				gDraft.packName, root.c_str());
		}
	}
	PadNav::EndSection();

	/* Parked: list-only until Lua authoring workflow is defined. */
	if (ImGui::CollapsingHeader("Lua (pack scripts)###gw2tt_tt_lua"))
		DrawLuaFilesUi();

	ImGui::TextDisabled("%zu trails · %zu markers", gDraft.trails.size() +
		(gDraft.active.points.size() >= 2 ? 1u : 0u), gDraft.pois.size());

	if (gDraft.status[0])
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
}

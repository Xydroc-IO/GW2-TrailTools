#include "entryInternal.h"

#include "imgui/imgui.h"

#include "CrashTrail.h"
#include "Globals.h"
#include "PathingTrails.h"
#include "Settings.h"
#include "TrailToolsPad.h"
#include "TrailToolsQuickAccess.h"
#include "UI.h"
#include "WorldClick.h"

namespace
{
	constexpr const char* KB_TOGGLE = "KB_TRLS_TOGGLE";

	void OnTogglePad(const char*, bool release)
	{
		if (release)
			return;
		G::ShowTrailTools = !G::ShowTrailTools;
		if (G::ShowTrailTools)
			TrailToolsPad::Open();
		Settings::SetDirty();
	}
}

void EntryDetail::AddonLoad(AddonAPI_t* api)
{
	G::API = api;

	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(api->ImguiContext));
	ImGui::SetAllocatorFunctions(
		reinterpret_cast<void* (*)(size_t, void*)>(api->ImguiMalloc),
		reinterpret_cast<void (*)(void*, void*)>(api->ImguiFree));

	G::NexusLink = static_cast<NexusLinkData_t*>(api->DataLink_Get(DL_NEXUS_LINK));
	G::Mumble = static_cast<MumbleLinkedMem*>(api->DataLink_Get(DL_MUMBLE_LINK));

	CrashTrail::Install();
	Settings::Load();
	PathingTrails::Init();

	api->GUI_Register(RT_Render, UI_Render);
	api->GUI_Register(RT_OptionsRender, UI_Options);
	api->WndProc_Register(WorldClick::WndProc);
	api->InputBinds_RegisterWithString(KB_TOGGLE, OnTogglePad, "ALT+SHIFT+T");
	TrailToolsQuickAccess::Init();

	api->Log(LOGL_INFO, ADDON_NAME,
		"Loaded — Alt+Shift+T toggles Trail Tools.");
}

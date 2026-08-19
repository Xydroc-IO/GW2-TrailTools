#include <windows.h>

#include "AddonVersion.h"
#include "Globals.h"
#include "entryInternal.h"

namespace G
{
	AddonDefinition_t AddonDef{};
	AddonAPI_t*       API       = nullptr;
	NexusLinkData_t*  NexusLink = nullptr;
	MumbleLinkedMem*  Mumble    = nullptr;
	HMODULE           Self      = nullptr;

	bool  ShowTrailTools = true;
	bool  ShowWiki = false;
	bool  ShowNotes = false;
	bool  ShowTpWatch = false;
	bool  HideWhenMapOpen = true;
	bool  HideOutOfGameplay = true;

	float Opacity = 0.97f;
	float FontScale = 1.25f;
	bool  FontScaleAuto = false;
	float WorldTrailMaxDist = 120.f;
	float WorldTrailWidth = 0.55f;
	float WorldTrailPlayerClear = 0.f;
	bool  WorldTrailPlayerClearOn = false;
	bool  WorldTrailUseTexture = false;
	float WorldMarkerPlayerClear = 1.f;
	float WorldMarkerScale = 2.f;
	float CompassMarkerScale = 1.f;
	float SideRailW = 0.f;
	float WindowWidth = 0.f;
	float WindowHeight = 0.f;
	float WindowPosX = 60.f;
	float WindowPosY = 60.f;

	char  PathingEnabled[8192] = "";

	PadGeom PadTrailTools{};
	PadGeom PadTrailEditor{};
	PadGeom PadMarkerEditor{};
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
		G::Self = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
	G::AddonDef.Signature        = ADDON_SIG;
	G::AddonDef.APIVersion       = NEXUS_API_VERSION;
	G::AddonDef.Name             = ADDON_NAME;
	G::AddonDef.Version.Major    = ADDON_VERSION_MAJOR;
	G::AddonDef.Version.Minor    = ADDON_VERSION_MINOR;
	G::AddonDef.Version.Build    = ADDON_VERSION_BUILD;
	G::AddonDef.Version.Revision = ADDON_VERSION_REVISION;
	G::AddonDef.Author           = "xydroc";
	G::AddonDef.Description      =
		"Full TacO/Blish trail & marker pack authoring for Guild Wars 2 "
		"(Raidcore Nexus). Draft world GPS preview, import/build .taco — "
		"no game memory R/W.";
	G::AddonDef.Load             = EntryDetail::AddonLoad;
	G::AddonDef.Unload           = EntryDetail::AddonUnload;
	G::AddonDef.Flags            = AF_None;
	G::AddonDef.Provider         = UP_GitHub;
	G::AddonDef.UpdateLink       = "https://github.com/Xydroc-IO/GW2-TrailTools";
	return &G::AddonDef;
}

#include "entryInternal.h"

#include "Globals.h"
#include "PathingTrails.h"
#include "Settings.h"
#include "TrailToolsQuickAccess.h"
#include "TrailToolsShared.h"
#include "UI.h"

void EntryDetail::AddonUnload()
{
	if (!G::API)
		return;

	TrailToolsDetail::CloseAllPopouts();
	PathingTrails::SerializeEnabledPaths(G::PathingEnabled, sizeof(G::PathingEnabled));
	PathingTrails::Shutdown();
	TrailToolsQuickAccess::Shutdown();

	Settings::SetDirty();
	Settings::Save(true);

	G::API->GUI_Deregister(UI_Render);
	G::API->GUI_Deregister(UI_Options);
	G::API->InputBinds_Deregister("KB_TRLS_TOGGLE");

	G::API = nullptr;
	G::NexusLink = nullptr;
	G::Mumble = nullptr;
}

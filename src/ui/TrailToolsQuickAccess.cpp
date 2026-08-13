#include "TrailToolsQuickAccess.h"

#include "Globals.h"
#include "TrailToolsIcon.h"

namespace
{
	constexpr const char* kQaId = "QA_GW2_TRAILTOOLS";
	/* Bump suffix when baked PNG changes — Nexus caches textures by id. */
	constexpr const char* kTexId = "TEX_GW2_TRAILTOOLS_QA_v3";
	constexpr const char* kTexHoverId = "TEX_GW2_TRAILTOOLS_QA_HOVER_v3";
	constexpr const char* kKbToggle = "KB_TRLS_TOGGLE";
	bool gAdded = false;
}

void TrailToolsQuickAccess::Init()
{
	if (!G::API || gAdded)
		return;
	if (!G::API->Textures_GetOrCreateFromMemory || !G::API->QuickAccess_Add)
		return;

	G::API->Textures_GetOrCreateFromMemory(
		kTexId,
		const_cast<unsigned char*>(kTrailToolsIconPng),
		static_cast<uint64_t>(kTrailToolsIconPng_len));
	G::API->Textures_GetOrCreateFromMemory(
		kTexHoverId,
		const_cast<unsigned char*>(kTrailToolsIconHoverPng),
		static_cast<uint64_t>(kTrailToolsIconHoverPng_len));

	G::API->QuickAccess_Add(
		kQaId,
		kTexId,
		kTexHoverId,
		kKbToggle,
		"Trail Tools (Alt+Shift+T)");
	gAdded = true;

	if (G::API->Log)
		G::API->Log(LOGL_INFO, ADDON_NAME, "QuickAccess icon registered");
}

void TrailToolsQuickAccess::Shutdown()
{
	if (!G::API || !gAdded)
		return;
	if (G::API->QuickAccess_Remove)
		G::API->QuickAccess_Remove(kQaId);
	gAdded = false;
}

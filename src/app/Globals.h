#pragma once

#include <cstdint>

#include "nexus/Nexus.h"

#define ADDON_NAME "GW2-TrailTools"
#define ADDON_SIG  0x54524C53u /* 'TRLS' */

struct MumbleContext
{
	unsigned char serverAddress[28];
	uint32_t mapId;
	uint32_t mapType;
	uint32_t shardId;
	uint32_t instance;
	uint32_t buildId;
	uint32_t uiState;
	uint16_t compassWidth;
	uint16_t compassHeight;
	float    compassRotation;
	float    playerX;
	float    playerY;
	float    mapCenterX;
	float    mapCenterY;
	float    mapScale;
	uint32_t processId;
	uint8_t  mountIndex;
};

struct MumbleLinkedMem
{
	uint32_t uiVersion;
	uint32_t uiTick;
	float    fAvatarPosition[3];
	float    fAvatarFront[3];
	float    fAvatarTop[3];
	wchar_t  name[256];
	float    fCameraPosition[3];
	float    fCameraFront[3];
	float    fCameraTop[3];
	wchar_t  identity[256];
	uint32_t context_len;
	unsigned char context[256];
	wchar_t  description[2048];
};

enum class UiStateBits : uint32_t
{
	MapOpen            = 1u << 0,
	CompassTopRight    = 1u << 1,
	CompassRotation    = 1u << 2,
	GameFocus          = 1u << 3,
	Competitive        = 1u << 4,
	TextboxFocus       = 1u << 5,
	InCombat           = 1u << 6,
};

namespace G
{
	extern AddonDefinition_t AddonDef;
	extern AddonAPI_t*       API;
	extern NexusLinkData_t*  NexusLink;
	extern MumbleLinkedMem*  Mumble;
	extern HMODULE           Self;

	extern bool  ShowTrailTools;
	/* PadDock layout fields (no CEF helper window in this addon). */
	extern bool  ShowWiki;
	extern bool  ShowNotes;
	extern bool  ShowTpWatch;
	extern bool  HideWhenMapOpen;
	extern bool  HideOutOfGameplay;

	extern float Opacity;
	extern float FontScale;
	extern bool  FontScaleAuto;
	extern float WorldTrailMaxDist;
	extern float WorldTrailWidth;
	extern float WorldTrailPlayerClear;
	extern float WorldMarkerPlayerClear;
	extern float WorldMarkerScale;
	extern float CompassMarkerScale;
	extern float SideRailW;
	extern float WindowWidth;
	extern float WindowHeight;
	extern float WindowPosX;
	extern float WindowPosY;

	/* '|' separated category paths — enable after Build .taco / Copy from Pathing. */
	extern char  PathingEnabled[8192];

	struct PadGeom
	{
		float x = -1.f;
		float y = -1.f;
		float w = 0.f;
		float h = 0.f;
	};
	extern PadGeom PadTrailTools;
	extern PadGeom PadTrailEditor;
	extern PadGeom PadMarkerEditor;
}

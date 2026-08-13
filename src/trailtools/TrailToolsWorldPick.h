#pragma once

#include "WorldGpsMath.h"

#include <cstdint>

/* Mumble camera × feet-Y plane pick (no game memory / Present hooks). */
namespace TrailToolsWorldPick
{
	/* Tick once per frame from UI_Render after pads — applies click if enabled. */
	void Tick();
	bool CameraRay(WorldGpsMath::Vec3& cam, WorldGpsMath::Vec3& dir);
	bool RayPlaneY(float planeY, float& outX, float& outY, float& outZ);
	bool RayFeetPlane(float& outX, float& outY, float& outZ);
	int NearestTrailPointScreen(float mx, float my, float maxPx);
	int NearestPoiScreen(float mx, float my, float maxPx, uint32_t mapId);
}

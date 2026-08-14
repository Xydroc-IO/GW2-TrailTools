#pragma once

/* Local ground: walked Mumble feet + draft + open-pack points, plane fit.
   Not TacO mesh snap — no game collision / memory. */
namespace TrailToolsGround
{
	void TickSample();
	bool RaySnap(float& outX, float& outY, float& outZ);
	float EstimateY(float x, float z, float fallback);
	int  PoseSamples();
}

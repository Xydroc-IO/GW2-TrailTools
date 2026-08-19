#pragma once

#include "WorldGpsMath.h"

namespace TrailToolsUberToolDetail
{
	enum class Kind { None, Trail, Poi };
	enum class Axis { None, X, Y, Z, Plane };

	struct State
	{
		Kind kind = Kind::None;
		int  index = -1;
		Axis drag = Axis::None;
		bool pushed = false;
		float origX = 0.f, origY = 0.f, origZ = 0.f;
		float axisT0 = 0.f;
		float grabMx = 0.f, grabMy = 0.f;
	};

	extern State gSt;

	inline WorldGpsMath::Vec3 AxisDir(Axis a)
	{
		if (a == Axis::X)
			return { 1.f, 0.f, 0.f };
		if (a == Axis::Y)
			return { 0.f, 1.f, 0.f };
		return { 0.f, 0.f, 1.f };
	}

	bool GetSel(WorldGpsMath::Vec3& o);
	float GizmoLen(const WorldGpsMath::Vec3& origin);
	int HitAxis(const WorldGpsMath::Vec3& origin, const WorldGpsMath::Mat4& vp,
		float sw, float sh, float mx, float my);
}

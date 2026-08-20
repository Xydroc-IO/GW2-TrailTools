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
	extern int   gLockPoi; /* >=0: FollowSelection stays on this draft POI */

	inline WorldGpsMath::Vec3 AxisDir(Axis a)
	{
		if (a == Axis::X)
			return { 1.f, 0.f, 0.f };
		if (a == Axis::Y)
			return { 0.f, 1.f, 0.f };
		return { 0.f, 0.f, 1.f };
	}

	bool GetSel(WorldGpsMath::Vec3& o);
	void SetSel(const WorldGpsMath::Vec3& o);
	float GizmoLen(const WorldGpsMath::Vec3& origin);
	int HitAxis(const WorldGpsMath::Vec3& origin, const WorldGpsMath::Mat4& vp,
		float sw, float sh, float mx, float my);

	inline float DistPtSeg2(float px, float py, float ax, float ay, float bx, float by, float& t)
	{
		const float abx = bx - ax, aby = by - ay;
		const float apx = px - ax, apy = py - ay;
		const float ab2 = abx * abx + aby * aby;
		t = (ab2 > 1e-8f) ? (apx * abx + apy * aby) / ab2 : 0.f;
		if (t < 0.f)
			t = 0.f;
		else if (t > 1.f)
			t = 1.f;
		const float qx = ax + abx * t, qy = ay + aby * t;
		const float dx = px - qx, dy = py - qy;
		return dx * dx + dy * dy;
	}
}

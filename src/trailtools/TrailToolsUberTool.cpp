#include "TrailToolsUberTool.h"
#include "TrailToolsUberToolInternal.h"

#include "Globals.h"
#include "TrailToolsShared.h"
#include "TrailToolsTrailGeom.h"
#include "WorldGpsMath.h"

#include <algorithm>
#include <cmath>

namespace TrailToolsUberToolDetail
{
	State gSt;
	int   gLockPoi = -1;
}

bool TrailToolsUberToolDetail::GetSel(WorldGpsMath::Vec3& o)
{
	using namespace TrailToolsDetail;
	if (gSt.kind == Kind::Trail)
	{
		DraftTrail& tr = RecordingTrail();
		if (gSt.index < 0 || gSt.index >= static_cast<int>(tr.points.size()))
			return false;
		const auto& p = tr.points[static_cast<size_t>(gSt.index)];
		if (TrailToolsTrailGeom::IsBreak(p))
			return false;
		o = { p.x, p.y, p.z };
		return true;
	}
	if (gSt.kind == Kind::Poi)
	{
		if (gSt.index < 0 || gSt.index >= static_cast<int>(gDraft.pois.size()))
			return false;
		const auto& p = gDraft.pois[static_cast<size_t>(gSt.index)];
		o = { p.x, p.y, p.z };
		return true;
	}
	return false;
}

void TrailToolsUberToolDetail::SetSel(const WorldGpsMath::Vec3& o)
{
	using namespace TrailToolsDetail;
	if (gSt.kind == Kind::Trail)
	{
		DraftTrail& tr = RecordingTrail();
		if (gSt.index < 0 || gSt.index >= static_cast<int>(tr.points.size()))
			return;
		auto& p = tr.points[static_cast<size_t>(gSt.index)];
		p.x = o.x;
		p.y = o.y;
		p.z = o.z;
		RecordingTrailDirty() = true;
		RecordingSelectedPoint() = gSt.index;
		return;
	}
	if (gSt.kind == Kind::Poi && gSt.index >= 0 &&
		gSt.index < static_cast<int>(gDraft.pois.size()))
	{
		auto& p = gDraft.pois[static_cast<size_t>(gSt.index)];
		p.x = o.x;
		p.y = o.y;
		p.z = o.z;
		gDraft.selectedPoi = gSt.index;
	}
}

float TrailToolsUberToolDetail::GizmoLen(const WorldGpsMath::Vec3& origin)
{
	using WorldGpsMath::Vec3;
	if (!G::Mumble)
		return 8.f;
	Vec3 cam{
		G::Mumble->fCameraPosition[0],
		G::Mumble->fCameraPosition[1],
		G::Mumble->fCameraPosition[2]
	};
	const float dist = std::sqrt((origin - cam).LengthSq());
	return std::clamp(dist * 0.22f, 4.f, 14.f);
}

int TrailToolsUberToolDetail::HitAxis(const WorldGpsMath::Vec3& origin, const WorldGpsMath::Mat4& vp,
	float sw, float sh, float mx, float my)
{
	using WorldGpsMath::Vec3;
	const float len = GizmoLen(origin);
	float ox = 0.f, oy = 0.f;
	if (!WorldGpsMath::WorldToScreen(origin, vp, sw, sh, ox, oy))
		return 0;
	int best = 0;
	float bestD = 40.f * 40.f;
	const Axis axes[3] = { Axis::X, Axis::Y, Axis::Z };
	for (int i = 0; i < 3; ++i)
	{
		const Vec3 tip = origin + AxisDir(axes[i]) * len;
		float tx = 0.f, ty = 0.f;
		if (!WorldGpsMath::WorldToScreen(tip, vp, sw, sh, tx, ty))
			continue;
		float t = 0.f;
		float d = DistPtSeg2(mx, my, ox, oy, tx, ty, t);
		const float tdx = tx - mx, tdy = ty - my;
		const float tipD = tdx * tdx + tdy * tdy;
		if (tipD < d)
			d = tipD;
		if (d < bestD)
		{
			bestD = d;
			best = static_cast<int>(axes[i]);
		}
	}
	return best;
}

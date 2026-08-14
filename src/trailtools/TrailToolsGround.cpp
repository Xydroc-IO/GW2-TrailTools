#include "TrailToolsGround.h"

#include "Globals.h"
#include "PackEditInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsWorldPick.h"
#include "WorldGpsMath.h"

#include <cmath>
#include <cstdint>

namespace
{
	constexpr int kCap = 192;
	constexpr int kFitMax = 64;
	constexpr float kMinStep = 0.35f;
	constexpr float kRadius = 40.f;

	struct Sample
	{
		float x = 0.f, y = 0.f, z = 0.f, d2 = 0.f;
	};

	Sample gPose[kCap]{};
	int gN = 0;
	int gHead = 0;
	uint32_t gMap = 0;
	float gLx = 0.f, gLy = 0.f, gLz = 0.f;
	bool gHave = false;

	void Clear()
	{
		gN = 0;
		gHead = 0;
		gHave = false;
	}

	void Push(float x, float y, float z)
	{
		gPose[gHead] = { x, y, z, 0.f };
		gHead = (gHead + 1) % kCap;
		if (gN < kCap)
			++gN;
	}

	void AddNear(Sample* fit, int& nf, float sx, float sy, float sz,
		float qx, float qz)
	{
		const float dx = sx - qx;
		const float dz = sz - qz;
		const float d2 = dx * dx + dz * dz;
		if (d2 > kRadius * kRadius)
			return;
		if (nf < kFitMax)
		{
			fit[nf++] = { sx, sy, sz, d2 };
			return;
		}
		int worst = 0;
		for (int i = 1; i < nf; ++i)
		{
			if (fit[i].d2 > fit[worst].d2)
				worst = i;
		}
		if (d2 < fit[worst].d2)
			fit[worst] = { sx, sy, sz, d2 };
	}

	bool Solve3(
		double a00, double a01, double a02, double b0,
		double a10, double a11, double a12, double b1,
		double a20, double a21, double a22, double b2,
		double& x0, double& x1, double& x2)
	{
		const double det =
			a00 * (a11 * a22 - a12 * a21) -
			a01 * (a10 * a22 - a12 * a20) +
			a02 * (a10 * a21 - a11 * a20);
		if (std::fabs(det) < 1e-8)
			return false;
		x0 = (b0 * (a11 * a22 - a12 * a21) -
			a01 * (b1 * a22 - a12 * b2) +
			a02 * (b1 * a21 - a11 * b2)) / det;
		x1 = (a00 * (b1 * a22 - a12 * b2) -
			b0 * (a10 * a22 - a12 * a20) +
			a02 * (a10 * b2 - b1 * a20)) / det;
		x2 = (a00 * (a11 * b2 - b1 * a21) -
			a01 * (a10 * b2 - b1 * a20) +
			b0 * (a10 * a21 - a11 * a20)) / det;
		return std::isfinite(x0) && std::isfinite(x1) && std::isfinite(x2);
	}

	float PlaneOrIdw(const Sample* fit, int nf, float qx, float qz, float fallback)
	{
		if (nf <= 0)
			return fallback;
		double wsum = 0.0, ysum = 0.0;
		double xx = 0.0, xz = 0.0, zz = 0.0, xw = 0.0, zw = 0.0;
		double xy = 0.0, zy = 0.0, yw = 0.0;
		for (int i = 0; i < nf; ++i)
		{
			const double X = static_cast<double>(fit[i].x - qx);
			const double Z = static_cast<double>(fit[i].z - qz);
			const double Y = static_cast<double>(fit[i].y);
			const double w = 1.0 / (static_cast<double>(fit[i].d2) + 0.35);
			wsum += w;
			ysum += w * Y;
			xx += w * X * X;
			xz += w * X * Z;
			zz += w * Z * Z;
			xw += w * X;
			zw += w * Z;
			xy += w * X * Y;
			zy += w * Z * Y;
			yw += w * Y;
		}
		if (wsum < 1e-9)
			return fallback;
		double a = 0.0, b = 0.0, c = 0.0;
		if (nf >= 3 && Solve3(xx, xz, xw, xy, xz, zz, zw, zy, xw, zw, wsum, yw, a, b, c))
		{
			const float y = static_cast<float>(c);
			if (std::isfinite(y) && std::fabs(a) < 3.0 && std::fabs(b) < 3.0)
				return y;
		}
		return static_cast<float>(ysum / wsum);
	}

	void CollectDraft(Sample* fit, int& nf, uint32_t mapId, float qx, float qz)
	{
		using namespace TrailToolsDetail;
		auto trailPts = [&](const DraftTrail& tr) {
			if (mapId && tr.mapId && tr.mapId != mapId)
				return;
			const int n = static_cast<int>(tr.points.size());
			int step = n > 96 ? n / 64 : 1;
			if (step < 1)
				step = 1;
			for (int i = 0; i < n; i += step)
			{
				const auto& p = tr.points[static_cast<size_t>(i)];
				if (p.x == 0.f && p.y == 0.f && p.z == 0.f)
					continue;
				AddNear(fit, nf, p.x, p.y, p.z, qx, qz);
			}
		};
		trailPts(RecordingTrail());
		for (int i = 0; i < kMaxTrailEditors; ++i)
		{
			if (gTrailEditors[i].open)
				trailPts(gTrailEditors[i].trail);
		}
		for (const auto& t : gDraft.trails)
			trailPts(t);
		for (const auto& p : gDraft.pois)
		{
			if (mapId && p.mapId && p.mapId != mapId)
				continue;
			AddNear(fit, nf, p.x, p.y, p.z, qx, qz);
		}
	}

	void CollectPack(Sample* fit, int& nf, uint32_t mapId, float qx, float qz)
	{
		const auto& items = PackEdit::gDoc.items;
		for (size_t i = 0; i < items.size(); ++i)
		{
			const auto& it = items[i];
			if (it.tombstone)
				continue;
			if (mapId && it.mapId && it.mapId != mapId)
				continue;
			if (it.isTrail)
			{
				const int n = static_cast<int>(it.points.size());
				int step = n > 96 ? n / 64 : 1;
				if (step < 1)
					step = 1;
				for (int k = 0; k < n; k += step)
				{
					const auto& p = it.points[static_cast<size_t>(k)];
					if (p.x == 0.f && p.y == 0.f && p.z == 0.f)
						continue;
					AddNear(fit, nf, p.x, p.y, p.z, qx, qz);
				}
				continue;
			}
			AddNear(fit, nf, it.x, it.y, it.z, qx, qz);
		}
	}

	float HeightAt(float x, float z, float fallback)
	{
		Sample fit[kFitMax];
		int nf = 0;
		uint32_t mapId = 0;
		float ax = 0.f, ay = 0.f, az = 0.f;
		const bool pose = TrailToolsDetail::ReadMumblePose(mapId, ax, ay, az);
		for (int i = 0; i < gN; ++i)
			AddNear(fit, nf, gPose[i].x, gPose[i].y, gPose[i].z, x, z);
		if (pose)
			AddNear(fit, nf, ax, ay, az, x, z);
		CollectDraft(fit, nf, mapId, x, z);
		CollectPack(fit, nf, mapId, x, z);
		return PlaneOrIdw(fit, nf, x, z, fallback);
	}
}

float TrailToolsGround::EstimateY(float x, float z, float fallback)
{
	return HeightAt(x, z, fallback);
}

void TrailToolsGround::TickSample()
{
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!TrailToolsDetail::ReadMumblePose(mapId, x, y, z))
		return;
	if (mapId != gMap)
	{
		Clear();
		gMap = mapId;
	}
	if (gHave)
	{
		const float dx = x - gLx, dy = y - gLy, dz = z - gLz;
		if (dx * dx + dy * dy + dz * dz < kMinStep * kMinStep)
			return;
	}
	Push(x, y, z);
	gLx = x;
	gLy = y;
	gLz = z;
	gHave = true;
}

bool TrailToolsGround::RaySnap(float& outX, float& outY, float& outZ)
{
	uint32_t mapId = 0;
	float fx = 0.f, fy = 0.f, fz = 0.f;
	if (!TrailToolsDetail::ReadMumblePose(mapId, fx, fy, fz))
		return false;
	float hx = 0.f, hy = fy, hz = 0.f;
	if (!TrailToolsWorldPick::RayPlaneY(fy, hx, hy, hz))
		return false;
	for (int i = 0; i < 6; ++i)
	{
		const float gy = HeightAt(hx, hz, fy);
		if (!TrailToolsWorldPick::RayPlaneY(gy, hx, hy, hz))
			return false;
		hy = gy;
	}
	outX = hx;
	outY = HeightAt(hx, hz, hy);
	outZ = hz;
	return WorldGpsMath::ReasonablePos(outX, outY, outZ);
}

int TrailToolsGround::PoseSamples()
{
	return gN;
}

#include "WorldGpsMath.h"

#include "Globals.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace WorldGpsMath
{
	bool Finite3(float x, float y, float z)
	{
		return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
	}

	bool ReasonablePos(float x, float y, float z)
	{
		return Finite3(x, y, z) &&
			std::fabs(x) < 1.0e6f && std::fabs(y) < 1.0e6f && std::fabs(z) < 1.0e6f;
	}

	float ParseFovRadians()
	{
		if (!G::Mumble)
			return kDefaultFov;
		char id[260]{};
		const wchar_t* w = G::Mumble->identity;
		size_t n = 0;
		for (; n < 255 && w[n]; ++n)
			id[n] = (w[n] < 128) ? static_cast<char>(w[n]) : ' ';
		id[n] = 0;
		const char* p = std::strstr(id, "\"fov\"");
		if (!p)
			p = std::strstr(id, "\"FOV\"");
		if (!p)
			return kDefaultFov;
		p = std::strchr(p, ':');
		if (!p)
			return kDefaultFov;
		const float fov = static_cast<float>(std::atof(p + 1));
		return (std::isfinite(fov) && fov > 0.2f && fov < 3.f) ? fov : kDefaultFov;
	}

	bool BuildViewProj(float screenW, float screenH, Mat4& out, Vec3& camOut)
	{
		if (!G::Mumble)
			return false;
		const float* cp = G::Mumble->fCameraPosition;
		const float* cf = G::Mumble->fCameraFront;
		const float* ct = G::Mumble->fCameraTop;
		if (!ReasonablePos(cp[0], cp[1], cp[2]))
			return false;
		if (!Finite3(cf[0], cf[1], cf[2]) || !Finite3(ct[0], ct[1], ct[2]))
			return false;

		Vec3 camPos{cp[0], cp[1], cp[2]};
		Vec3 f = Vec3{cf[0], cf[1], cf[2]}.Normalised();
		if (f.LengthSq() < 0.5f)
			return false;
		Vec3 topHint{ct[0], ct[1], ct[2]};
		Vec3 worldUp = (topHint.LengthSq() > 0.01f) ? topHint.Normalised()
			: Vec3{0.f, 1.f, 0.f};
		if (std::fabs(f.Dot(worldUp)) > 0.98f)
			worldUp = Vec3{0.f, 1.f, 0.f};
		Vec3 r = worldUp.Cross(f).Normalised();
		if (r.LengthSq() < 0.5f)
		{
			worldUp = Vec3{0.f, 1.f, 0.f};
			r = worldUp.Cross(f).Normalised();
		}
		if (r.LengthSq() < 0.5f)
			return false;
		Vec3 u = f.Cross(r).Normalised();
		if (u.LengthSq() < 0.5f)
			return false;

		Mat4 view{};
		view.m[0][0] = r.x; view.m[1][0] = r.y; view.m[2][0] = r.z;
		view.m[3][0] = -r.Dot(camPos);
		view.m[0][1] = u.x; view.m[1][1] = u.y; view.m[2][1] = u.z;
		view.m[3][1] = -u.Dot(camPos);
		view.m[0][2] = f.x; view.m[1][2] = f.y; view.m[2][2] = f.z;
		view.m[3][2] = -f.Dot(camPos);
		view.m[3][3] = 1.f;

		const float fov = ParseFovRadians();
		const float aspect = (screenH > 0.f) ? screenW / screenH : 1.7778f;
		const float tanHalfFov = std::tan(fov * 0.5f);
		if (!std::isfinite(tanHalfFov) || tanHalfFov < 1e-4f)
			return false;

		Mat4 proj{};
		proj.m[0][0] = 1.f / (aspect * tanHalfFov);
		proj.m[1][1] = 1.f / tanHalfFov;
		proj.m[2][2] = kFarClip / (kFarClip - kNearClip);
		proj.m[2][3] = 1.f;
		proj.m[3][2] = -(kNearClip * kFarClip) / (kFarClip - kNearClip);

		out = proj * view;
		camOut = camPos;
		return true;
	}

	bool InvertMat4(const Mat4& in, Mat4& out)
	{
		float a[4][8]{};
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
				a[r][c] = in.m[c][r];
			a[r][4 + r] = 1.f;
		}
		for (int i = 0; i < 4; ++i)
		{
			int piv = i;
			for (int r = i + 1; r < 4; ++r)
			{
				if (std::fabs(a[r][i]) > std::fabs(a[piv][i]))
					piv = r;
			}
			if (std::fabs(a[piv][i]) < 1e-12f)
				return false;
			if (piv != i)
			{
				for (int c = 0; c < 8; ++c)
					std::swap(a[i][c], a[piv][c]);
			}
			const float d = a[i][i];
			for (int c = 0; c < 8; ++c)
				a[i][c] /= d;
			for (int r = 0; r < 4; ++r)
			{
				if (r == i)
					continue;
				const float f = a[r][i];
				for (int c = 0; c < 8; ++c)
					a[r][c] -= f * a[i][c];
			}
		}
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				out.m[c][r] = a[r][4 + c];
		return true;
	}

	bool ScreenRay(float mx, float my, float screenW, float screenH,
		const Mat4& viewProj, Vec3& origin, Vec3& dir)
	{
		if (screenW < 8.f || screenH < 8.f)
			return false;
		Mat4 inv{};
		if (!InvertMat4(viewProj, inv))
			return false;
		const float ndcX = (mx / screenW) * 2.f - 1.f;
		const float ndcY = 1.f - (my / screenH) * 2.f;
		auto unproj = [&](float ndcZ, Vec3& p) -> bool
		{
			float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
			inv.Transform(ndcX, ndcY, ndcZ, x, y, z, w);
			if (!std::isfinite(w) || std::fabs(w) < 1e-8f)
				return false;
			p = { x / w, y / w, z / w };
			return ReasonablePos(p.x, p.y, p.z);
		};
		Vec3 p0{}, p1{};
		if (!unproj(0.f, p0) || !unproj(1.f, p1))
			return false;
		origin = p0;
		dir = (p1 - p0).Normalised();
		return dir.LengthSq() > 0.5f;
	}

	float TrailPlayerClearMul()
	{
		if (!G::WorldTrailPlayerClearOn)
			return 0.f;
		const float m = G::WorldTrailPlayerClear;
		if (!(m > 0.01f))
			return 1.f;
		return std::clamp(m, 0.f, 3.f);
	}

	bool WorldToScreen(const Vec3& world, const Mat4& viewProj,
		float screenW, float screenH, float& sx, float& sy)
	{
		if (!ReasonablePos(world.x, world.y, world.z))
			return false;
		float cx, cy, cz, cw;
		viewProj.Transform(world.x, world.y, world.z, cx, cy, cz, cw);
		if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(cw) || cw <= 1.25f)
			return false;
		const float ndcX = cx / cw;
		const float ndcY = cy / cw;
		if (!std::isfinite(ndcX) || !std::isfinite(ndcY))
			return false;
		if (std::fabs(ndcX) > 40.f || std::fabs(ndcY) > 40.f)
			return false;
		sx = (ndcX + 1.f) * 0.5f * screenW;
		sy = (-ndcY + 1.f) * 0.5f * screenH;
		return std::isfinite(sx) && std::isfinite(sy);
	}

	float ClipW(const Vec3& world, const Mat4& viewProj)
	{
		float cx, cy, cz, cw;
		viewProj.Transform(world.x, world.y, world.z, cx, cy, cz, cw);
		return cw;
	}

	Vec3 Lerp3(const Vec3& a, const Vec3& b, float t)
	{
		return {
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t,
		};
	}

	void TrailFadeRange(float maxDist, float& fadeStart, float& fadeEnd)
	{
		/* Soft far edge - keep corridors visible; hard cuts blinked mid-path. */
		fadeStart = maxDist * 0.92f;
		fadeEnd = maxDist * 1.85f;
		fadeEnd = std::max(fadeStart + 20.f, fadeEnd);
	}
}

#include "PackEditInternal.h"

#include "Globals.h"
#include "TrailToolsShared.h"
#include "TrailToolsWorldPick.h"
#include "WorldClick.h"
#include "WorldGpsMath.h"

#include "imgui/imgui.h"

#include <cmath>
#include <vector>

void PackEdit::Tick()
{
	static bool sDrag = false;
	static float sHx = 0.f, sHz = 0.f, sRot = 0.f;
	struct Orig { int i; float x, z; };
	static std::vector<Orig> sOrig;

	if (!WorldClick::LeftHeld())
		sDrag = false;
	if (!gDoc.gizmoOn || gDoc.items.empty())
		return;

	float mx = 0.f, my = 0.f;
	const bool click = WorldClick::TakeLeftDown(mx, my);
	if (click && !ImGui::GetIO().WantCaptureMouse)
	{
		uint32_t mapId = 0;
		if (G::Mumble)
		{
			const auto* ctx = reinterpret_cast<const MumbleContext*>(G::Mumble->context);
			if (ctx)
				mapId = ctx->mapId;
		}
		using namespace WorldGpsMath;
		Mat4 vp{};
		Vec3 cam{};
		const ImGuiIO& io = ImGui::GetIO();
		if (BuildViewProj(io.DisplaySize.x, io.DisplaySize.y, vp, cam))
		{
			int best = -1;
			float bestD = 22.f * 22.f;
			for (int i = 0; i < static_cast<int>(gDoc.items.size()); ++i)
			{
				const auto& it = gDoc.items[static_cast<size_t>(i)];
				if (it.tombstone)
					continue;
				if (gDoc.thisMapOnly && mapId && it.mapId && it.mapId != mapId)
					continue;
				if (!gDoc.hidden.empty() && CategoryHidden(it.type))
					continue;
				float sx = 0.f, sy = 0.f;
				if (it.isTrail)
				{
					for (int k = 0; k < static_cast<int>(it.points.size()); ++k)
					{
						const auto& pt = it.points[static_cast<size_t>(k)];
						if (!WorldToScreen({ pt.x, pt.y, pt.z }, vp,
							io.DisplaySize.x, io.DisplaySize.y, sx, sy))
							continue;
						const float dx = sx - mx, dy = sy - my;
						const float d = dx * dx + dy * dy;
						if (d < bestD)
						{
							bestD = d;
							best = i;
							gDoc.selPoint = k;
						}
					}
					continue;
				}
				if (!WorldToScreen({ it.x, it.y, it.z }, vp,
					io.DisplaySize.x, io.DisplaySize.y, sx, sy))
					continue;
				const float dx = sx - mx, dy = sy - my;
				const float d = dx * dx + dy * dy;
				if (d < bestD)
				{
					bestD = d;
					best = i;
				}
			}
			if (best >= 0)
			{
				if (io.KeyCtrl)
					SelectToggle(best);
				else
					RevealItem(best);
			}
		}
	}

	PePathable* p = Selected();
	if (!p || !WorldClick::LeftHeld() || ImGui::GetIO().WantCaptureMouse)
		return;
	float hx = 0.f, hy = 0.f, hz = 0.f;
	if (!TrailToolsWorldPick::RayFeetPlane(hx, hy, hz))
		return;
	if (!sDrag)
	{
		PushUndo();
		sDrag = true;
		sHx = hx;
		sHz = hz;
		sRot = p->rotate;
		sOrig.clear();
		const std::vector<int>& sel = gDoc.selItems.empty()
			? std::vector<int>{ gDoc.selItem } : gDoc.selItems;
		for (int i : sel)
		{
			if (i < 0 || i >= static_cast<int>(gDoc.items.size()))
				continue;
			auto& it = gDoc.items[static_cast<size_t>(i)];
			if (it.tombstone || it.isTrail)
				continue;
			sOrig.push_back({ i, it.x, it.z });
		}
	}
	if (p->isTrail && gDoc.selPoint >= 0 &&
		gDoc.selPoint < static_cast<int>(p->points.size()))
	{
		auto& pt = p->points[static_cast<size_t>(gDoc.selPoint)];
		pt.x += hx - sHx;
		pt.z += hz - sHz;
		sHx = hx;
		sHz = hz;
		gDoc.dirty = true;
		return;
	}
	if (gDoc.rotateMode)
	{
		p->rotate = sRot + (hx - sHx) * 8.f;
		gDoc.dirty = true;
		return;
	}
	const float dx = hx - sHx;
	const float dz = hz - sHz;
	if (sOrig.size() > 1)
	{
		for (const auto& o : sOrig)
			gDoc.items[static_cast<size_t>(o.i)].x = o.x + dx;
		for (const auto& o : sOrig)
			gDoc.items[static_cast<size_t>(o.i)].z = o.z + dz;
	}
	else
	{
		p->x += dx;
		p->z += dz;
		sHx = hx;
		sHz = hz;
	}
	gDoc.dirty = true;
}

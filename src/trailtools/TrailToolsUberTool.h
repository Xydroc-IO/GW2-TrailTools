#pragma once

/* Shared 3D gizmo for draft trail vectors and markers. Drag a vertex on the
   ground plane; RGB arrows lock X / Y / Z. Ctrl+click inserts on a trail. */
namespace TrailToolsUberTool
{
	/* True if this client click is on a vertex / gizmo (WndProc may swallow). */
	bool WantSwallow(float mx, float my);
	void FollowSelection(); /* gizmo tracks Select Nearest / list / Move to Feet */
	bool Tick();
	void Render();
}

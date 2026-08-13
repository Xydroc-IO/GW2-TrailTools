#pragma once

/* TacO-style 3D UberTool: click draft markers/trail verts in the world, RGB-axis
   gizmo to move, Ctrl+click to insert a trail vertex, right-click cancels a drag.
   Uses Mumble camera × plane math only (no game memory / Present hooks). */
namespace TrailToolsUberTool
{
	/* Returns true if the click/drag was consumed (WorldPick should skip). */
	bool Tick();
	void Render();
}

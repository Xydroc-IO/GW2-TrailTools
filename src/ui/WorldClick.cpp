#include "WorldClick.h"

#include "TrailToolsUberTool.h"

#include "imgui/imgui.h"

namespace
{
	bool  gWantUi = false;
	bool  gPending = false;
	bool  gHeld = false;
	bool  gSwallowed = false;
	bool  gHaveCursor = false;
	float gMx = 0.f, gMy = 0.f;

	void Track(LPARAM lParam)
	{
		gMx = static_cast<float>(static_cast<short>(LOWORD(lParam)));
		gMy = static_cast<float>(static_cast<short>(HIWORD(lParam)));
		gHaveCursor = true;
	}
}

UINT WorldClick::WndProc(HWND, UINT msg, WPARAM, LPARAM lParam)
{
	if (msg == WM_MOUSEMOVE)
		Track(lParam);
	if (msg == WM_LBUTTONDOWN)
	{
		Track(lParam);
		gHeld = true;
		if (!gWantUi)
		{
			gPending = true;
			if (TrailToolsUberTool::WantSwallow(gMx, gMy))
			{
				gSwallowed = true;
				return 0; /* keep camera look from eating gizmo drags */
			}
		}
	}
	else if (msg == WM_LBUTTONUP)
	{
		gHeld = false;
		if (gSwallowed)
		{
			gSwallowed = false;
			return 0;
		}
	}
	return 1;
}

bool WorldClick::TakeLeftDown(float& mx, float& my)
{
	if (!gPending)
		return false;
	gPending = false;
	mx = gMx;
	my = gMy;
	return true;
}

bool WorldClick::LeftHeld()
{
	return gHeld && !gWantUi;
}

bool WorldClick::Cursor(float& mx, float& my)
{
	if (!gHaveCursor)
		return false;
	mx = gMx;
	my = gMy;
	return true;
}

void WorldClick::TickImGui()
{
	gWantUi = ImGui::GetIO().WantCaptureMouse;
}

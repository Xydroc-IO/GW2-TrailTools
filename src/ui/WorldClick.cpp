#include "WorldClick.h"

#include "imgui/imgui.h"

namespace
{
	bool  gWantUi = false;
	bool  gPending = false;
	bool  gHeld = false;
	float gMx = 0.f, gMy = 0.f;
}

UINT WorldClick::WndProc(HWND, UINT msg, WPARAM, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN)
	{
		gHeld = true;
		if (!gWantUi)
		{
			gPending = true;
			gMx = static_cast<float>(static_cast<short>(LOWORD(lParam)));
			gMy = static_cast<float>(static_cast<short>(HIWORD(lParam)));
		}
	}
	else if (msg == WM_LBUTTONUP)
		gHeld = false;
	return 1; /* never swallow — game keeps camera look */
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

void WorldClick::TickImGui()
{
	gWantUi = ImGui::GetIO().WantCaptureMouse;
}

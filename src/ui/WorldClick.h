#pragma once

#include <windows.h>

/* Nexus WndProc world clicks — ImGui never sees LBUTTONDOWN on empty viewport. */
namespace WorldClick
{
	UINT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/* True once per left-down that missed ImGui (screen coords). */
	bool TakeLeftDown(float& mx, float& my);
	void DiscardPending(); /* drop a world click that landed on ImGui / our pads */
	bool LeftHeld();
	/* Latest client mouse from WndProc (true after any mouse message). */
	bool Cursor(float& mx, float& my);
	void TickImGui(); /* sync WantCaptureMouse before pad logic */
}

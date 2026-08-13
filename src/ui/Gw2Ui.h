#pragma once

#include "imgui/imgui.h"

struct ImGuiWindow;

/* Lean Gw2Ui for Trail Tools — CDN icons via Nexus; soft plate chrome (no ui-chrome pack). */
namespace Gw2Ui
{
	enum class Icon : int
	{
		Close        = 155014,
		Check        = 155023,
		Alert        = 155018,
		Key          = 155048,
		Bag          = 156670,
		Inventory    = 157098,
		Options      = 157109,
		Map          = 157122,
		TrailAnvil   = 155867,
		SettingsGear = 3713037,
	};

	void Request(int assetId);
	inline void Request(Icon icon) { Request(static_cast<int>(icon)); }
	void WarmCommon();

	bool Image(int assetId, float size = 24.f);
	bool Image(Icon icon, float size = 24.f);

	bool PaintPadChrome(float opacity = 1.f, bool omitLeftEdge = false,
		bool omitRightEdge = false, bool solidStack = false);
	void PaintNativeScrollbars(float opacity = 1.f, ImGuiWindow* root = nullptr);
	bool DrawPadTitleBar(const char* title, bool* pOpen, float opacity = 1.f,
		float leftExtend = 0.f, bool solidStack = false);

	inline ImGuiWindowFlags PadWindowFlags(ImGuiWindowFlags extra = 0)
	{
		return ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | extra;
	}

	bool IconButton(const char* id, int assetId, float size = 26.f);
	bool IconButton(const char* id, Icon icon, float size = 26.f);
	bool IconLabelButton(const char* label, int assetId, float iconSize = 20.f);
	bool IconLabelButton(const char* label, Icon icon, float iconSize = 20.f);
	bool RailToggle(const char* label, bool on, int assetId = 0, float iconSize = 18.f,
		bool showLabel = false);
	bool RailToggle(const char* label, bool on, Icon icon, float iconSize = 18.f,
		bool showLabel = false);
}

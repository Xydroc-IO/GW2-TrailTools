#pragma once

#include <windows.h>

/* Lean Wine detect — PadDock skips fragile ImGui window lookups under Wine. */
namespace EiRuntime
{
	inline bool IsWine()
	{
		static int sCached = -1;
		if (sCached < 0)
		{
			HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
			sCached = (ntdll && GetProcAddress(ntdll, "wine_get_version")) ? 1 : 0;
		}
		return sCached == 1;
	}
}

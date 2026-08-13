#pragma once

#include <string>

/* Runtime data: <GW2>/addons/GW2-TrailTools/
   DLL: <GW2>/addons/GW2-TrailTools.dll */
namespace AddonPaths
{
	std::wstring DataDir();
	std::string  DataDirUtf8();

	std::wstring PagesDir();
	std::wstring LiveCacheDir();
	std::wstring CacheDir();
	std::wstring CmdsDir();
	std::wstring ConfigDir();
	std::wstring ThemesDir();
	std::wstring CrashLogsDir();

	std::wstring EnsureUnder(const std::wstring& root, const wchar_t* relative);
}

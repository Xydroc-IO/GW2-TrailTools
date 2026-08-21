#pragma once

#include <string>
#include <vector>

/* Texture browser for default trail texture / marker icon (Nexus Options). */
namespace TrailToolsAssets
{
	struct Entry
	{
		std::string relPath;   /* pack-relative e.g. Data/Images/x.png */
		std::string label;     /* display */
		bool        fromTaco = false;
		std::string tacoName;  /* source pack filename when fromTaco */
		std::string zipEntry;  /* entry inside .taco */
	};

	void RefreshAuthoringList(std::vector<Entry>& out);
	void RefreshInstalledTacoList(std::vector<Entry>& out); /* PNG-like entries */
	/* Copy zipEntry from taco into authoring Markers/; returns new relative path. */
	bool ImportFromTaco(const std::wstring& tacoPath, const std::string& zipEntry,
		std::string& outRelPath, std::string& err);
	void DrawBrowserUi(); /* ImGui — default trail texture / marker icon picker */
}

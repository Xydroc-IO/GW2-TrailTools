#pragma once

#include "PackEdit.h"

#include <sstream>

namespace PackEdit
{
	extern PeDoc gDoc;
	PePathable* Selected();
	bool CategoryHidden(const std::string& typePath);
	void RevealItem(int index);
	void ToggleHidden(const std::string& path);
	void SelectToggle(int index);
	bool IsSelected(int index);
	PathingParse::MarkerStyle EffectiveStyle(const PePathable& p);
	void AppendStyleXml(std::ostringstream& os, const PathingParse::MarkerStyle& s);
	void DrawTree();
	void DrawDetails();
	void DrawResources();
	void DrawPopouts();
	void ClearHistory();
	void ApplyParsed(PeDoc& doc,
		const std::vector<PathingTrails::Category>& catRoots,
		std::vector<PathingParse::IndexedPoi>& pois,
		std::vector<PathingParse::IndexedTrail>& trails,
		std::unordered_map<std::string, PathingParse::MarkerStyle> styles);
}

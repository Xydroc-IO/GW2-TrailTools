#pragma once

#include "TrailToolsShared.h"

#include <string>

namespace TrailToolsXml
{
	/* One OverlayData file (categories + trails + markers). */
	std::string EmitOverlayData(const TrailToolsDetail::DraftPack& pack);
	std::string EmitTrailElement(const TrailToolsDetail::DraftTrail& trail);
	std::string EmitPoiElement(const TrailToolsDetail::DraftPoi& poi);

	bool WriteUtf8File(const std::wstring& path, const std::string& utf8);
	bool WriteOverlayFile(const std::wstring& path, const TrailToolsDetail::DraftPack& pack);
	/* Writes Pack.xml and removes leftover Pack_Menu.xml / Pack_Data.xml. */
	bool WritePackXmlLayout(const TrailToolsDetail::DraftPack& pack);
	/* If path is *_Menu.xml or *_Data.xml, rewrite to Pack.xml. */
	void CoerceSingleOverlayPath(std::wstring& path);
}

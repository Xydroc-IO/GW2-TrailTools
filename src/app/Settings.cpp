#include "Settings.h"

#include "AddonPaths.h"
#include "Globals.h"
#include "PadDock.h"
#include "TrailToolsBinds.h"
#include "TrailToolsShared.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <windows.h>

namespace
{
	bool gDirty = false;

	std::string SettingsPathUtf8()
	{
		return AddonPaths::DataDirUtf8() + "\\settings.ini";
	}

	bool ParseLine(const char* line, char* key, size_t keyLen, char* val, size_t valLen)
	{
		const char* eq = std::strchr(line, '=');
		if (!eq)
			return false;
		size_t kn = static_cast<size_t>(eq - line);
		if (kn == 0 || kn >= keyLen)
			return false;
		std::memcpy(key, line, kn);
		key[kn] = 0;
		std::snprintf(val, valLen, "%s", eq + 1);
		size_t vl = std::strlen(val);
		while (vl > 0 && (val[vl - 1] == '\r' || val[vl - 1] == '\n' || val[vl - 1] == ' '))
			val[--vl] = 0;
		return true;
	}

	bool AsBool(const char* v)
	{
		return v[0] == '1' || v[0] == 't' || v[0] == 'T' || v[0] == 'y' || v[0] == 'Y';
	}

	void Apply(const char* key, const char* val)
	{
		if (std::strcmp(key, "Opacity") == 0)
			G::Opacity = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "FontScale") == 0)
			G::FontScale = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "FontScaleAuto") == 0)
			G::FontScaleAuto = AsBool(val);
		else if (std::strcmp(key, "HideWhenMapOpen") == 0)
			G::HideWhenMapOpen = AsBool(val);
		else if (std::strcmp(key, "HideOutOfGameplay") == 0)
			G::HideOutOfGameplay = AsBool(val);
		else if (std::strcmp(key, "WorldTrailMaxDist") == 0)
			G::WorldTrailMaxDist = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "WorldTrailWidth") == 0)
			G::WorldTrailWidth = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "WorldTrailPlayerClear") == 0)
			G::WorldTrailPlayerClear = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "WorldTrailPlayerClearOn") == 0)
			G::WorldTrailPlayerClearOn = AsBool(val);
		else if (std::strcmp(key, "WorldTrailUseTexture") == 0)
			G::WorldTrailUseTexture = AsBool(val);
		else if (std::strcmp(key, "WorldMarkerPlayerClear") == 0)
			G::WorldMarkerPlayerClear = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "WorldMarkerScale") == 0)
			G::WorldMarkerScale = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "CompassMarkerScale") == 0)
			G::CompassMarkerScale = static_cast<float>(std::atof(val));
		else if (std::strcmp(key, "ShowTrailTools") == 0)
			G::ShowTrailTools = AsBool(val);
		else if (std::strcmp(key, "TrailToolsLastTrlDir") == 0)
		{
			std::snprintf(TrailToolsDetail::gDraft.lastTrlDir,
				sizeof(TrailToolsDetail::gDraft.lastTrlDir), "%s", val);
		}
		else if (std::strcmp(key, "TrailToolsXmlLayout") == 0)
			TrailToolsDetail::gDraft.xmlLayout = 0;
		else if (std::strcmp(key, "TrailToolsBinds") == 0)
			TrailToolsBinds::Deserialize(val);
		else if (std::strcmp(key, "PathingEnabled") == 0)
			std::snprintf(G::PathingEnabled, sizeof(G::PathingEnabled), "%s", val);
		else if (std::strcmp(key, "PadTrailTools") == 0)
			PadDock::ParseGeom(val, G::PadTrailTools);
		else if (std::strcmp(key, "PadTrailEditor") == 0)
			PadDock::ParseGeom(val, G::PadTrailEditor);
		else if (std::strcmp(key, "PadMarkerEditor") == 0)
			PadDock::ParseGeom(val, G::PadMarkerEditor);
	}
}

void Settings::SetDirty()
{
	gDirty = true;
}

void Settings::Load()
{
	AddonPaths::EnsureUnder(AddonPaths::DataDir(), L"pathing");
	const std::string path = SettingsPathUtf8();
	FILE* f = std::fopen(path.c_str(), "r");
	if (!f)
		return;
	char line[4096]{};
	char key[128]{};
	char val[3500]{};
	while (std::fgets(line, sizeof(line), f))
	{
		if (!ParseLine(line, key, sizeof(key), val, sizeof(val)))
			continue;
		Apply(key, val);
	}
	std::fclose(f);
	gDirty = false;
}

void Settings::Save(bool force)
{
	if (!force && !gDirty)
		return;
	static DWORD sLast = 0;
	const DWORD now = GetTickCount();
	if (!force && sLast != 0 && (now - sLast) < 2500)
		return;

	AddonPaths::DataDir();
	const std::string path = SettingsPathUtf8();
	FILE* f = std::fopen(path.c_str(), "w");
	if (!f)
		return;

	std::fprintf(f, "Opacity=%.3f\n", G::Opacity);
	std::fprintf(f, "FontScale=%.3f\n", G::FontScale);
	std::fprintf(f, "FontScaleAuto=%d\n", G::FontScaleAuto ? 1 : 0);
	std::fprintf(f, "HideWhenMapOpen=%d\n", G::HideWhenMapOpen ? 1 : 0);
	std::fprintf(f, "HideOutOfGameplay=%d\n", G::HideOutOfGameplay ? 1 : 0);
	std::fprintf(f, "WorldTrailMaxDist=%.1f\n", G::WorldTrailMaxDist);
	std::fprintf(f, "WorldTrailWidth=%.2f\n", G::WorldTrailWidth);
	std::fprintf(f, "WorldTrailPlayerClear=%.2f\n", G::WorldTrailPlayerClear);
	std::fprintf(f, "WorldTrailPlayerClearOn=%d\n", G::WorldTrailPlayerClearOn ? 1 : 0);
	std::fprintf(f, "WorldTrailUseTexture=%d\n", G::WorldTrailUseTexture ? 1 : 0);
	std::fprintf(f, "WorldMarkerPlayerClear=%.2f\n", G::WorldMarkerPlayerClear);
	std::fprintf(f, "WorldMarkerScale=%.2f\n", G::WorldMarkerScale);
	std::fprintf(f, "CompassMarkerScale=%.2f\n", G::CompassMarkerScale);
	std::fprintf(f, "ShowTrailTools=%d\n", G::ShowTrailTools ? 1 : 0);
	std::fprintf(f, "TrailToolsLastTrlDir=%s\n", TrailToolsDetail::gDraft.lastTrlDir);
	std::fprintf(f, "TrailToolsXmlLayout=0\n");
	std::fprintf(f, "TrailToolsBinds=%s\n", TrailToolsBinds::Serialize().c_str());
	std::fprintf(f, "PathingEnabled=%s\n", G::PathingEnabled);
	PadDock::WriteGeom(f, "PadTrailTools", G::PadTrailTools);
	PadDock::WriteGeom(f, "PadTrailEditor", G::PadTrailEditor);
	PadDock::WriteGeom(f, "PadMarkerEditor", G::PadMarkerEditor);
	std::fclose(f);
	gDirty = false;
	sLast = now;
}

void Settings::SaveNow()
{
	gDirty = true;
	Save(true);
}

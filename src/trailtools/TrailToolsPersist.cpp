#include "TrailToolsShared.h"
#include "TrailToolsTrl.h"
#include "TrailToolsXml.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <windows.h>

namespace
{
	std::wstring SessionPath()
	{
		return TrailToolsDetail::PackDir() + L"\\_draft_session.xml";
	}

	bool WriteAll(const std::wstring& path, const std::string& data)
	{
		HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (h == INVALID_HANDLE_VALUE)
			return false;
		DWORD w = 0;
		const BOOL ok = WriteFile(h, data.data(), static_cast<DWORD>(data.size()), &w, nullptr);
		CloseHandle(h);
		return ok && w == data.size();
	}

	bool ReadAll(const std::wstring& path, std::string& out)
	{
		HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (h == INVALID_HANDLE_VALUE)
			return false;
		LARGE_INTEGER sz{};
		if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0 || sz.QuadPart > 16 * 1024 * 1024)
		{
			CloseHandle(h);
			return false;
		}
		out.resize(static_cast<size_t>(sz.QuadPart));
		DWORD r = 0;
		const BOOL ok = ReadFile(h, out.data(), static_cast<DWORD>(out.size()), &r, nullptr);
		CloseHandle(h);
		return ok && r == out.size();
	}

	std::string Attr(const std::string& tag, const char* key)
	{
		const std::string needle = std::string(key) + "=\"";
		size_t p = tag.find(needle);
		if (p == std::string::npos)
			return {};
		p += needle.size();
		size_t e = tag.find('"', p);
		if (e == std::string::npos)
			return {};
		return tag.substr(p, e - p);
	}

	std::wstring RelToPackPath(const std::string& rel)
	{
		std::wstring p = TrailToolsDetail::PackDir();
		p.push_back(L'\\');
		for (char c : rel)
			p.push_back(c == '/' ? L'\\' : static_cast<wchar_t>(static_cast<unsigned char>(c)));
		return p;
	}

	/* Next `<Name .../>` or `<Name ...>...</Name>` (skips `<Names>`). */
	bool NextElem(const std::string& xml, const char* name, size_t& pos, std::string& tag)
	{
		const size_t nlen = std::strlen(name);
		while (true)
		{
			const size_t at = xml.find(name[0] == '<' ? std::string(name) : (std::string("<") + name), pos);
			if (at == std::string::npos)
				return false;
			const size_t after = at + 1 + nlen;
			if (after < xml.size())
			{
				const char c = xml[after];
				if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '/' || c == '>'))
				{
					pos = after;
					continue;
				}
			}
			const size_t gt = xml.find('>', at);
			if (gt == std::string::npos)
				return false;
			tag = xml.substr(at, gt - at);
			const bool self = gt > 0 && xml[gt - 1] == '/';
			if (self)
			{
				pos = gt + 1;
				return true;
			}
			const std::string close = std::string("</") + name + ">";
			const size_t end = xml.find(close, gt);
			pos = (end == std::string::npos) ? gt + 1 : end + close.size();
			return true;
		}
	}

	using TrailToolsDetail::CategoryNode;
	using TrailToolsDetail::DraftPoi;
	using TrailToolsDetail::DraftTrail;

	bool ParseOpenTag(const std::string& xml, size_t& pos, const char* name, std::string& tag,
		bool& selfClose)
	{
		const std::string open = std::string("<") + name;
		const size_t p = xml.find(open, pos);
		if (p == std::string::npos)
			return false;
		const size_t gt = xml.find('>', p);
		if (gt == std::string::npos)
			return false;
		tag = xml.substr(p, gt - p + 1);
		selfClose = tag.size() >= 2 && tag[tag.size() - 2] == '/';
		pos = gt + 1;
		return true;
	}

	void FillCategoryAttrs(CategoryNode& n, const std::string& tag)
	{
		n.name = Attr(tag, "name");
		n.displayName = Attr(tag, "DisplayName");
		if (n.displayName.empty())
			n.displayName = Attr(tag, "displayName");
		n.iconFile = Attr(tag, "iconFile");
		n.texture = Attr(tag, "texture");
		const std::string fn = Attr(tag, "fadeNear");
		if (!fn.empty()) n.fadeNear = static_cast<float>(std::atof(fn.c_str()));
		const std::string ff = Attr(tag, "fadeFar");
		if (!ff.empty()) n.fadeFar = static_cast<float>(std::atof(ff.c_str()));
		const std::string ts = Attr(tag, "trailScale");
		if (!ts.empty()) n.trailScale = static_cast<float>(std::atof(ts.c_str()));
		const std::string is = Attr(tag, "iconSize");
		if (!is.empty()) n.iconSize = static_cast<float>(std::atof(is.c_str()));
		const std::string al = Attr(tag, "alpha");
		if (!al.empty()) n.alpha = static_cast<float>(std::atof(al.c_str()));
		const std::string col = Attr(tag, "color");
		if (!col.empty())
			n.color = static_cast<uint32_t>(std::strtoul(col.c_str(), nullptr, 16));
		n.schedule = Attr(tag, "schedule");
		const std::string sd = Attr(tag, "schedule-duration");
		if (!sd.empty()) n.scheduleDuration = static_cast<float>(std::atof(sd.c_str()));
	}

	bool ParseCategoryTree(const std::string& xml, size_t& pos, CategoryNode& out)
	{
		std::string tag;
		bool selfClose = false;
		if (!ParseOpenTag(xml, pos, "MarkerCategory", tag, selfClose))
			return false;
		FillCategoryAttrs(out, tag);
		if (selfClose)
			return true;
		const std::string close = "</MarkerCategory>";
		while (true)
		{
			const size_t next = xml.find("<MarkerCategory", pos);
			const size_t end = xml.find(close, pos);
			if (end == std::string::npos)
				return false;
			if (next != std::string::npos && next < end)
			{
				CategoryNode ch;
				if (!ParseCategoryTree(xml, pos, ch))
					return false;
				out.children.push_back(std::move(ch));
				continue;
			}
			pos = end + close.size();
			return true;
		}
	}

	DraftPoi ParsePoiTag(const std::string& tag)
	{
		DraftPoi p;
		p.mapId = static_cast<uint32_t>(std::atoi(Attr(tag, "MapID").c_str()));
		p.x = static_cast<float>(std::atof(Attr(tag, "xpos").c_str()));
		p.y = static_cast<float>(std::atof(Attr(tag, "ypos").c_str()));
		p.z = static_cast<float>(std::atof(Attr(tag, "zpos").c_str()));
		p.type = Attr(tag, "type");
		p.guid = Attr(tag, "GUID");
		if (p.guid.empty())
			p.guid = Attr(tag, "guid");
		p.behavior = std::atoi(Attr(tag, "behavior").c_str());
		p.autoTrigger = Attr(tag, "autoTrigger") == "1" || Attr(tag, "autoTrigger") == "true";
		const std::string tr = Attr(tag, "triggerRange");
		if (!tr.empty())
			p.triggerRange = static_cast<float>(std::atof(tr.c_str()));
		p.tipName = Attr(tag, "tip-name");
		p.tipDescription = Attr(tag, "tip-description");
		p.info = Attr(tag, "info");
		p.copy = Attr(tag, "copy");
		p.copyMessage = Attr(tag, "copy-message");
		p.schedule = Attr(tag, "schedule");
		const std::string sd = Attr(tag, "schedule-duration");
		if (!sd.empty())
			p.scheduleDuration = static_cast<float>(std::atof(sd.c_str()));
		p.iconFile = Attr(tag, "iconFile");
		p.scriptOnce = Attr(tag, "script-once");
		if (p.scriptOnce.empty()) p.scriptOnce = Attr(tag, "scriptOnce");
		p.scriptTrigger = Attr(tag, "script-trigger");
		p.scriptFilter = Attr(tag, "script-filter");
		p.scriptTick = Attr(tag, "script-tick");
		p.scriptFocus = Attr(tag, "script-focus");
		p.hide = Attr(tag, "hide");
		p.show = Attr(tag, "show");
		{
			const std::string v = Attr(tag, "fadeNear");
			if (!v.empty()) p.fadeNear = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "fadeFar");
			if (!v.empty()) p.fadeFar = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "alpha");
			if (!v.empty()) p.alpha = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "iconSize");
			if (!v.empty()) p.iconSize = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "heightOffset");
			if (!v.empty()) p.heightOffset = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "mapDisplaySize");
			if (!v.empty()) p.mapDisplaySize = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "minSize");
			if (!v.empty()) p.minSize = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "maxSize");
			if (!v.empty()) p.maxSize = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "resetLength");
			if (!v.empty()) p.resetLength = static_cast<float>(std::atof(v.c_str()));
		}
		p.invertBehavior = Attr(tag, "invertBehavior") == "1" || Attr(tag, "invertBehavior") == "true";
		p.minimapVisible = !(Attr(tag, "miniMapVisibility") == "0");
		p.inGameVisible = !(Attr(tag, "inGameVisibility") == "0");
		{
			const std::string v = Attr(tag, "achievementId");
			if (!v.empty()) p.achievementId = std::atoi(v.c_str());
		}
		{
			const std::string v = Attr(tag, "achievementBit");
			if (!v.empty()) p.achievementBit = std::atoi(v.c_str());
		}
		p.festival = Attr(tag, "festival");
		{
			const std::string v = Attr(tag, "profession");
			if (!v.empty()) p.profession = std::atoi(v.c_str());
		}
		{
			const std::string v = Attr(tag, "race");
			if (!v.empty()) p.race = std::atoi(v.c_str());
		}
		{
			const std::string v = Attr(tag, "mount");
			if (!v.empty()) p.mount = std::atoi(v.c_str());
		}
		p.toggleCategory = Attr(tag, "toggleCategory");
		return p;
	}

	void ApplyTrailTagAttrs(const std::string& tag, DraftTrail& t)
	{
		t.texture = Attr(tag, "texture");
		{
			const std::string v = Attr(tag, "animSpeed");
			if (!v.empty()) t.animSpeed = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "alpha");
			if (!v.empty()) t.alpha = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "fadeNear");
			if (!v.empty()) t.fadeNear = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "fadeFar");
			if (!v.empty()) t.fadeFar = static_cast<float>(std::atof(v.c_str()));
		}
		{
			const std::string v = Attr(tag, "trailScale");
			if (!v.empty()) t.trailScale = static_cast<float>(std::atof(v.c_str()));
		}
	}
}

bool TrailToolsDetail::SaveDraftSession()
{
	EnsureWorkspace();
	std::string xml = TrailToolsXml::EmitOverlayData(gDraft);
	xml += "<!-- ACTIVE map=\"";
	xml += std::to_string(gDraft.active.mapId);
	xml += "\" file=\"";
	xml += gDraft.active.fileRel;
	xml += "\" type=\"";
	xml += gDraft.active.type;
	xml += "\" pts=\"";
	xml += std::to_string(gDraft.active.points.size());
	xml += "\" markerType=\"";
	xml += gDraft.markerType;
	xml += "\" trailType=\"";
	xml += gDraft.trailType;
	xml += "\" -->\n";
	if (!WriteAll(SessionPath(), xml))
	{
		SetStatus("Failed to save draft session.");
		return false;
	}
	SetStatus("Draft session saved.");
	return true;
}

bool TrailToolsDetail::LoadDraftSession()
{
	std::string xml;
	if (!ReadAll(SessionPath(), xml))
	{
		SetStatus("No draft session on disk.");
		return false;
	}
	return ApplyOverlayXml(xml);
}

bool TrailToolsDetail::ApplyOverlayXml(const std::string& xmlIn)
{
	std::string xml = xmlIn;
	if (xml.size() >= 3 &&
		static_cast<unsigned char>(xml[0]) == 0xEF &&
		static_cast<unsigned char>(xml[1]) == 0xBB &&
		static_cast<unsigned char>(xml[2]) == 0xBF)
		xml.erase(0, 3);

	size_t pos = xml.find("<OverlayData");
	if (pos != std::string::npos)
	{
		pos = xml.find('>', pos);
		if (pos != std::string::npos)
		{
			++pos;
			CategoryNode root;
			size_t catPos = pos;
			if (ParseCategoryTree(xml, catPos, root) && !root.name.empty())
				gDraft.root = std::move(root);
		}
	}

	gDraft.pois.clear();
	pos = 0;
	std::string tag;
	while (NextElem(xml, "POI", pos, tag))
	{
		DraftPoi p = ParsePoiTag(tag);
		if (p.mapId && !p.type.empty())
			gDraft.pois.push_back(std::move(p));
	}

	gDraft.trails.clear();
	pos = 0;
	while (NextElem(xml, "Trail", pos, tag))
	{
		DraftTrail t;
		t.type = Attr(tag, "type");
		t.fileRel = Attr(tag, "trailData");
		if (t.fileRel.empty())
			t.fileRel = Attr(tag, "trailFile");
		ApplyTrailTagAttrs(tag, t);
		if (!t.type.empty() && !t.fileRel.empty())
		{
			uint32_t mid = 0;
			std::vector<PathingTrails::WorldPoint> pts;
			if (TrailToolsTrl::Read(RelToPackPath(t.fileRel), mid, pts))
			{
				t.mapId = mid;
				t.points = std::move(pts);
			}
			gDraft.trails.push_back(std::move(t));
		}
	}

	const size_t act = xml.find("<!-- ACTIVE ");
	if (act != std::string::npos)
	{
		const size_t end = xml.find("-->", act);
		const std::string atag = end == std::string::npos
			? xml.substr(act)
			: xml.substr(act, end - act);
		gDraft.active = {};
		gDraft.active.mapId = static_cast<uint32_t>(std::atoi(Attr(atag, "map").c_str()));
		gDraft.active.fileRel = Attr(atag, "file");
		gDraft.active.type = Attr(atag, "type");
		const std::string mt = Attr(atag, "markerType");
		if (!mt.empty())
			std::snprintf(gDraft.markerType, sizeof(gDraft.markerType), "%s", mt.c_str());
		const std::string tt = Attr(atag, "trailType");
		if (!tt.empty())
			std::snprintf(gDraft.trailType, sizeof(gDraft.trailType), "%s", tt.c_str());
		if (!gDraft.active.fileRel.empty())
		{
			uint32_t mid = 0;
			std::vector<PathingTrails::WorldPoint> pts;
			if (TrailToolsTrl::Read(RelToPackPath(gDraft.active.fileRel), mid, pts))
			{
				gDraft.active.mapId = mid;
				gDraft.active.points = std::move(pts);
			}
			else
			{
				for (const auto& t : gDraft.trails)
				{
					if (t.fileRel == gDraft.active.fileRel)
					{
						gDraft.active = t;
						break;
					}
				}
			}
		}
	}
	else if (!gDraft.trails.empty())
	{
		gDraft.active = gDraft.trails.front();
		gDraft.selectedTrail = 0;
	}

	if (!gDraft.markerType[0] && !gDraft.root.name.empty())
		std::snprintf(gDraft.markerType, sizeof(gDraft.markerType), "%s.circle",
			gDraft.root.name.c_str());
	if (!gDraft.trailType[0] && !gDraft.root.name.empty())
		std::snprintf(gDraft.trailType, sizeof(gDraft.trailType), "%s.example",
			gDraft.root.name.c_str());

	SetStatus("Loaded draft (%zu POIs, %zu trails, %zu active pts).",
		gDraft.pois.size(), gDraft.trails.size(), gDraft.active.points.size());
	return true;
}

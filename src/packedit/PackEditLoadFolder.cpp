#include "PackEditInternal.h"

#include "PathingParse.h"

#include <cstdio>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>

namespace
{
	std::string EntryUtf8(const std::vector<uint8_t>& b)
	{
		return std::string(reinterpret_cast<const char*>(b.data()), b.size());
	}

	std::string WideToUtf8(const wchar_t* w)
	{
		char buf[520]{};
		WideCharToMultiByte(CP_UTF8, 0, w, -1, buf, 520, nullptr, nullptr);
		return buf;
	}

	void WalkDir(const std::wstring& dir, const std::string& rel, int depth,
		const std::function<void(const std::wstring&, const std::string&)>& add)
	{
		if (depth > 8)
			return;
		WIN32_FIND_DATAW fd{};
		std::wstring glob = dir;
		if (!glob.empty() && glob.back() != L'\\' && glob.back() != L'/')
			glob.push_back(L'\\');
		glob += L"*";
		HANDLE h = FindFirstFileW(glob.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE)
			return;
		do
		{
			if (fd.cFileName[0] == L'.')
				continue;
			std::wstring full = dir + L"\\" + fd.cFileName;
			std::string name = WideToUtf8(fd.cFileName);
			std::string childRel = rel.empty() ? name : (rel + "/" + name);
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				WalkDir(full, childRel, depth + 1, add);
			else
				add(full, childRel);
		} while (FindNextFileW(h, &fd));
		FindClose(h);
	}
}

bool PackEdit::OpenFolder(const std::wstring& dir, std::string& err)
{
	WIN32_FIND_DATAW fd{};
	std::wstring probe = dir + L"\\*";
	HANDLE h = FindFirstFileW(probe.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
	{
		err = "Folder not readable.";
		return false;
	}
	FindClose(h);

	PeDoc doc{};
	doc.path = dir;
	doc.fromZip = false;
	doc.worldDraw = false;
	doc.gizmoOn = true;
	doc.thisMapOnly = true;
	std::vector<PathingParse::IndexedPoi> pois;
	std::vector<PathingParse::IndexedTrail> trails;
	std::unordered_map<std::string, PathingParse::MarkerStyle> styles;
	std::unordered_map<std::string, uint32_t> catMaps;
	std::vector<PathingTrails::Category> catRoots;

	WalkDir(dir, "", 0, [&](const std::wstring& full, const std::string& rel) {
		PeEntry e;
		e.name = rel;
		if (!PathingParse::ReadFileW(full, e.bytes, 8u * 1024u * 1024u))
			return;
		const std::string low = PathingParse::ToLower(e.name);
		if (low.size() >= 4 && low.compare(low.size() - 4, 4, ".xml") == 0)
		{
			const std::string xml = EntryUtf8(e.bytes);
			const size_t poi0 = pois.size();
			const size_t tr0 = trails.size();
			PathingParse::ParseMarkerMenuXml(xml, catRoots, styles);
			PathingParse::CollectCategoryMapIds(xml, catMaps);
			PathingParse::IndexPoisXml(dir, xml, pois, catMaps);
			PathingParse::IndexXml(dir, xml, trails);
			for (size_t k = poi0; k < pois.size(); ++k)
				pois[k].xmlEntry = e.name;
			for (size_t k = tr0; k < trails.size(); ++k)
				trails[k].xmlEntry = e.name;
		}
		doc.entries.push_back(std::move(e));
	});

	ApplyParsed(doc, catRoots, pois, trails, std::move(styles));
	gDoc = std::move(doc);
	std::snprintf(gDoc.status, sizeof(gDoc.status), "Opened folder (%zu items).", gDoc.items.size());
	return true;
}

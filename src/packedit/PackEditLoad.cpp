#include "PackEditInternal.h"

#include "PathingParse.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>

#include "miniz/miniz.h"

namespace
{
	std::string EntryUtf8(const std::vector<uint8_t>& b)
	{
		return std::string(reinterpret_cast<const char*>(b.data()), b.size());
	}
}

bool PackEdit::OpenZip(const std::wstring& path, std::string& err)
{
	std::vector<uint8_t> blob;
	if (!PathingParse::ReadFileW(path, blob, PathingParse::kMaxZipBytes))
	{
		err = "Could not read pack file.";
		return false;
	}
	mz_zip_archive zip{};
	if (!mz_zip_reader_init_mem(&zip, blob.data(), blob.size(), 0))
	{
		err = "Not a zip/.taco archive.";
		return false;
	}

	PeDoc doc{};
	doc.path = path;
	doc.fromZip = true;
	doc.worldDraw = true;
	doc.gizmoOn = true;
	doc.thisMapOnly = true;
	const int n = static_cast<int>(mz_zip_reader_get_num_files(&zip));
	std::vector<PathingParse::IndexedPoi> pois;
	std::vector<PathingParse::IndexedTrail> trails;
	std::unordered_map<std::string, PathingParse::MarkerStyle> styles;
	std::unordered_map<std::string, uint32_t> catMaps;
	std::vector<PathingTrails::Category> catRoots;

	for (int i = 0; i < n; ++i)
	{
		mz_zip_archive_file_stat st{};
		if (!mz_zip_reader_file_stat(&zip, static_cast<mz_uint>(i), &st) || st.m_is_directory)
			continue;
		PeEntry e;
		e.name = st.m_filename;
		std::replace(e.name.begin(), e.name.end(), '\\', '/');
		if (!PathingParse::ZipExtractIndex(zip, i, e.bytes, 16u * 1024u * 1024u))
			continue;
		const std::string low = PathingParse::ToLower(e.name);
		if (low.size() >= 4 && low.compare(low.size() - 4, 4, ".xml") == 0)
		{
			const std::string xml = EntryUtf8(e.bytes);
			const size_t poi0 = pois.size();
			const size_t tr0 = trails.size();
			PathingParse::ParseMarkerMenuXml(xml, catRoots, styles);
			PathingParse::CollectCategoryMapIds(xml, catMaps);
			PathingParse::IndexPoisXml(path, xml, pois, catMaps);
			PathingParse::IndexXml(path, xml, trails);
			for (size_t k = poi0; k < pois.size(); ++k)
				pois[k].xmlEntry = e.name;
			for (size_t k = tr0; k < trails.size(); ++k)
				trails[k].xmlEntry = e.name;
		}
		doc.entries.push_back(std::move(e));
	}
	mz_zip_reader_end(&zip);

	ApplyParsed(doc, catRoots, pois, trails, std::move(styles));
	gDoc = std::move(doc);
	std::snprintf(gDoc.status, sizeof(gDoc.status),
		"Opened %zu items (%zu xml/bin entries).", gDoc.items.size(), gDoc.entries.size());
	return true;
}

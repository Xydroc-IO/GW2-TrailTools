#include "PathingIndex.h"

#include "AddonPaths.h"
#include "Globals.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>
#include "miniz/miniz.h"

namespace PathingDetail
{
	void MergeCategoryTree(std::vector<PathingTrails::Category>& dest, PathingTrails::Category&& src)
	{
		PathingTrails::Category* found = nullptr;
		for (PathingTrails::Category& c : dest)
		{
			if (c.path == src.path)
			{
				found = &c;
				break;
			}
		}
		if (!found)
		{
			dest.push_back(std::move(src));
			return;
		}
		if (!src.label.empty())
			found->label = std::move(src.label);
		if (!src.tip.empty())
			found->tip = std::move(src.tip);
		if (!src.separator)
			found->separator = false;
		for (PathingTrails::Category& ch : src.children)
			MergeCategoryTree(found->children, std::move(ch));
		src.children.clear();
	}

	void ListTacoFiles(const std::wstring& dir, std::vector<std::wstring>& out)
	{
		const std::wstring pattern = dir + L"\\*.taco";
		WIN32_FIND_DATAW fd{};
		HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE)
			return;
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			out.push_back(dir + L"\\" + fd.cFileName);
		} while (FindNextFileW(h, &fd));
		FindClose(h);
	}

	void IndexPack(const std::wstring& packPath, std::vector<IndexedTrail>& out,
		std::vector<IndexedPoi>& poisOut, std::vector<PathingTrails::Category>& menuOut,
		std::unordered_map<std::string, MarkerStyle>& stylesOut, uint32_t epoch)
	{
		std::vector<uint8_t> file;
		if (!ReadFileW(packPath, file, kMaxZipBytes))
			return;

		mz_zip_archive zip{};
		if (!mz_zip_reader_init_mem(&zip, file.data(), file.size(), 0))
			return;

		const int n = static_cast<int>(mz_zip_reader_get_num_files(&zip));
		std::unordered_map<std::string, uint32_t> categoryMapIds;
		for (int i = 0; i < n; ++i)
		{
			if (gEpoch.load(std::memory_order_acquire) != epoch)
				break;
			mz_zip_archive_file_stat st{};
			if (!mz_zip_reader_file_stat(&zip, static_cast<mz_uint>(i), &st) || st.m_is_directory)
				continue;
			std::string name(st.m_filename);
			std::replace(name.begin(), name.end(), '\\', '/');
			const std::string low = ToLower(name);
			if (low.size() < 5 || low.compare(low.size() - 4, 4, ".xml") != 0)
				continue;
			if (st.m_uncomp_size == 0 || st.m_uncomp_size > 8u * 1024u * 1024u)
				continue;

			size_t sz = 0;
			void* mem = mz_zip_reader_extract_to_heap(&zip, static_cast<mz_uint>(i), &sz, 0);
			if (!mem || sz == 0)
				continue;
			std::string xml(static_cast<char*>(mem), sz);
			mz_free(mem);
			CollectCategoryMapIds(xml, categoryMapIds);
			IndexXml(packPath, xml, out);
			IndexPoisXml(packPath, xml, poisOut, categoryMapIds);

			if (ToLower(xml).find("<markercategory") != std::string::npos)
			{
				std::vector<PathingTrails::Category> parsed;
				ParseMarkerMenuXml(xml, parsed, stylesOut);
				for (PathingTrails::Category& root : parsed)
					MergeCategoryTree(menuOut, std::move(root));
			}
		}
		mz_zip_reader_end(&zip);
	}

	static std::string WideLeafUtf8(const std::wstring& path)
	{
		const size_t slash = path.find_last_of(L"\\/");
		const std::wstring leaf = (slash == std::wstring::npos) ? path : path.substr(slash + 1);
		std::string out;
		out.reserve(leaf.size());
		for (wchar_t c : leaf)
		{
			if (c < 128)
				out.push_back(static_cast<char>(c));
		}
		return out;
	}

	void WorkerIndexAndLoad(uint32_t epoch, uint32_t mapId)
	{
		std::vector<std::wstring> packs;
		const std::wstring pathing = AddonPaths::EnsureUnder(AddonPaths::DataDir(), L"pathing");
		ListTacoFiles(pathing, packs);

		std::vector<IndexedTrail> index;
		std::vector<IndexedPoi> pois;
		std::vector<PathingTrails::Category> menu;
		std::unordered_map<std::string, MarkerStyle> styles;
		std::vector<std::string> names;
		index.reserve(4096);
		pois.reserve(8192);

		for (const std::wstring& p : packs)
		{
			if (gEpoch.load(std::memory_order_acquire) != epoch)
				return;
			IndexPack(p, index, pois, menu, styles, epoch);
			names.push_back(WideLeafUtf8(p));
		}

		{
			std::lock_guard<std::mutex> lock(gMutex);
			if (gEpoch.load(std::memory_order_acquire) != epoch)
				return;
			gIndex = std::move(index);
			gPoiIndex = std::move(pois);
			gCategoryStyles = std::move(styles);
			gMenu = std::move(menu);
			MarkEnabled(gMenu);
			gPackNames = std::move(names);
			gPackCount.store(static_cast<int>(gPackNames.size()), std::memory_order_release);
			gMenuRevision.fetch_add(1, std::memory_order_release);
			gContentRevision.fetch_add(1, std::memory_order_release);
		}

		if (mapId != 0)
			LoadMapMarkers(mapId, epoch);
	}
}

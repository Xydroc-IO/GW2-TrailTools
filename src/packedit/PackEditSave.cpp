#include "PackEditInternal.h"

#include "PathingParse.h"

#include "miniz/miniz.h"

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>

namespace
{
	std::string Esc(const std::string& s)
	{
		std::string o;
		for (char c : s)
		{
			if (c == '&') o += "&amp;";
			else if (c == '"') o += "&quot;";
			else if (c == '<') o += "&lt;";
			else o.push_back(c);
		}
		return o;
	}

	void EmitCat(std::ostringstream& os, const PackEdit::PeCategory& n, int ind)
	{
		std::string pad(static_cast<size_t>(ind * 3), ' ');
		os << pad << "<MarkerCategory name=\"" << Esc(n.name)
			<< "\" DisplayName=\"" << Esc(n.display) << "\"";
		auto it = PackEdit::gDoc.catStyles.find(n.path);
		if (it != PackEdit::gDoc.catStyles.end())
			PackEdit::AppendStyleXml(os, it->second);
		if (n.children.empty())
			os << "/>\n";
		else
		{
			os << ">\n";
			for (const auto& ch : n.children)
				EmitCat(os, ch, ind + 1);
			os << pad << "</MarkerCategory>\n";
		}
	}

	std::string EmitOverlay(const PackEdit::PeDoc& d)
	{
		std::ostringstream os;
		os << "<OverlayData>\n";
		for (const auto& r : d.roots)
			EmitCat(os, r, 1);
		os << "   <POIs>\n";
		for (const auto& p : d.items)
		{
			if (p.tombstone)
				continue;
			if (p.isTrail)
			{
				if (p.type.empty() || p.trailData.empty())
					continue;
				os << "      <Trail type=\"" << Esc(p.type)
					<< "\" trailData=\"" << Esc(p.trailData) << "\"";
				PackEdit::AppendStyleXml(os, p.style);
				os << "/>\n";
			}
			else
			{
				os << "      <POI MapID=\"" << p.mapId
					<< "\" xpos=\"" << p.x << "\" ypos=\"" << p.y << "\" zpos=\"" << p.z
					<< "\" type=\"" << Esc(p.type) << "\"";
				if (!p.guid.empty())
					os << " GUID=\"" << Esc(p.guid) << "\"";
				if (p.rotate != 0.f)
					os << " rotate=\"" << p.rotate << "\"";
				PackEdit::AppendStyleXml(os, p.style);
				os << "/>\n";
			}
		}
		os << "   </POIs>\n</OverlayData>\n";
		return os.str();
	}

	std::vector<uint8_t> TrlBytes(uint32_t mapId,
		const std::vector<PathingTrails::WorldPoint>& pts)
	{
		std::vector<uint8_t> b(8 + pts.size() * 12);
		uint32_t ver = 0;
		std::memcpy(b.data(), &ver, 4);
		std::memcpy(b.data() + 4, &mapId, 4);
		size_t o = 8;
		for (const auto& p : pts)
		{
			float t[3] = { p.x, p.y, p.z };
			std::memcpy(b.data() + o, t, 12);
			o += 12;
		}
		return b;
	}

	bool WriteFileW(const std::wstring& path, const void* data, size_t n)
	{
		HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (h == INVALID_HANDLE_VALUE)
			return false;
		DWORD w = 0;
		const BOOL ok = WriteFile(h, data, static_cast<DWORD>(n), &w, nullptr);
		CloseHandle(h);
		return ok && w == n;
	}
}

bool PackEdit::SaveZip(const std::wstring& path, std::string& err)
{
	mz_zip_archive zip{};
	if (!mz_zip_writer_init_heap(&zip, 0, 256 * 1024))
	{
		err = "Zip writer init failed.";
		return false;
	}
	bool ok = true;
	int xmlN = 0;
	std::string hostXml;
	for (const auto& e : gDoc.entries)
	{
		const std::string low = PathingParse::ToLower(e.name);
		if (low.size() >= 4 && low.compare(low.size() - 4, 4, ".xml") == 0)
		{
			++xmlN;
			if (hostXml.empty())
				hostXml = e.name;
			if (PathingParse::ToLower(e.name) == "overlaydata.xml")
				hostXml = e.name;
		}
	}
	if (xmlN == 0)
	{
		const std::string overlay = EmitOverlay(gDoc);
		ok = mz_zip_writer_add_mem(&zip, "OverlayData.xml", overlay.data(), overlay.size(),
			MZ_DEFAULT_COMPRESSION) != 0;
	}
	else
	{
		for (auto& p : gDoc.items)
		{
			if (p.tombstone || !p.xmlFile.empty())
				continue;
			p.xmlFile = hostXml;
		}
		for (const auto& e : gDoc.entries)
		{
			const std::string low = PathingParse::ToLower(e.name);
			if (low.size() < 4 || low.compare(low.size() - 4, 4, ".xml") != 0)
				continue;
			std::string xml(reinterpret_cast<const char*>(e.bytes.data()), e.bytes.size());
			xml = PatchXmlFile(xml, e.name);
			ok = ok && mz_zip_writer_add_mem(&zip, e.name.c_str(), xml.data(), xml.size(),
				MZ_DEFAULT_COMPRESSION) != 0;
		}
	}

	for (const auto& p : gDoc.items)
	{
		if (p.tombstone || !p.isTrail || p.trailData.empty() || p.points.empty())
			continue;
		auto trl = TrlBytes(p.mapId ? p.mapId : 1, p.points);
		ok = ok && mz_zip_writer_add_mem(&zip, p.trailData.c_str(), trl.data(), trl.size(),
			MZ_DEFAULT_COMPRESSION) != 0;
	}

	for (const auto& e : gDoc.entries)
	{
		const std::string low = PathingParse::ToLower(e.name);
		if (low.size() >= 4 && low.compare(low.size() - 4, 4, ".xml") == 0)
			continue;
		if (low.size() >= 4 && low.compare(low.size() - 4, 4, ".trl") == 0)
		{
			bool replaced = false;
			for (const auto& p : gDoc.items)
			{
				if (p.tombstone || !p.isTrail || p.points.empty())
					continue;
				if (PathingParse::ToLower(p.trailData) == low)
				{
					replaced = true;
					break;
				}
			}
			if (replaced)
				continue;
		}
		ok = ok && mz_zip_writer_add_mem(&zip, e.name.c_str(), e.bytes.data(), e.bytes.size(),
			MZ_DEFAULT_COMPRESSION) != 0;
	}

	void* outBuf = nullptr;
	size_t outSize = 0;
	if (!ok || !mz_zip_writer_finalize_heap_archive(&zip, &outBuf, &outSize) || !outBuf)
	{
		mz_zip_writer_end(&zip);
		err = "Zip finalize failed.";
		return false;
	}
	mz_zip_writer_end(&zip);
	if (!WriteFileW(path, outBuf, outSize))
	{
		err = "Could not write .taco.";
		return false;
	}
	gDoc.path = path;
	gDoc.dirty = false;
	std::snprintf(gDoc.status, sizeof(gDoc.status),
		"Saved pack (kept original XML files; patched POI/Trail tags).");
	return true;
}

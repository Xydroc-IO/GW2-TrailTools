#include "PackEditInternal.h"

#include "PathingParse.h"

#include <cstdio>
#include <sstream>
#include <string>

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

	void SetAttr(std::string& tag, const char* key, const std::string& val)
	{
		std::string needle = std::string(key) + "=\"";
		size_t p = tag.find(needle);
		if (p == std::string::npos)
		{
			std::string low = PathingParse::ToLower(tag);
			std::string nl = PathingParse::ToLower(needle);
			p = low.find(nl);
			if (p == std::string::npos)
			{
				size_t ins = tag.rfind("/>");
				if (ins == std::string::npos)
					ins = tag.rfind('>');
				if (ins == std::string::npos)
					return;
				tag.insert(ins, std::string(" ") + key + "=\"" + val + "\"");
				return;
			}
		}
		p = tag.find('"', p);
		if (p == std::string::npos)
			return;
		const size_t e = tag.find('"', p + 1);
		if (e == std::string::npos)
			return;
		tag.replace(p + 1, e - (p + 1), val);
	}

	std::string UpdatedTag(const PackEdit::PePathable& p)
	{
		std::string tag = p.rawTag;
		if (tag.empty())
		{
			std::ostringstream os;
			if (p.isTrail)
			{
				os << "<Trail type=\"" << Esc(p.type)
					<< "\" trailData=\"" << Esc(p.trailData) << "\"";
				PackEdit::AppendStyleXml(os, p.style);
				os << "/>";
			}
			else
			{
				os << "<POI MapID=\"" << p.mapId
					<< "\" xpos=\"" << p.x << "\" ypos=\"" << p.y << "\" zpos=\"" << p.z
					<< "\" type=\"" << Esc(p.type) << "\"";
				if (!p.guid.empty())
					os << " GUID=\"" << Esc(p.guid) << "\"";
				if (p.rotate != 0.f)
					os << " rotate=\"" << p.rotate << "\"";
				PackEdit::AppendStyleXml(os, p.style);
				os << "/>";
			}
			return os.str();
		}
		if (p.isTrail)
		{
			SetAttr(tag, "type", Esc(p.type));
			SetAttr(tag, "trailData", Esc(p.trailData));
		}
		else
		{
			char buf[64]{};
			std::snprintf(buf, sizeof(buf), "%u", p.mapId);
			SetAttr(tag, "MapID", buf);
			std::snprintf(buf, sizeof(buf), "%.6g", p.x);
			SetAttr(tag, "xpos", buf);
			std::snprintf(buf, sizeof(buf), "%.6g", p.y);
			SetAttr(tag, "ypos", buf);
			std::snprintf(buf, sizeof(buf), "%.6g", p.z);
			SetAttr(tag, "zpos", buf);
			SetAttr(tag, "type", Esc(p.type));
			if (!p.guid.empty())
				SetAttr(tag, "GUID", Esc(p.guid));
			if (p.rotate != 0.f)
			{
				std::snprintf(buf, sizeof(buf), "%.4g", p.rotate);
				SetAttr(tag, "rotate", buf);
			}
		}
		return tag;
	}

	bool SameFile(const std::string& itemFile, const std::string& xmlName)
	{
		if (itemFile.empty())
			return false;
		return PathingParse::ToLower(itemFile) == PathingParse::ToLower(xmlName);
	}
}

std::string PackEdit::PatchXmlFile(const std::string& xml, const std::string& fileName)
{
	std::string out = xml;
	std::string append;
	for (auto& p : gDoc.items)
	{
		if (!SameFile(p.xmlFile, fileName))
			continue;

		if (p.tombstone)
		{
			if (!p.rawTag.empty())
			{
				size_t f = out.find(p.rawTag);
				if (f != std::string::npos)
					out.erase(f, p.rawTag.size());
			}
			continue;
		}

		const std::string neu = UpdatedTag(p);
		if (!p.rawTag.empty())
		{
			size_t f = out.find(p.rawTag);
			if (f != std::string::npos)
			{
				out.replace(f, p.rawTag.size(), neu);
				p.rawTag = neu;
				continue;
			}
			if (!p.guid.empty())
			{
				const std::string gneedle = "GUID=\"" + p.guid + "\"";
				f = out.find(gneedle);
				if (f != std::string::npos)
				{
					size_t a = out.rfind('<', f);
					size_t b = out.find('>', f);
					if (a != std::string::npos && b != std::string::npos)
					{
						out.replace(a, b - a + 1, neu);
						p.rawTag = neu;
						continue;
					}
				}
			}
		}
		append += "      " + neu + "\n";
		p.xmlFile = fileName;
		p.rawTag = neu;
	}

	if (append.empty())
		return out;
	size_t pois = out.find("</POIs>");
	if (pois == std::string::npos)
		pois = out.find("</pois>");
	if (pois != std::string::npos)
		out.insert(pois, append);
	else
		out += "\n<POIs>\n" + append + "</POIs>\n";
	return out;
}

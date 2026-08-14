#include "PackEditInternal.h"

#include "PathingParse.h"

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
	void ConvertCats(const std::vector<PathingTrails::Category>& in,
		std::vector<PackEdit::PeCategory>& out)
	{
		out.clear();
		out.reserve(in.size());
		for (const auto& c : in)
		{
			PackEdit::PeCategory n;
			n.path = c.path;
			n.display = c.label.empty() ? c.path : c.label;
			const size_t dot = c.path.rfind('.');
			n.name = (dot == std::string::npos) ? c.path : c.path.substr(dot + 1);
			n.hidden = c.hidden;
			ConvertCats(c.children, n.children);
			out.push_back(std::move(n));
		}
	}
}

void PackEdit::ApplyParsed(PeDoc& doc,
	const std::vector<PathingTrails::Category>& catRoots,
	std::vector<PathingParse::IndexedPoi>& pois,
	std::vector<PathingParse::IndexedTrail>& trails,
	std::unordered_map<std::string, PathingParse::MarkerStyle> styles)
{
	ConvertCats(catRoots, doc.roots);
	doc.catStyles = std::move(styles);
	doc.items.reserve(pois.size() + trails.size());
	for (const auto& p : pois)
	{
		PePathable it;
		it.isTrail = false;
		it.type = p.type;
		it.guid = p.guid;
		it.mapId = p.mapId;
		it.x = p.wx;
		it.y = p.wy;
		it.z = p.wz;
		it.style = p.style;
		it.xmlFile = p.xmlEntry;
		it.rawTag = p.tag;
		it.rotate = 0.f;
		{
			const std::string rs = PathingParse::Attr(p.tag, "rotate");
			if (!rs.empty())
				it.rotate = static_cast<float>(std::atof(rs.c_str()));
		}
		doc.items.push_back(std::move(it));
	}
	for (auto& t : trails)
	{
		PePathable it;
		it.isTrail = true;
		it.type = t.type;
		it.trailData = t.entryName;
		it.style = t.style;
		it.xmlFile = t.xmlEntry;
		it.rawTag = t.tag;
		it.mapId = t.mapId;
		for (const auto& e : doc.entries)
		{
			const std::string el = PathingParse::ToLower(e.name);
			const std::string tl = PathingParse::ToLower(t.entryName);
			if (el == tl || el.find(tl) != std::string::npos)
			{
				uint32_t mid = 0;
				PathingParse::ParseTrl(e.bytes, mid, it.points);
				if (mid)
					it.mapId = mid;
				break;
			}
		}
		doc.items.push_back(std::move(it));
	}
}

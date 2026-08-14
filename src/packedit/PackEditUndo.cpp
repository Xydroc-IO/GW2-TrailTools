#include "PackEditInternal.h"

#include <vector>

namespace
{
	constexpr int kMax = 32;
	struct Step
	{
		int index = -1;
		PackEdit::PePathable before{};
		PackEdit::PePathable after{};
	};
	Step gUndo[kMax]{};
	int gU = 0, gR = 0;
	Step gRedo[kMax]{};
}

void PackEdit::ClearHistory()
{
	gU = 0;
	gR = 0;
}

void PackEdit::PushUndo()
{
	PePathable* p = Selected();
	if (!p)
		return;
	Step s;
	s.index = gDoc.selItem;
	s.before = *p;
	s.after = *p;
	gUndo[gU % kMax] = s;
	++gU;
	gR = 0;
}

bool PackEdit::Undo()
{
	if (gU <= 0)
		return false;
	--gU;
	Step s = gUndo[gU % kMax];
	if (s.index < 0 || s.index >= static_cast<int>(gDoc.items.size()))
		return false;
	gRedo[gR % kMax] = s;
	++gR;
	gDoc.items[static_cast<size_t>(s.index)] = s.before;
	gDoc.selItem = s.index;
	gDoc.dirty = true;
	return true;
}

bool PackEdit::Redo()
{
	if (gR <= 0)
		return false;
	--gR;
	Step s = gRedo[gR % kMax];
	if (s.index < 0 || s.index >= static_cast<int>(gDoc.items.size()))
		return false;
	gDoc.items[static_cast<size_t>(s.index)] = s.after;
	gDoc.selItem = s.index;
	gDoc.dirty = true;
	return true;
}

void PackEdit::AddPoiAt(float x, float y, float z, uint32_t mapId)
{
	AddPoiAtFeet();
	PePathable* p = Selected();
	if (!p)
		return;
	p->x = x;
	p->y = y;
	p->z = z;
	if (mapId)
		p->mapId = mapId;
}

void PackEdit::AddPoiAtFeet()
{
	PePathable p;
	if (PePathable* src = Selected())
	{
		p.type = src->type;
		p.style = src->style;
		p.mapId = src->mapId ? src->mapId : 1;
	}
	else
	{
		p.type = gDoc.roots.empty() ? "mypack" : gDoc.roots[0].path;
		p.mapId = 1;
	}
	gDoc.items.push_back(std::move(p));
	RevealItem(static_cast<int>(gDoc.items.size()) - 1);
	gDoc.dirty = true;
}

void PackEdit::AddTrailEmpty()
{
	PePathable p;
	p.isTrail = true;
	p.type = gDoc.roots.empty() ? "mypack" : gDoc.roots[0].path;
	p.trailData = "Data/new.trl";
	gDoc.items.push_back(std::move(p));
	gDoc.selItem = static_cast<int>(gDoc.items.size()) - 1;
	gDoc.dirty = true;
}

void PackEdit::AddCategory()
{
	PeCategory c;
	c.name = "newcat";
	c.display = "New category";
	if (gDoc.roots.empty())
		c.path = "newcat";
	else
	{
		c.path = gDoc.roots[0].path + ".newcat";
		gDoc.roots[0].children.push_back(std::move(c));
		gDoc.dirty = true;
		return;
	}
	gDoc.roots.push_back(std::move(c));
	gDoc.dirty = true;
}

void PackEdit::TombstoneSelected()
{
	PePathable* p = Selected();
	if (!p)
		return;
	PushUndo();
	p->tombstone = true;
	gDoc.dirty = true;
}

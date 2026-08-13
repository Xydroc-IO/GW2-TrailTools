#include "PackEditInternal.h"

#include <cstdio>

namespace PackEdit
{
	PeDoc gDoc{};

	PePathable* Selected()
	{
		if (gDoc.selItem < 0 || gDoc.selItem >= static_cast<int>(gDoc.items.size()))
			return nullptr;
		PePathable& p = gDoc.items[static_cast<size_t>(gDoc.selItem)];
		return p.tombstone ? nullptr : &p;
	}

	bool CategoryHidden(const std::string& typePath)
	{
		if (typePath.empty())
			return false;
		std::string cur;
		size_t i = 0;
		while (i <= typePath.size())
		{
			if (i == typePath.size() || typePath[i] == '.')
			{
				if (!cur.empty() && gDoc.hidden.count(cur))
					return true;
				if (i == typePath.size())
					break;
			}
			if (i < typePath.size())
				cur.push_back(typePath[i]);
			++i;
		}
		return false;
	}

	void RevealItem(int index)
	{
		gDoc.selItem = index;
		gDoc.selItems.clear();
		if (index >= 0)
			gDoc.selItems.push_back(index);
	}

	void ToggleHidden(const std::string& path)
	{
		if (gDoc.hidden.count(path))
			gDoc.hidden.erase(path);
		else
			gDoc.hidden.insert(path);
	}

	void NewEmpty()
	{
		gDoc = {};
		ClearHistory();
		gDoc.fromZip = true;
		PeCategory root;
		root.name = "mypack";
		root.display = "My Pack";
		root.path = "mypack";
		gDoc.roots.push_back(std::move(root));
		std::snprintf(gDoc.status, sizeof(gDoc.status), "New empty pack — Save As to write a .taco.");
	}

	void ClosePack()
	{
		gDoc = {};
		ClearHistory();
		std::snprintf(gDoc.status, sizeof(gDoc.status),
			"Closed pack. Nothing in the editor (disk files were not deleted).");
	}
}

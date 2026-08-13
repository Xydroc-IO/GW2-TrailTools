#include "PackEditInternal.h"

#include <cstddef>
#include <cstdio>
#include <sstream>
#include <string>

namespace PackEdit
{
	std::string XmlEsc(const std::string& s)
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

	void AppendStyleXml(std::ostringstream& os, const PathingParse::MarkerStyle& s)
	{
		if (s.hasIconFile && !s.iconFile.empty())
			os << " iconFile=\"" << XmlEsc(s.iconFile) << "\"";
		if (s.hasTexture && !s.texture.empty())
			os << " texture=\"" << XmlEsc(s.texture) << "\"";
		if (s.hasIconSize)
			os << " iconSize=\"" << s.iconSize << "\"";
		if (s.hasTrailScale)
			os << " trailScale=\"" << s.trailScale << "\"";
		if (s.hasAlpha)
			os << " alpha=\"" << s.alpha << "\"";
		if (s.hasFadeNear)
			os << " fadeNear=\"" << s.fadeNear << "\"";
		if (s.hasFadeFar)
			os << " fadeFar=\"" << s.fadeFar << "\"";
		if (s.hasHeightOffset)
			os << " heightOffset=\"" << s.heightOffset << "\"";
		if (s.hasMapDisplaySize)
			os << " mapDisplaySize=\"" << s.mapDisplaySize << "\"";
		if (s.hasMinSize)
			os << " minSize=\"" << s.minSize << "\"";
		if (s.hasMaxSize)
			os << " maxSize=\"" << s.maxSize << "\"";
		if (s.hasColor)
		{
			char hex[16]{};
			std::snprintf(hex, sizeof(hex), "%08X", s.color);
			os << " color=\"" << hex << "\"";
		}
		if (s.hasBehavior)
			os << " behavior=\"" << s.behavior << "\"";
		if (s.hasAutoTrigger)
			os << " autoTrigger=\"" << (s.autoTrigger ? "1" : "0") << "\"";
		if (s.hasTriggerRange)
			os << " triggerRange=\"" << s.triggerRange << "\"";
		if (s.hasInGameVisible)
			os << " inGameVisibility=\"" << (s.inGameVisible ? "1" : "0") << "\"";
		if (s.hasMinimapVisible)
			os << " minimapVisibility=\"" << (s.minimapVisible ? "1" : "0") << "\"";
		if (s.hasMapVisible)
			os << " mapVisibility=\"" << (s.mapVisible ? "1" : "0") << "\"";
		if (s.hasResetLength)
			os << " resetLength=\"" << s.resetLength << "\"";
		if (s.hasInvertBehavior)
			os << " invertBehavior=\"" << (s.invertBehavior ? "1" : "0") << "\"";
		if (s.hasHide && !s.hide.empty())
			os << " hide=\"" << XmlEsc(s.hide) << "\"";
		if (s.hasShow && !s.show.empty())
			os << " show=\"" << XmlEsc(s.show) << "\"";
		if (s.hasTipName && !s.tipName.empty())
			os << " tip-name=\"" << XmlEsc(s.tipName) << "\"";
		if (s.hasTipDescription && !s.tipDescription.empty())
			os << " tip-description=\"" << XmlEsc(s.tipDescription) << "\"";
		if (s.hasInfo && !s.info.empty())
			os << " info=\"" << XmlEsc(s.info) << "\"";
		if (s.hasCopy && !s.copy.empty())
			os << " copy=\"" << XmlEsc(s.copy) << "\"";
		if (s.hasCopyMessage && !s.copyMessage.empty())
			os << " copy-message=\"" << XmlEsc(s.copyMessage) << "\"";
		if (s.hasSchedule && !s.schedule.empty())
			os << " schedule=\"" << XmlEsc(s.schedule) << "\"";
		if (s.hasScheduleDuration)
			os << " schedule-duration=\"" << s.scheduleDuration << "\"";
		if (s.hasScriptOnce && !s.scriptOnce.empty())
			os << " script-once=\"" << XmlEsc(s.scriptOnce) << "\"";
		if (s.hasScriptTrigger && !s.scriptTrigger.empty())
			os << " script-trigger=\"" << XmlEsc(s.scriptTrigger) << "\"";
		if (s.hasScriptFilter && !s.scriptFilter.empty())
			os << " script-filter=\"" << XmlEsc(s.scriptFilter) << "\"";
		if (s.hasScriptTick && !s.scriptTick.empty())
			os << " script-tick=\"" << XmlEsc(s.scriptTick) << "\"";
		if (s.hasScriptFocus && !s.scriptFocus.empty())
			os << " script-focus=\"" << XmlEsc(s.scriptFocus) << "\"";
	}

	PathingParse::MarkerStyle EffectiveStyle(const PePathable& p)
	{
		return PathingParse::ResolveStyle(p.type, p.style, gDoc.catStyles);
	}

	void SelectToggle(int index)
	{
		if (index < 0)
			return;
		for (size_t i = 0; i < gDoc.selItems.size(); ++i)
		{
			if (gDoc.selItems[i] == index)
			{
				gDoc.selItems.erase(gDoc.selItems.begin() + static_cast<std::ptrdiff_t>(i));
				if (gDoc.selItem == index)
					gDoc.selItem = gDoc.selItems.empty() ? -1 : gDoc.selItems.back();
				return;
			}
		}
		gDoc.selItems.push_back(index);
		gDoc.selItem = index;
	}

	bool IsSelected(int index)
	{
		for (int s : gDoc.selItems)
		{
			if (s == index)
				return true;
		}
		return gDoc.selItem == index;
	}

	int LintIssues()
	{
		int n = 0;
		for (const auto& p : gDoc.items)
		{
			if (p.tombstone)
				continue;
			if (p.type.empty())
				++n;
			if (!p.isTrail && p.mapId == 0)
				++n;
			if (p.isTrail && p.trailData.empty())
				++n;
		}
		return n;
	}

	void DuplicateSelected()
	{
		PePathable* p = Selected();
		if (!p)
			return;
		PePathable c = *p;
		c.guid.clear();
		c.x += 1.f;
		gDoc.items.push_back(std::move(c));
		RevealItem(static_cast<int>(gDoc.items.size()) - 1);
		gDoc.dirty = true;
	}
}

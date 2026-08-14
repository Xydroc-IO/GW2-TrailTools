#include "PackEditInternal.h"

#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <unordered_map>
#include <vector>

namespace
{
	void EnsureIndex(std::unordered_map<std::string, std::vector<int>>& byType, size_t& gen)
	{
		if (gen == PackEdit::gDoc.items.size())
			return;
		byType.clear();
		byType.reserve(256);
		for (int i = 0; i < static_cast<int>(PackEdit::gDoc.items.size()); ++i)
		{
			const auto& it = PackEdit::gDoc.items[static_cast<size_t>(i)];
			if (it.tombstone)
				continue;
			byType[it.type].push_back(i);
		}
		gen = PackEdit::gDoc.items.size();
	}

	void DrawCat(PackEdit::PeCategory& n,
		const std::unordered_map<std::string, std::vector<int>>& byType)
	{
		ImGui::PushID(n.path.c_str());
		const bool hid = PackEdit::gDoc.hidden.count(n.path) > 0;
		if (ImGui::SmallButton(hid ? "Show" : "Hide"))
			PackEdit::ToggleHidden(n.path);
		ImGui::SameLine();
		const bool open = ImGui::TreeNodeEx(n.display.empty() ? n.name.c_str() : n.display.c_str(), 0);
		if (open)
		{
			for (auto& ch : n.children)
				DrawCat(ch, byType);
			auto it = byType.find(n.path);
			if (it != byType.end())
			{
				int shown = 0;
				for (int i : it->second)
				{
					if (shown >= 40)
						break;
					const auto& item = PackEdit::gDoc.items[static_cast<size_t>(i)];
					ImGui::PushID(i);
					char lab[160]{};
					std::snprintf(lab, sizeof(lab), "%s %s map %u",
						item.isTrail ? "Trail" : "POI",
						item.guid.empty() ? item.trailData.c_str() : item.guid.c_str(), item.mapId);
					if (ImGui::Selectable(lab, PackEdit::IsSelected(i)))
					{
						if (ImGui::GetIO().KeyCtrl)
							PackEdit::SelectToggle(i);
						else
							PackEdit::RevealItem(i);
					}
					++shown;
					ImGui::PopID();
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void PackEdit::DrawTree()
{
	using namespace PackEdit;
	static std::unordered_map<std::string, std::vector<int>> sByType;
	static size_t sGen = static_cast<size_t>(-1);
	EnsureIndex(sByType, sGen);

	ImGui::TextDisabled("Pack tree — expand a category (not all open). Ctrl+click multi-select.");
	if (ImGui::BeginChild("###pe_tree", ImVec2(0.f, 220.f), true, PadNav::kNestedList))
	{
		ImGui::PushTextWrapPos(0.f);
		if (gDoc.roots.empty() && gDoc.items.empty())
			ImGui::TextDisabled("Open a .taco or folder.");
		for (auto& r : gDoc.roots)
			DrawCat(r, sByType);
		ImGui::Separator();
		ImGui::TextDisabled("All items (%zu) — virtual list", gDoc.items.size());
		const int n = static_cast<int>(gDoc.items.size());
		ImGuiListClipper clip;
		clip.Begin(n);
		while (clip.Step())
		{
			for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i)
			{
				const auto& it = gDoc.items[static_cast<size_t>(i)];
				if (it.tombstone)
				{
					ImGui::Dummy(ImVec2(1.f, ImGui::GetTextLineHeightWithSpacing()));
					continue;
				}
				ImGui::PushID(i + 100000);
				char lab[180]{};
				std::snprintf(lab, sizeof(lab), "%s  %s  map %u",
					it.isTrail ? "T" : "M", it.type.c_str(), it.mapId);
				if (ImGui::Selectable(lab, IsSelected(i)))
				{
					if (ImGui::GetIO().KeyCtrl)
						SelectToggle(i);
					else
						RevealItem(i);
				}
				ImGui::PopID();
			}
		}
		ImGui::PopTextWrapPos();
	}
	ImGui::EndChild();
}

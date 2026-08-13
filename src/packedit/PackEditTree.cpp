#include "PackEditInternal.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>

namespace
{
	void DrawCat(PackEdit::PeCategory& n)
	{
		ImGui::PushID(n.path.c_str());
		const bool hid = PackEdit::gDoc.hidden.count(n.path) > 0;
		if (ImGui::SmallButton(hid ? "Show" : "Hide"))
			PackEdit::ToggleHidden(n.path);
		ImGui::SameLine();
		const bool open = ImGui::TreeNodeEx(n.display.empty() ? n.name.c_str() : n.display.c_str(),
			ImGuiTreeNodeFlags_DefaultOpen);
		if (open)
		{
			for (auto& ch : n.children)
				DrawCat(ch);
			int shown = 0;
			for (int i = 0; i < static_cast<int>(PackEdit::gDoc.items.size()) && shown < 80; ++i)
			{
				const auto& it = PackEdit::gDoc.items[static_cast<size_t>(i)];
				if (it.tombstone || it.type != n.path)
					continue;
				ImGui::PushID(i);
				char lab[160]{};
				std::snprintf(lab, sizeof(lab), "%s %s map %u",
					it.isTrail ? "Trail" : "POI",
					it.guid.empty() ? it.trailData.c_str() : it.guid.c_str(), it.mapId);
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
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void PackEdit::DrawTree()
{
	using namespace PackEdit;
		ImGui::TextDisabled("Pack tree — Hide toggles world draw. Ctrl+click multi-select.");
	if (ImGui::BeginChild("###pe_tree", ImVec2(0.f, 220.f), true))
	{
		if (gDoc.roots.empty() && gDoc.items.empty())
			ImGui::TextDisabled("Open a .taco or folder.");
		for (auto& r : gDoc.roots)
			DrawCat(r);
		ImGui::Separator();
		ImGui::TextDisabled("All items (%zu)", gDoc.items.size());
		int shown = 0;
		for (int i = 0; i < static_cast<int>(gDoc.items.size()) && shown < 120; ++i)
		{
			const auto& it = gDoc.items[static_cast<size_t>(i)];
			if (it.tombstone)
				continue;
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
			++shown;
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

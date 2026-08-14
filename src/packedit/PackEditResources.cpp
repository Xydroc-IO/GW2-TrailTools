#include "PackEditInternal.h"

#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>

namespace
{
	bool EntryMatches(const PackEdit::PePathable& it, const std::string& name)
	{
		if (name.empty())
			return false;
		if (it.trailData == name)
			return true;
		const auto st = PackEdit::EffectiveStyle(it);
		if (st.iconFile == name || st.texture == name)
			return true;
		auto ends = [&](const std::string& s) {
			return s.size() >= name.size() &&
				s.compare(s.size() - name.size(), name.size(), name) == 0;
		};
		return ends(it.trailData) || ends(st.iconFile) || ends(st.texture);
	}
}

void PackEdit::DrawResources()
{
	PadNav::SectionTitle("Resources");
	PadNav::BeginSection("pe_res");
	ImGui::TextDisabled("%zu files — click to copy path and jump to a user.",
		gDoc.entries.size());
	if (ImGui::BeginChild("###pe_reslist", ImVec2(0.f, 160.f), true, PadNav::kNestedList))
	{
		/* Hub PushWrap + clipper made rows unclickable (uneven heights). */
		ImGui::PushTextWrapPos(0.f);
		const int n = static_cast<int>(gDoc.entries.size());
		ImGuiListClipper clip;
		clip.Begin(n);
		while (clip.Step())
		{
			for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i)
			{
				const auto& e = gDoc.entries[static_cast<size_t>(i)];
				ImGui::PushID(i);
				char lab[220]{};
				std::snprintf(lab, sizeof(lab), "%s  (%zu B)", e.name.c_str(), e.bytes.size());
				if (ImGui::Selectable(lab, false, ImGuiSelectableFlags_SpanAvailWidth))
				{
					ImGui::SetClipboardText(e.name.c_str());
					int hit = -1;
					for (int k = 0; k < static_cast<int>(gDoc.items.size()); ++k)
					{
						const auto& it = gDoc.items[static_cast<size_t>(k)];
						if (it.tombstone)
							continue;
						if (EntryMatches(it, e.name))
						{
							hit = k;
							break;
						}
					}
					if (hit >= 0)
						RevealItem(hit);
					std::snprintf(gDoc.status, sizeof(gDoc.status),
						hit >= 0 ? "Resource %s (copied)." : "Copied %s (no marker uses it).",
						e.name.c_str());
				}
				ImGui::PopID();
			}
		}
		ImGui::PopTextWrapPos();
	}
	ImGui::EndChild();
	PadNav::EndSection();
}

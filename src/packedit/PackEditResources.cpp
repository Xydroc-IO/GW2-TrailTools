#include "PackEditInternal.h"

#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <string>

void PackEdit::DrawResources()
{
	using namespace PackEdit;
	PadNav::SectionTitle("Resources");
	PadNav::BeginSection("pe_res");
	ImGui::TextDisabled("%zu pack entries (images, xml, trl, lua).", gDoc.entries.size());
	if (ImGui::BeginChild("###pe_reslist", ImVec2(0.f, 140.f), true))
	{
		for (size_t i = 0; i < gDoc.entries.size(); ++i)
		{
			const auto& e = gDoc.entries[i];
			int refs = 0;
			const std::string& nm = e.name;
			for (const auto& it : gDoc.items)
			{
				if (it.tombstone)
					continue;
				if (it.trailData == nm || it.style.iconFile == nm || it.style.texture == nm)
					++refs;
			}
			ImGui::Text("%s  (%zu B, %d refs)", nm.c_str(), e.bytes.size(), refs);
		}
	}
	ImGui::EndChild();
	PadNav::EndSection();
}

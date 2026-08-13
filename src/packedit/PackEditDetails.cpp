#include "PackEditInternal.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>

void PackEdit::DrawDetails()
{
	using namespace PackEdit;
	PePathable* p = Selected();
	PadNav::SectionTitle("Details");
	PadNav::BeginSection("pe_det");
	if (!p)
	{
		ImGui::TextDisabled("Select a POI or trail in the tree.");
		PadNav::EndSection();
		return;
	}
	ImGui::TextUnformatted(p->isTrail ? "Trail" : "Marker");
	char type[160]{};
	std::snprintf(type, sizeof(type), "%s", p->type.c_str());
	if (ImGui::InputText("type###pe_type", type, sizeof(type)))
	{
		PushUndo();
		p->type = type;
		gDoc.dirty = true;
	}
	if (!p->isTrail)
	{
		int map = static_cast<int>(p->mapId);
		if (ImGui::InputInt("MapID###pe_map", &map))
		{
			PushUndo();
			p->mapId = map > 0 ? static_cast<uint32_t>(map) : 0;
			gDoc.dirty = true;
		}
		float xyz[3] = { p->x, p->y, p->z };
		if (ImGui::InputFloat3("xyz###pe_xyz", xyz, "%.4f"))
		{
			PushUndo();
			p->x = xyz[0];
			p->y = xyz[1];
			p->z = xyz[2];
			gDoc.dirty = true;
		}
		if (ImGui::DragFloat("rotate###pe_rot", &p->rotate, 0.5f, -180.f, 180.f))
		{
			gDoc.dirty = true;
		}
		char guid[96]{};
		std::snprintf(guid, sizeof(guid), "%s", p->guid.c_str());
		if (ImGui::InputText("GUID###pe_guid", guid, sizeof(guid)))
		{
			p->guid = guid;
			gDoc.dirty = true;
		}
	}
	else
	{
		char td[200]{};
		std::snprintf(td, sizeof(td), "%s", p->trailData.c_str());
		if (ImGui::InputText("trailData###pe_td", td, sizeof(td)))
		{
			p->trailData = td;
			gDoc.dirty = true;
		}
		ImGui::TextDisabled("%zu points", p->points.size());
		if (ImGui::BeginChild("###pe_pts", ImVec2(0.f, 120.f), true))
		{
			for (int i = 0; i < static_cast<int>(p->points.size()); ++i)
			{
				auto& pt = p->points[static_cast<size_t>(i)];
				ImGui::PushID(i);
				float v[3] = { pt.x, pt.y, pt.z };
				if (ImGui::InputFloat3("###pt", v, "%.3f"))
				{
					pt.x = v[0];
					pt.y = v[1];
					pt.z = v[2];
					gDoc.selPoint = i;
					gDoc.dirty = true;
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
	if (ImGui::Button("Delete (tombstone)###pe_del"))
		TombstoneSelected();
	PadNav::WrapSameLine(PadNav::ButtonWidth("Duplicate"));
	if (ImGui::Button("Duplicate###pe_dup"))
		DuplicateSelected();

	ImGui::Separator();
	ImGui::TextDisabled("Style (own attrs; category inheritance on world draw)");
	auto* st = &p->style;
	char icon[200]{};
	std::snprintf(icon, sizeof(icon), "%s", st->iconFile.c_str());
	if (ImGui::InputText("iconFile###pe_ico", icon, sizeof(icon)))
	{
		st->iconFile = icon;
		st->hasIconFile = !st->iconFile.empty();
		gDoc.dirty = true;
	}
	char tex[200]{};
	std::snprintf(tex, sizeof(tex), "%s", st->texture.c_str());
	if (ImGui::InputText("texture###pe_tex", tex, sizeof(tex)))
	{
		st->texture = tex;
		st->hasTexture = !st->texture.empty();
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("iconSize###pe_isz", &st->iconSize, 0.05f, 0.05f, 8.f))
	{
		st->hasIconSize = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("trailScale###pe_tsc", &st->trailScale, 0.05f, 0.05f, 8.f))
	{
		st->hasTrailScale = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("alpha###pe_a", &st->alpha, 0.02f, 0.f, 1.f))
	{
		st->hasAlpha = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("fadeNear###pe_fn", &st->fadeNear, 1.f, -1.f, 2000.f))
	{
		st->hasFadeNear = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("fadeFar###pe_ff", &st->fadeFar, 1.f, -1.f, 4000.f))
	{
		st->hasFadeFar = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("heightOffset###pe_ho", &st->heightOffset, 0.1f, -20.f, 40.f))
	{
		st->hasHeightOffset = true;
		gDoc.dirty = true;
	}
	int beh = st->behavior;
	if (ImGui::InputInt("behavior###pe_beh", &beh))
	{
		st->behavior = beh;
		st->hasBehavior = true;
		gDoc.dirty = true;
	}
	if (ImGui::Checkbox("autoTrigger###pe_at", &st->autoTrigger))
	{
		st->hasAutoTrigger = true;
		gDoc.dirty = true;
	}
	if (ImGui::DragFloat("triggerRange###pe_tr", &st->triggerRange, 0.1f, 0.f, 80.f))
	{
		st->hasTriggerRange = true;
		gDoc.dirty = true;
	}
	char info[240]{};
	std::snprintf(info, sizeof(info), "%s", st->info.c_str());
	if (ImGui::InputText("info###pe_info", info, sizeof(info)))
	{
		st->info = info;
		st->hasInfo = !st->info.empty();
		gDoc.dirty = true;
	}
	char copy[240]{};
	std::snprintf(copy, sizeof(copy), "%s", st->copy.c_str());
	if (ImGui::InputText("copy###pe_copy", copy, sizeof(copy)))
	{
		st->copy = copy;
		st->hasCopy = !st->copy.empty();
		gDoc.dirty = true;
	}
	const auto eff = EffectiveStyle(*p);
	ImGui::TextDisabled("Inherited icon: %s",
		eff.iconFile.empty() ? "(none)" : eff.iconFile.c_str());
	PadNav::EndSection();
}

#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"
#include "TrailToolsXml.h"

#include "HelperTheme.h"
#include "PadNav.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>

void TrailToolsDetail::DrawSelectedPoiEditor(DraftPoi& p)
{
	char type[160]{};
	char guid[96]{};
	std::snprintf(type, sizeof(type), "%s", p.type.c_str());
	std::snprintf(guid, sizeof(guid), "%s", p.guid.c_str());
	PadNav::PushWidthForLabel("type###gw2tt_tt_ptype");
	if (ImGui::InputText("type###gw2tt_tt_ptype", type, sizeof(type)))
		p.type = type;
	PadNav::PopWidthForLabel();
	PadNav::PushWidthForLabel("GUID###gw2tt_tt_pguid");
	if (ImGui::InputText("GUID###gw2tt_tt_pguid", guid, sizeof(guid)))
		p.guid = guid;
	PadNav::PopWidthForLabel();
	PadNav::PushWidthForLabel("XYZ###gw2tt_tt_mnudge");
	ImGui::DragFloat3("XYZ###gw2tt_tt_mnudge", &p.x, 0.05f);
	PadNav::PopWidthForLabel();
	if (ImGui::SmallButton("+X")) p.x += 0.5f;
	PadNav::WrapSameLine(PadNav::ButtonWidth("-X"));
	if (ImGui::SmallButton("-X")) p.x -= 0.5f;
	PadNav::WrapSameLine(PadNav::ButtonWidth("+Z"));
	if (ImGui::SmallButton("+Z")) p.z += 0.5f;
	PadNav::WrapSameLine(PadNav::ButtonWidth("-Z"));
	if (ImGui::SmallButton("-Z")) p.z -= 0.5f;
	PadNav::WrapSameLine(PadNav::ButtonWidth("+Y"));
	if (ImGui::SmallButton("+Y")) p.y += 0.25f;
	PadNav::WrapSameLine(PadNav::ButtonWidth("-Y"));
	if (ImGui::SmallButton("-Y")) p.y -= 0.25f;

	DrawPoiBehaviorAndFilters(p);
	DrawPoiScriptAttrs(p);

	ImGui::TextUnformatted("POI XML");
	{
		const std::string line = TrailToolsXml::EmitPoiElement(p);
		PadNav::PushWrap();
		ImGui::TextColored(HelperTheme::Muted, "%s", line.c_str());
		PadNav::PopWrap();
		if (ImGui::Button("Copy POI XML###gw2tt_tt_mcopy"))
		{
			CopyClipboard(line.c_str());
			SetStatus("Copied POI XML.");
		}
	}
}

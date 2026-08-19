#include "TrailToolsBinds.h"
#include "TrailToolsInternal.h"
#include "TrailToolsShared.h"

#include "HelperTheme.h"
#include "PadNav.h"
#include "Settings.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
	float ChordButtonWidth(const char* shown)
	{
		const float pad = ImGui::GetStyle().FramePadding.x * 2.f;
		const float textW = ImGui::CalcTextSize(shown, nullptr, true).x + pad;
		const float minW = ImGui::CalcTextSize("CTRL+NUMPAD0", nullptr, true).x + pad;
		return textW > minW ? textW : minW;
	}

	void BindButton(const char* id, TrailToolsBinds::Chord& chord, int captureId)
	{
		auto& st = TrailToolsBinds::Get();
		const bool listening = st.captureTarget == captureId;
		char lab[96]{};
		if (listening)
			std::snprintf(lab, sizeof(lab), "Press key...###%s", id);
		else
			std::snprintf(lab, sizeof(lab), "%s###%s",
				TrailToolsBinds::FormatChord(chord).c_str(), id);

		char vis[96]{};
		if (const char* hash = std::strstr(lab, "###"))
			std::snprintf(vis, sizeof(vis), "%.*s", (int)(hash - lab), lab);
		else
			std::snprintf(vis, sizeof(vis), "%s", lab);

		const float btnW = ChordButtonWidth(vis);
		if (ImGui::Button(lab, ImVec2(btnW, 0.f)))
			st.captureTarget = listening ? -1 : captureId;
		PadNav::WrapSameLine(PadNav::ButtonWidth("Clear"));
		if (ImGui::SmallButton((std::string("Clear###clr_") + id).c_str()))
		{
			chord = {};
			st.captureTarget = -1;
			Settings::SetDirty();
		}
	}

	void DrawTypeCombo(char* typeBuf, size_t typeLen, const char* id, float width)
	{
		using namespace TrailToolsDetail;
		std::vector<std::string> leaves;
		CollectLeafPaths(gDraft.root, "", leaves, false);
		if (leaves.empty() && gDraft.markerType[0])
			leaves.push_back(gDraft.markerType);
		int cur = -1;
		for (size_t i = 0; i < leaves.size(); ++i)
		{
			if (leaves[i] == typeBuf)
			{
				cur = static_cast<int>(i);
				break;
			}
		}
		const char* preview = typeBuf[0] ? typeBuf
			: (gDraft.markerType[0] ? "(default marker type)" : "(unset)");
		char comboId[64]{};
		std::snprintf(comboId, sizeof(comboId), "###%s_type", id);
		ImGui::SetNextItemWidth(width);
		if (ImGui::BeginCombo(comboId, preview))
		{
			if (ImGui::Selectable("(use default marker type)", typeBuf[0] == 0))
			{
				typeBuf[0] = 0;
				Settings::SetDirty();
			}
			for (size_t i = 0; i < leaves.size(); ++i)
			{
				const bool sel = static_cast<int>(i) == cur;
				if (ImGui::Selectable(leaves[i].c_str(), sel))
				{
					std::snprintf(typeBuf, typeLen, "%s", leaves[i].c_str());
					Settings::SetDirty();
				}
				if (sel)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	/* Label column sized to the longest row label so text is never covered by
	   the chord button (fixed SameLine(168) clipped longer names). */
	float LabelColumnWidth(const char* const* labels, int count)
	{
		float w = ImGui::CalcTextSize("Slot 10").x;
		for (int i = 0; i < count; ++i)
		{
			if (!labels[i])
				continue;
			const float tw = ImGui::CalcTextSize(labels[i], nullptr, true).x;
			if (tw > w)
				w = tw;
		}
		return w + ImGui::GetStyle().ItemInnerSpacing.x + 4.f;
	}

	void BindRow(const char* label, float labelColW, const char* id,
		TrailToolsBinds::Chord& chord, int captureId)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		const float chordW = ChordButtonWidth(
			TrailToolsBinds::FormatChord(chord).c_str());
		const float clearW = PadNav::ButtonWidth("Clear");
		const float need = chordW + clearW + ImGui::GetStyle().ItemSpacing.x * 2.f;
		/* Prefer label | bind on one row; wrap bind under label if pad is narrow. */
		PadNav::WrapSameLine(need);
		if (ImGui::GetCursorPosX() < labelColW)
			ImGui::SetCursorPosX(labelColW);
		BindButton(id, chord, captureId);
	}
}

void TrailToolsDetail::DrawKeybindsTab()
{
	using namespace TrailToolsBinds;
	auto& st = Get();

	PadNav::PushWrap();
	ImGui::TextDisabled(
		"Keybinds work while GW2 is focused (pad can be closed). "
		"Up to %d categories can be assigned to a keybind.",
		kPlaceSlots);
	PadNav::PopWrap();

	if (ImGui::Button("Reset defaults###gw2tt_kb_reset"))
	{
		SetDefaults();
		Settings::SetDirty();
		SetStatus("Keybinds reset to defaults.");
	}
	PadNav::WrapSameLine(ImGui::CalcTextSize("Click a bind, then press the combo. Esc cancels.").x);
	ImGui::TextDisabled("Click a bind, then press the combo. Esc cancels.");

	static const char* kTrailLabels[] = {
		"Start recording (no extra click-points)",
		"Pause / unpause",
		"New trail section",
		"Delete trail segment",
	};
	static const char* kMarkLabels[] = { "Delete marker" };
	const char* allLabels[] = {
		kTrailLabels[0], kTrailLabels[1], kTrailLabels[2], kTrailLabels[3], kMarkLabels[0],
	};
	const float labelCol = LabelColumnWidth(allLabels, 5);

	PadNav::SectionTitle("TRAILS");
	if (st.trailRecording)
	{
		ImGui::SameLine();
		ImGui::TextColored(st.trailPaused ? HelperTheme::Muted : HelperTheme::Ok,
			st.trailPaused ? "(paused)" : "(recording)");
	}

	BindRow(kTrailLabels[0], labelCol, "kb_tstart", st.trailStart, 0);
	BindRow(kTrailLabels[1], labelCol, "kb_tpause", st.trailPause, 1);
	BindRow(kTrailLabels[2], labelCol, "kb_tsec", st.trailSection, 2);
	BindRow(kTrailLabels[3], labelCol, "kb_tdel", st.trailDeleteSeg, 3);

	PadNav::SectionTitle("MARKERS");
	BindRow(kMarkLabels[0], labelCol, "kb_mdel", st.markerDelete, 4);
	ImGui::TextDisabled("Selected POI, or last one if none selected.");

	if (ImGui::BeginTable("###kb_place", 3,
		ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
	{
		ImGui::TableSetupColumn("Marker", ImGuiTableColumnFlags_WidthFixed, 88.f);
		ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Keybind", ImGuiTableColumnFlags_WidthStretch);
		for (int i = 0; i < kPlaceSlots; ++i)
		{
			ImGui::PushID(i);
			ImGui::TableNextColumn();
			char hint[24]{};
			std::snprintf(hint, sizeof(hint), "Marker %d", i + 1);
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputTextWithHint("###lab", hint, st.place[i].label, sizeof(st.place[i].label)))
				Settings::SetDirty();
			ImGui::TableNextColumn();
			char bid[32]{};
			std::snprintf(bid, sizeof(bid), "kb_p%d", i);
			DrawTypeCombo(st.place[i].type, sizeof(st.place[i].type), bid, -1.f);
			ImGui::TableNextColumn();
			BindButton(bid, st.place[i].chord, 10 + i);
			ImGui::SameLine();
			if (ImGui::SmallButton("Place###now"))
				ActionPlaceMarker(i);
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	if (gDraft.status[0])
	{
		PadNav::PushWrap();
		ImGui::TextColored(HelperTheme::Ok, "%s", gDraft.status);
		PadNav::PopWrap();
	}
}

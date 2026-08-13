#include "TrailToolsBinds.h"

#include "Globals.h"
#include "Settings.h"
#include "TrailToolsEditUndo.h"
#include "TrailToolsShared.h"

#include "imgui/imgui.h"

#include <cmath>
#include <cstring>

#include <windows.h>

namespace
{
	using TrailToolsBinds::Chord;
	using TrailToolsBinds::kPlaceSlots;

	bool  gHeld[32]{}; /* edge detect for fixed actions + place slots */
	float gLastSampleX = 0.f, gLastSampleY = 0.f, gLastSampleZ = 0.f;
	bool  gHaveSample = false;

	bool KeyDown(int vk)
	{
		return (GetAsyncKeyState(vk) & 0x8000) != 0;
	}

	bool ModsMatch(const Chord& c)
	{
		const bool ctrl = KeyDown(VK_CONTROL) || KeyDown(VK_LCONTROL) || KeyDown(VK_RCONTROL);
		const bool shift = KeyDown(VK_SHIFT) || KeyDown(VK_LSHIFT) || KeyDown(VK_RSHIFT);
		const bool alt = KeyDown(VK_MENU) || KeyDown(VK_LMENU) || KeyDown(VK_RMENU);
		return ctrl == c.ctrl && shift == c.shift && alt == c.alt;
	}

	bool ChordDown(const Chord& c)
	{
		return c.vk != 0 && ModsMatch(c) && KeyDown(static_cast<int>(c.vk));
	}

	bool Edge(int idx, bool down)
	{
		const bool was = gHeld[idx];
		gHeld[idx] = down;
		return down && !was;
	}

	bool TypingBlocked()
	{
		const ImGuiIO& io = ImGui::GetIO();
		return io.WantTextInput;
	}

	void AppendPointAtFeet(bool requireMapMatch)
	{
		using namespace TrailToolsDetail;
		DraftTrail& tr = RecordingTrail();
		int& sel = RecordingSelectedPoint();
		bool& dirty = RecordingTrailDirty();
		uint32_t mapId = 0;
		float x = 0.f, y = 0.f, z = 0.f;
		if (!ReadMumblePose(mapId, x, y, z))
		{
			SetStatus("No Mumble pose.");
			return;
		}
		if (tr.mapId == 0)
			tr.mapId = mapId;
		else if (requireMapMatch && tr.mapId != mapId)
		{
			SetStatus("Map mismatch - trail %u, you %u.", tr.mapId, mapId);
			return;
		}
		if (tr.type.empty() && gDraft.trailType[0])
			tr.type = gDraft.trailType;
		TrailToolsEditUndo::PushTrail();
		tr.points.push_back({ x, y, z });
		sel = static_cast<int>(tr.points.size()) - 1;
		dirty = true;
		gLastSampleX = x;
		gLastSampleY = y;
		gLastSampleZ = z;
		gHaveSample = true;
	}

	void SampleWhileRecording()
	{
		using namespace TrailToolsDetail;
		auto& gBinds = TrailToolsBinds::Get();
		if (!gBinds.trailRecording || gBinds.trailPaused)
			return;
		DraftTrail& tr = RecordingTrail();
		int& sel = RecordingSelectedPoint();
		bool& dirty = RecordingTrailDirty();
		uint32_t mapId = 0;
		float x = 0.f, y = 0.f, z = 0.f;
		if (!ReadMumblePose(mapId, x, y, z))
			return;
		if (tr.mapId == 0)
			tr.mapId = mapId;
		if (tr.mapId != mapId)
			return;
		const float kMinDist = (std::isfinite(gBinds.trailSampleSpacing) &&
			gBinds.trailSampleSpacing > 0.05f)
			? gBinds.trailSampleSpacing
			: 0.3f;
		if (gHaveSample)
		{
			const float dx = x - gLastSampleX;
			const float dy = y - gLastSampleY;
			const float dz = z - gLastSampleZ;
			if (dx * dx + dy * dy + dz * dz < kMinDist * kMinDist)
				return;
		}
		tr.points.push_back({ x, y, z });
		sel = static_cast<int>(tr.points.size()) - 1;
		dirty = true;
		gLastSampleX = x;
		gLastSampleY = y;
		gLastSampleZ = z;
		gHaveSample = true;
	}
}

void TrailToolsBinds::ActionTrailStart()
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();
	EnsureWorkspace();
	DraftTrail& tr = RecordingTrail();
	if (!gBinds.trailRecording)
	{
		gBinds.trailRecording = true;
		gBinds.trailPaused = false;
		gHaveSample = false;
		if (tr.type.empty() && gDraft.trailType[0])
			tr.type = gDraft.trailType;
		AppendPointAtFeet(false);
		SetStatus("Recording trail... (%zu pts).", tr.points.size());
		return;
	}
	if (gBinds.trailPaused)
	{
		gBinds.trailPaused = false;
		SetStatus("Trail recording resumed.");
		return;
	}
	AppendPointAtFeet(true);
	SetStatus("Keyframe #%zu.", tr.points.size());
}

void TrailToolsBinds::ActionTrailPause()
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();
	if (!gBinds.trailRecording)
	{
		SetStatus("Not recording.");
		return;
	}
	gBinds.trailPaused = !gBinds.trailPaused;
	SetStatus(gBinds.trailPaused ? "Trail recording paused." : "Trail recording resumed.");
}

void TrailToolsBinds::ActionTrailSection()
{
	using namespace TrailToolsDetail;
	DraftTrail& tr = RecordingTrail();
	int& sel = RecordingSelectedPoint();
	TrailToolsEditUndo::PushTrail();
	tr.points.push_back({ 0.f, 0.f, 0.f });
	sel = static_cast<int>(tr.points.size()) - 1;
	RecordingTrailDirty() = true;
	gHaveSample = false;
	SetStatus("Section break added.");
}

void TrailToolsBinds::ActionTrailDeleteSeg()
{
	using namespace TrailToolsDetail;
	DraftTrail& tr = RecordingTrail();
	int& sel = RecordingSelectedPoint();
	if (tr.points.empty())
	{
		SetStatus("No trail points.");
		return;
	}
	TrailToolsEditUndo::PushTrail();
	if (sel >= 0 && sel < static_cast<int>(tr.points.size()))
		tr.points.erase(tr.points.begin() + sel);
	else
	{
		tr.points.pop_back();
		sel = static_cast<int>(tr.points.size()) - 1;
	}
	if (sel >= static_cast<int>(tr.points.size()))
		sel = static_cast<int>(tr.points.size()) - 1;
	RecordingTrailDirty() = true;
	SetStatus("Deleted trail segment (%zu left).", tr.points.size());
}

void TrailToolsBinds::ActionPlaceMarker(int slotIndex)
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();
	EnsureWorkspace();
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	const char* type = gDraft.markerType;
	if (slotIndex >= 0 && slotIndex < kPlaceSlots && gBinds.place[slotIndex].type[0])
		type = gBinds.place[slotIndex].type;
	if (!type || !type[0])
	{
		SetStatus("Set a marker type for this slot (Keybinds tab).");
		return;
	}
	TrailToolsEditUndo::PushPois();
	DraftPoi p;
	p.mapId = mapId;
	p.x = x;
	p.y = y;
	p.z = z;
	p.type = type;
	p.guid = MakeGuidBase64();
	gDraft.pois.push_back(std::move(p));
	gDraft.selectedPoi = static_cast<int>(gDraft.pois.size()) - 1;
	const char* lab = (slotIndex >= 0 && slotIndex < kPlaceSlots && gBinds.place[slotIndex].label[0])
		? gBinds.place[slotIndex].label
		: type;
	SetStatus("Placed %s (#%zu).", lab, gDraft.pois.size());
}

void TrailToolsBinds::ActionDeleteMarker()
{
	using namespace TrailToolsDetail;
	if (gDraft.pois.empty())
	{
		SetStatus("No markers.");
		return;
	}
	TrailToolsEditUndo::PushPois();
	int& sel = gDraft.selectedPoi;
	if (sel < 0 || sel >= static_cast<int>(gDraft.pois.size()))
		sel = static_cast<int>(gDraft.pois.size()) - 1;
	gDraft.pois.erase(gDraft.pois.begin() + sel);
	if (sel >= static_cast<int>(gDraft.pois.size()))
		sel = static_cast<int>(gDraft.pois.size()) - 1;
	SetStatus("Deleted marker (%zu left).", gDraft.pois.size());
}

void TrailToolsBinds::Poll()
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();

	/* Capture mode: next non-modifier key with current mods becomes the bind. */
	if (gBinds.captureTarget >= 0)
	{
		const bool ctrl = KeyDown(VK_CONTROL) || KeyDown(VK_LCONTROL) || KeyDown(VK_RCONTROL);
		const bool shift = KeyDown(VK_SHIFT) || KeyDown(VK_LSHIFT) || KeyDown(VK_RSHIFT);
		const bool alt = KeyDown(VK_MENU) || KeyDown(VK_LMENU) || KeyDown(VK_RMENU);
		for (int vk = 1; vk < 256; ++vk)
		{
			if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL ||
				vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT ||
				vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU ||
				vk == VK_LWIN || vk == VK_RWIN || vk == VK_CAPITAL || vk == VK_NUMLOCK ||
				vk == VK_SCROLL || vk == VK_ESCAPE)
				continue;
			if (!KeyDown(vk))
				continue;
			Chord c;
			c.ctrl = ctrl;
			c.shift = shift;
			c.alt = alt;
			c.vk = static_cast<unsigned>(vk);
			const int t = gBinds.captureTarget;
			if (t == 0) gBinds.trailStart = c;
			else if (t == 1) gBinds.trailPause = c;
			else if (t == 2) gBinds.trailSection = c;
			else if (t == 3) gBinds.trailDeleteSeg = c;
			else if (t == 4) gBinds.markerDelete = c;
			else if (t >= 10 && t < 10 + kPlaceSlots)
				gBinds.place[t - 10].chord = c;
			gBinds.captureTarget = -1;
			Settings::SetDirty();
			/* swallow until release - clear held so we don't fire immediately */
			std::memset(gHeld, 0, sizeof(gHeld));
			return;
		}
		if (KeyDown(VK_ESCAPE))
			gBinds.captureTarget = -1;
		return;
	}

	SampleWhileRecording();

	if (TypingBlocked())
		return;

	if (Edge(0, ChordDown(gBinds.trailStart)))
		ActionTrailStart();
	if (Edge(1, ChordDown(gBinds.trailPause)))
		ActionTrailPause();
	if (Edge(2, ChordDown(gBinds.trailSection)))
		ActionTrailSection();
	if (Edge(3, ChordDown(gBinds.trailDeleteSeg)))
		ActionTrailDeleteSeg();
	if (Edge(4, ChordDown(gBinds.markerDelete)))
		ActionDeleteMarker();
	for (int i = 0; i < kPlaceSlots; ++i)
	{
		if (Edge(10 + i, ChordDown(gBinds.place[i].chord)))
			ActionPlaceMarker(i);
	}
}

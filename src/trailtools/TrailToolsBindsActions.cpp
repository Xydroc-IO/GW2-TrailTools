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
	DWORD gLastSampleTick = 0;
	int   gSampleFrames = 0;

	void MarkSampleNow()
	{
		gLastSampleTick = GetTickCount();
		if (gLastSampleTick == 0)
			gLastSampleTick = 1;
		gSampleFrames = 0;
	}

	bool SampleIntervalElapsed(float spacingSec)
	{
		++gSampleFrames;
		const DWORD now = GetTickCount();
		DWORD needMs = static_cast<DWORD>(spacingSec * 1000.f + 0.5f);
		if (needMs < 50)
			needMs = 50;
		if (needMs > 5000)
			needMs = 5000;
		int needFr = static_cast<int>(std::lround(static_cast<double>(spacingSec) * 60.0));
		if (needFr < 2)
			needFr = 2;
		if (needFr > 300)
			needFr = 300;
		const bool timeDue = gLastSampleTick != 0 && (now - gLastSampleTick) >= needMs;
		const bool frameDue = gSampleFrames >= needFr;
		if (!timeDue && !frameDue)
			return false;
		gLastSampleTick = now == 0 ? 1 : now;
		gSampleFrames = 0;
		return true;
	}

	bool MovedFrom(float x, float y, float z, float ox, float oy, float oz)
	{
		const float dx = x - ox, dy = y - oy, dz = z - oz;
		return dx * dx + dy * dy + dz * dz >= 0.12f * 0.12f;
	}

	bool LastRealPoint(const TrailToolsDetail::DraftTrail& tr,
		float& ox, float& oy, float& oz)
	{
		for (int i = static_cast<int>(tr.points.size()) - 1; i >= 0; --i)
		{
			const auto& p = tr.points[static_cast<size_t>(i)];
			if (p.x == 0.f && p.y == 0.f && p.z == 0.f)
				continue;
			ox = p.x;
			oy = p.y;
			oz = p.z;
			return true;
		}
		return false;
	}

	void RememberSample(float x, float y, float z)
	{
		gLastSampleX = x;
		gLastSampleY = y;
		gLastSampleZ = z;
		gHaveSample = true;
	}

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
		if (c.vk == 0 || c.vk <= VK_XBUTTON2)
			return false;
		return ModsMatch(c) && KeyDown(static_cast<int>(c.vk));
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
		RememberSample(x, y, z);
		MarkSampleNow();
	}

	void SampleWhileRecording()
	{
		using namespace TrailToolsDetail;
		auto& gBinds = TrailToolsBinds::Get();
		if (!gBinds.trailRecording || gBinds.trailPaused)
			return;
		DraftTrail& tr = RecordingTrail();
		bool& dirty = RecordingTrailDirty();
		uint32_t mapId = 0;
		float x = 0.f, y = 0.f, z = 0.f;
		if (!ReadMumblePose(mapId, x, y, z))
			return;
		if (tr.type.empty() && gDraft.trailType[0])
			tr.type = gDraft.trailType;
		if (tr.mapId == 0)
			tr.mapId = mapId;
		else if (tr.mapId != mapId)
			return;

		const float spacing = (std::isfinite(gBinds.trailSampleSpacing) &&
			gBinds.trailSampleSpacing >= 0.05f)
			? gBinds.trailSampleSpacing
			: 0.3f;
		if (!SampleIntervalElapsed(spacing))
			return;

		float lx = 0.f, ly = 0.f, lz = 0.f;
		const bool haveLast = LastRealPoint(tr, lx, ly, lz);
		if (haveLast && !MovedFrom(x, y, z, lx, ly, lz))
			return;
		if (gHaveSample && !MovedFrom(x, y, z, gLastSampleX, gLastSampleY, gLastSampleZ))
			return;

		tr.points.push_back({ x, y, z });
		dirty = true;
		RememberSample(x, y, z);
	}
}

void TrailToolsBinds::ActionTrailStart()
{
	using namespace TrailToolsDetail;
	auto& gBinds = Get();
	EnsureWorkspace();
	bool anyTrailEd = false;
	for (int i = 0; i < kMaxTrailEditors; ++i)
	{
		if (gTrailEditors[i].open)
		{
			anyTrailEd = true;
			break;
		}
	}
	if (!anyTrailEd)
		OpenNewTrailEditor();
	if (gTrailEditorDrawActive && gTrailEditorDrawSlot >= 0)
		gTrailRecordSlot = gTrailEditorDrawSlot;
	DraftTrail& tr = RecordingTrail();
	if (!gBinds.trailRecording)
	{
		gBinds.trailRecording = true;
		gBinds.trailPaused = false;
		RecordingWorldShown() = true;
		if (tr.type.empty() && gDraft.trailType[0])
			tr.type = gDraft.trailType;
		uint32_t mapId = 0;
		float x = 0.f, y = 0.f, z = 0.f;
		if (ReadMumblePose(mapId, x, y, z))
		{
			if (tr.mapId == 0 && mapId != 0)
				tr.mapId = mapId;
			const bool mapChange = tr.mapId != 0 && mapId != 0 && tr.mapId != mapId;
			if (mapChange)
			{
				int& sel = RecordingSelectedPoint();
				TrailToolsEditUndo::PushTrail();
				if (!tr.points.empty())
					tr.points.push_back({ 0.f, 0.f, 0.f });
				tr.mapId = mapId;
				tr.points.push_back({ x, y, z });
				sel = static_cast<int>(tr.points.size()) - 1;
				RecordingTrailDirty() = true;
				RememberSample(x, y, z);
			}
			else if (tr.points.empty())
				AppendPointAtFeet(false);
			else
				RememberSample(x, y, z);
		}
		MarkSampleNow();
		SetStatus("Recording trail... (%zu pts).", tr.points.size());
		return;
	}
	if (gBinds.trailPaused)
	{
		gBinds.trailPaused = false;
		MarkSampleNow();
		SetStatus("Trail recording resumed.");
		return;
	}
	SetStatus("Already recording — Pause or Stop.");
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
	if (!gBinds.trailPaused)
		MarkSampleNow();
	SetStatus(gBinds.trailPaused ? "Trail recording paused." : "Trail recording resumed.");
}

void TrailToolsBinds::ActionTrailSection()
{
	using namespace TrailToolsDetail;
	EnsureWorkspace();
	DraftTrail& tr = RecordingTrail();
	int& sel = RecordingSelectedPoint();
	uint32_t mapId = 0;
	float x = 0.f, y = 0.f, z = 0.f;
	if (!ReadMumblePose(mapId, x, y, z))
	{
		SetStatus("No Mumble pose.");
		return;
	}
	if (tr.type.empty() && gDraft.trailType[0])
		tr.type = gDraft.trailType;
	TrailToolsEditUndo::PushTrail();
	const bool empty = tr.points.empty();
	if (!empty)
		tr.points.push_back({ 0.f, 0.f, 0.f });
	tr.mapId = mapId;
	tr.points.push_back({ x, y, z });
	sel = static_cast<int>(tr.points.size()) - 1;
	RecordingTrailDirty() = true;
	RememberSample(x, y, z);
	gHaveSample = false;
	MarkSampleNow();
	SetStatus("New segment on map %u (%zu pts).", mapId, tr.points.size());
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
			if (vk <= VK_XBUTTON2)
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

void TrailToolsBinds::PollRecording()
{
	SampleWhileRecording();
}

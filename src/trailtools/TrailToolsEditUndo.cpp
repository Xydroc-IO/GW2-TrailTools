#include "TrailToolsEditUndo.h"

#include "TrailToolsShared.h"

#include "imgui/imgui.h"

#include <vector>

namespace
{
	constexpr int kCap = 32;

	struct TrailSnap
	{
		TrailToolsDetail::DraftTrail trail;
		int selectedPoint = -1;
	};
	struct PoiSnap
	{
		std::vector<TrailToolsDetail::DraftPoi> pois;
		int selectedPoi = -1;
	};

	std::vector<TrailSnap> gTrailUndo;
	std::vector<PoiSnap> gPoiUndo;
}

void TrailToolsEditUndo::PushTrail()
{
	using namespace TrailToolsDetail;
	if (static_cast<int>(gTrailUndo.size()) >= kCap)
		gTrailUndo.erase(gTrailUndo.begin());
	TrailSnap s;
	s.trail = RecordingTrail();
	s.selectedPoint = RecordingSelectedPoint();
	gTrailUndo.push_back(std::move(s));
}

void TrailToolsEditUndo::PushPois()
{
	using namespace TrailToolsDetail;
	if (static_cast<int>(gPoiUndo.size()) >= kCap)
		gPoiUndo.erase(gPoiUndo.begin());
	PoiSnap s;
	s.pois = gDraft.pois;
	s.selectedPoi = gDraft.selectedPoi;
	gPoiUndo.push_back(std::move(s));
}

bool TrailToolsEditUndo::CanUndoTrail()
{
	return !gTrailUndo.empty();
}

bool TrailToolsEditUndo::CanUndoPois()
{
	return !gPoiUndo.empty();
}

bool TrailToolsEditUndo::UndoTrail()
{
	using namespace TrailToolsDetail;
	if (gTrailUndo.empty())
		return false;
	TrailSnap s = std::move(gTrailUndo.back());
	gTrailUndo.pop_back();
	RecordingTrail() = std::move(s.trail);
	RecordingSelectedPoint() = s.selectedPoint;
	RecordingTrailDirty() = true;
	SetStatus("Trail undo (%zu pts).", RecordingTrail().points.size());
	return true;
}

bool TrailToolsEditUndo::UndoPois()
{
	using namespace TrailToolsDetail;
	if (gPoiUndo.empty())
		return false;
	PoiSnap s = std::move(gPoiUndo.back());
	gPoiUndo.pop_back();
	gDraft.pois = std::move(s.pois);
	gDraft.selectedPoi = s.selectedPoi;
	SetStatus("Marker undo (%zu left).", gDraft.pois.size());
	return true;
}

void TrailToolsEditUndo::PollTrailHotkey(bool padFocused)
{
	if (!padFocused)
		return;
	const ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		UndoTrail();
}

void TrailToolsEditUndo::PollPoisHotkey(bool padFocused)
{
	if (!padFocused)
		return;
	const ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		UndoPois();
}

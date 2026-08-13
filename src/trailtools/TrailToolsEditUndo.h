#pragma once

#include "TrailToolsShared.h"

/* Snapshot undo for active trail points and POI list (ring ~32). */
namespace TrailToolsEditUndo
{
	void PushTrail();
	void PushPois();
	bool CanUndoTrail();
	bool CanUndoPois();
	bool UndoTrail();
	bool UndoPois();
	/* Call each frame from TrailsN / MarkersN when focused — Ctrl+Z. */
	void PollTrailHotkey(bool padFocused);
	void PollPoisHotkey(bool padFocused);
}

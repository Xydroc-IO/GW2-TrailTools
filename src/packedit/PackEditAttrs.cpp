#include "PackEdit.h"

namespace PackEdit
{
	static const AttrDef kTable[] = {
		{ "type", AttrKind::String, true, true, false },
		{ "GUID", AttrKind::String, true, false, false },
		{ "MapID", AttrKind::Int, true, true, true },
		{ "xpos", AttrKind::Float, true, false, false },
		{ "ypos", AttrKind::Float, true, false, false },
		{ "zpos", AttrKind::Float, true, false, false },
		{ "rotate", AttrKind::Float, true, false, false },
		{ "trailData", AttrKind::String, false, true, false },
		{ "iconFile", AttrKind::String, true, false, true },
		{ "texture", AttrKind::String, false, true, true },
		{ "iconSize", AttrKind::Float, true, false, true },
		{ "trailScale", AttrKind::Float, false, true, true },
		{ "alpha", AttrKind::Float, true, true, true },
		{ "fadeNear", AttrKind::Float, true, true, true },
		{ "fadeFar", AttrKind::Float, true, true, true },
		{ "heightOffset", AttrKind::Float, true, false, true },
		{ "mapDisplaySize", AttrKind::Float, true, false, true },
		{ "minSize", AttrKind::Float, true, false, true },
		{ "maxSize", AttrKind::Float, true, false, true },
		{ "color", AttrKind::String, true, true, true },
		{ "behavior", AttrKind::Int, true, false, true },
		{ "autoTrigger", AttrKind::Bool, true, false, true },
		{ "triggerRange", AttrKind::Float, true, false, true },
		{ "resetLength", AttrKind::Float, true, false, true },
		{ "invertBehavior", AttrKind::Bool, true, false, true },
		{ "inGameVisible", AttrKind::Bool, true, true, true },
		{ "minimapVisible", AttrKind::Bool, true, true, true },
		{ "DisplayName", AttrKind::String, false, false, true },
		{ "tip-name", AttrKind::String, true, false, true },
		{ "info", AttrKind::String, true, false, false },
		{ "copy", AttrKind::String, true, false, false },
		{ "schedule", AttrKind::String, true, true, true },
		{ "script-once", AttrKind::String, true, false, false },
		{ "script-trigger", AttrKind::String, true, false, false },
	};

	const AttrDef* AttrTable(int& count)
	{
		count = static_cast<int>(sizeof(kTable) / sizeof(kTable[0]));
		return kTable;
	}
}

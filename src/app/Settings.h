#pragma once

namespace Settings
{
	void Load();
	void Save(bool force = false);
	void SetDirty();
	void SaveNow();
}

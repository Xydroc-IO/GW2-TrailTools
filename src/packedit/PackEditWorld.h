#pragma once

#include "WorldGpsMath.h"

#include <cstdint>
#include <vector>

/* Editor world pass: distance-cull then D3D trails + a few ImGui markers.
   One instance — opening a large .taco must not draw the whole pack every frame. */
namespace PackEdit
{
	class WorldGpu
	{
	public:
		static WorldGpu& Get();
		void Draw();

	private:
		void Cull(uint32_t mapId, const WorldGpsMath::Vec3& avatar, float maxDist);
		std::vector<int> marks_;
		std::vector<int> trails_;
	};
}

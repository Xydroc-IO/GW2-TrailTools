#include "PackEditInternal.h"

#include "imgui/imgui.h"

void PackEdit::DrawPopouts()
{
	if (gDoc.popTree)
	{
		if (ImGui::Begin("Pack tree###pe_pop_tree", &gDoc.popTree))
			DrawTree();
		ImGui::End();
	}
	if (gDoc.popDet)
	{
		if (ImGui::Begin("Pack details###pe_pop_det", &gDoc.popDet))
			DrawDetails();
		ImGui::End();
	}
	if (gDoc.popRes)
	{
		if (ImGui::Begin("Pack resources###pe_pop_res", &gDoc.popRes))
			DrawResources();
		ImGui::End();
	}
	if (gDoc.popMap)
	{
		if (ImGui::Begin("Pack 2D map###pe_pop_map", &gDoc.popMap))
			DrawCanvas();
		ImGui::End();
	}
}

#include "cm/cm_typedefs.hpp"
#include "r_world.hpp"
#include "net/nvar_table.hpp"
#include "r/gui/r_main_gui.hpp"
#include "shared/sv_shared.hpp"

CWorldWindow::CWorldWindow(const std::string& name)
	: CGuiElement(name) {

}


void CWorldWindow::Render()
{
#if(!DEBUG_SUPPORT)
	static auto func = CMain::Shared::GetFunctionSafe("GetContext");

	if (!func) {
		func = CMain::Shared::GetFunctionSafe("GetContext");
		return;
	}

	ImGui::SetCurrentContext(func->As<ImGuiContext*>()->Call());

#endif

	GUI_RenderNVars();

	ImGui::NewLine();

	ImGui::Text("remove collision from selected brushes when the diameter is greater than");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50.f);
	ImGui::DragFloat("##volume", &m_fBrushVolume, 10.f, 0.f, 0.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	ImGui::Text("units");

	if (ImGui::ButtonCentered("Apply")) {
		std::unique_lock lock(CClipMap::GetLock());
		CClipMap::RemoveBrushCollisionsBasedOnVolume(m_fBrushVolume);
	}
	if (ImGui::ButtonCentered("Restore Changes")) {
		std::unique_lock lock(CClipMap::GetLock());
		CClipMap::RestoreBrushCollisions();
	}


	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Text("note: if you are rendering entity connections, disable \"As Polygons\"");

}


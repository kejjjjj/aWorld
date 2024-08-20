#include "main.hpp"
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
	ImGui::Separator();
	ImGui::Text("note: if you are rendering entity connections, disable \"As Polygons\"");

}


#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cl/cl_utils.hpp"
#include "net/nvar_table.hpp"
#include "r/gui/r_main_gui.hpp"
#include "r/r_drawtools.hpp"
#include "r/r_utils.hpp"
#include "r_drawactive.hpp"
#include "utils/hook.hpp"
#include "utils/typedefs.hpp"

#include "cm/cm_typedefs.hpp"
#include "cm/cm_entity.hpp"

void CG_DrawActive()
{
	if (R_NoRender())
#if(DEBUG_SUPPORT)
		return hooktable::find<void>(HOOK_PREFIX(__func__))->call();
#else
		return;
#endif
	const auto showCollision = NVar_FindMalleableVar<bool>("Show Collision");

	if (showCollision->Get() && showCollision->GetChildAs<ImNVar<std::string>>("Entity Filter")->GetChild("Spawnstrings")->As<ImNVar<bool>>()->Get()) {
		CGentities::ForEach([&showCollision](const GentityPtr_t& ptr) { 
			if(ptr)
				ptr->CG_Render2D(showCollision->GetChildAs<ImNVar<float>>("Draw Distance")->Get()); 
		});
	}

#if(DEBUG_SUPPORT)
	return hooktable::find<void>(HOOK_PREFIX(__func__))->call();
#endif



}
#include "cm_renderer.hpp"
#include "cm_brush.hpp"

#include "r/r_utils.hpp"
#include "utils/hook.hpp"

void RB_DrawDebug([[maybe_unused]] GfxViewParms* viewParms)
{

	if (R_NoRender())
#if(DEBUG_SUPPORT)
		return hooktable::find<void, GfxViewParms*>(HOOK_PREFIX(__func__))->call(viewParms);
#else
		return;
#endif

#if(DEBUG_SUPPORT)
	hooktable::find<void, GfxViewParms*>(HOOK_PREFIX(__func__))->call(viewParms);
#endif
	
	CM_ShowCollision(viewParms);

}


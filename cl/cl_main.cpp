#include "cl_main.hpp"
#include "cl/cl_utils.hpp"
#include "utils/hook.hpp"
#include <cg/cg_local.hpp>
#include <cg/cg_offsets.hpp>
#include "cm/cm_typedefs.hpp"

void CL_Disconnect(int clientNum)
{
	if (clientUI->connectionState != CA_DISCONNECTED) { //gets called in the loading screen in 1.7
		CClipMap::ClearThreadSafe();
		CGentities::ClearThreadSafe();
	}

	hooktable::find<void, int>(HOOK_PREFIX(__func__))->call(clientNum);

}

void CL_CreateNewCommands([[maybe_unused]] int localClientNum)
{

}

#pragma once

#include <vector>

namespace Varjus
{
	struct CRuntimeContext;
}

using IValues = std::vector<class IValue*>;
IValue* WorldDebugString(Varjus::CRuntimeContext* const ctx, IValue* _this, const IValues& args);
IValue* WorldDebugLine(Varjus::CRuntimeContext* const ctx, IValue* _this, const IValues& args);

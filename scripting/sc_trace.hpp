#pragma once

#include <vector>

namespace Varjus
{
	struct CRuntimeContext;
}

using IValues = std::vector<class IValue*>;
[[nodiscard]] IValue* WorldTrace(Varjus::CRuntimeContext* const ctx, IValue* _this, const IValues& args);
[[nodiscard]] IValue* WorldPlayerTrace(Varjus::CRuntimeContext* const ctx, IValue* _this, const IValues& args);

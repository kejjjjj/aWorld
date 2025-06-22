#pragma once

#include <vector>

using IValues = std::vector<class IValue*>;
class IValue* WorldDebugString(struct CRuntimeContext* const ctx, IValue* _this, const IValues& args);
class IValue* WorldDebugLine(struct CRuntimeContext* const ctx, IValue* _this, const IValues& args);

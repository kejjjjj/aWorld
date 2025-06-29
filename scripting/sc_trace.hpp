#pragma once

#include <vector>

using IValues = std::vector<class IValue*>;
[[nodiscard]] IValue* WorldTrace(struct CRuntimeContext* const ctx, IValue* _this, const IValues& args);
[[nodiscard]] IValue* WorldPlayerTrace(struct CRuntimeContext* const ctx, IValue* _this, const IValues& args);

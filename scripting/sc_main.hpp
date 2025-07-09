#pragma once

#include "varjus_api/varjus_api.hpp"
#include "varjus_api/internal/variables.hpp"
#include "varjus_api/internal/exceptions/exception.hpp"
#include "utils/typedefs.hpp"

#include <ranges>

template<typename ... Args>
inline CObjectValue* ToKVObject(Varjus::CProgramRuntime* const rt, Args&&... args) {
	return CObjectValue::Construct(rt, { { CStringValue::Construct(rt, std::get<0>(args)), std::get<1>(args) }... });
}

inline auto ToVec3FromObject(Varjus::CProgramRuntime* const rt, const fvec3& vec)
{
	IValues values = { CDoubleValue::Construct(rt, vec.x), CDoubleValue::Construct(rt, vec.y), CDoubleValue::Construct(rt, vec.z) };

	return CArrayValue::Construct(rt, std::move(values));
	//return ToKVObject(rt,
	//	std::make_pair(VSL("x"), CDoubleValue::Construct(rt, vec.x)),
	//	std::make_pair(VSL("y"), CDoubleValue::Construct(rt, vec.y)),
	//	std::make_pair(VSL("z"), CDoubleValue::Construct(rt, vec.z))
	//);
}

template<std::size_t count = 3>
inline fvec3 IsVecArray(Varjus::CProgramRuntime* const rt, const IValue* v) {
	auto asArray = v->ToArray();

	auto& vars = asArray->Internal()->GetContent().GetVariables();
	if (vars.size() != count)
		throw Varjus::CRuntimeError(rt, VSL("expected 3 elements for the array"));

	fvec3 vec3;
	for (std::size_t i{}; auto & var : vars | std::views::take(count)) {
		if (!var->GetValue()->IsArithmetic())
			throw Varjus::CRuntimeError(rt, VSL("expected an arithmetic type"));

		vec3[i++] = static_cast<float>(var->GetValue()->ToDouble());
	}

	return vec3;
};

[[nodiscard]] Varjus::Success SC_AddWorldObjects(Varjus::State& state);
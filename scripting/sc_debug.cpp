#include "sc_debug.hpp"

#include "sc_main.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_trace.hpp"
#include "r/backend/rb_endscene.hpp"

#include <ranges>

auto ToColor(Varjus::CProgramRuntime* const rt, const IValue* v, float* receiver) {
	auto asArray = v->ToArray();

	auto& vars = asArray->Internal()->GetContent().GetVariables();
	if (vars.size() != 4)
		throw Varjus::CRuntimeError(rt, VSL("expected 4 elements for the array"));

	for (std::size_t i{}; auto & var : vars | std::views::take(4)) {
		if (!var->GetValue()->IsArithmetic())
			throw Varjus::CRuntimeError(rt, VSL("expected an arithmetic type"));

		receiver[i++] = static_cast<float>(var->GetValue()->ToDouble());
	}

	return receiver;
}

VARJUS_DEFINE_METHOD(WorldDebugString, ctx, _this, args)
{
	if (args[0]->Type() != Varjus::t_array)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected origin to be an array"));

	if (args[1]->Type() != Varjus::t_array)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected color to be an array"));

	if (!args[2]->IsArithmetic())
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected scale to be arithmetic"));

	if (args[3]->Type() != Varjus::t_string)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected text to be a string"));

	if (!args[4]->IsArithmetic())
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected duration to be arithmetic"));

	float col[4]{};
	CL_AddDebugString(0, IsVecArray(ctx->m_pRuntime, args[0]), ToColor(ctx->m_pRuntime, args[1], col),
		static_cast<float>(args[2]->ToDouble()), args[3]->ToString().c_str(), args[4]->ToInt());

	return IValue::Construct(ctx->m_pRuntime);
}

VARJUS_DEFINE_METHOD(WorldDebugLine, ctx, _this, args)
{
	if (args[0]->Type() != Varjus::t_array)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected start to be an array"));

	if (args[1]->Type() != Varjus::t_array)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected end to be an array"));

	if (args[2]->Type() != Varjus::t_array)
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected color to be an array"));

	if (!args[3]->IsBooleanConvertible())
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected depthtest to be boolean convertible"));

	if (!args[4]->IsArithmetic())
		throw Varjus::CRuntimeError(ctx->m_pRuntime, VSL("expected duration to be arithmetic"));

	float col[4]{};

	CG_DebugLine(IsVecArray(ctx->m_pRuntime, args[0]), IsVecArray(ctx->m_pRuntime, args[1]),
		ToColor(ctx->m_pRuntime, args[2], col), args[3]->ToBoolean(), args[4]->ToInt());

	return IValue::Construct(ctx->m_pRuntime);
}
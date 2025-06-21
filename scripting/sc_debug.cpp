#include "sc_debug.hpp"

#include "sc_main.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_trace.hpp"
#include "r/backend/rb_endscene.hpp"

#include <ranges>

VARJUS_DEFINE_METHOD(WorldDebugString, ctx, _this, args)
{
	if (args[0]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected origin to be an array"));

	if (args[1]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected color to be an array"));

	if (!args[2]->IsArithmetic())
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected scale to be arithmetic"));

	if (args[3]->Type() != t_string)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected text to be a string"));

	if (!args[4]->IsArithmetic())
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected duration to be arithmetic"));

	float col[4]{};

	const auto ToColor = [&](const IValue* v) {

		auto asArray = v->ToArray();

		auto& vars = asArray->Internal()->GetContent().GetVariables();
		if (vars.size() != 4)
			throw CRuntimeError(ctx->m_pRuntime, VSL("expected 4 elements for the array"));

		for (std::size_t i{}; auto & var : vars | std::views::take(4)) {
			if (!var->GetValue()->IsArithmetic())
				throw CRuntimeError(ctx->m_pRuntime, VSL("expected an arithmetic type"));

			col[i++] = static_cast<float>(var->GetValue()->ToDouble());
		}

		return col;
		};

	CL_AddDebugString(0, IsVecArray(ctx->m_pRuntime, args[0]), ToColor(args[1]),
		static_cast<float>(args[2]->ToDouble()), args[3]->ToString().c_str(), args[4]->ToInt());

	return IValue::Construct(ctx->m_pRuntime);
}

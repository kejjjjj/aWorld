#include "sc_trace.hpp"

#include "sc_main.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_trace.hpp"
#include "r/backend/rb_endscene.hpp"

#include <ranges>

VARJUS_DEFINE_METHOD(WorldTrace, ctx, _this, args)
{

	if (args[0]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected mins to be an array"));

	if (args[1]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected maxs to be an array"));

	if (args[2]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected start to be an array"));

	if (args[3]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected end to be an array"));

	if (!args[4]->IsIntegral())
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected mask to be integral"));

	const auto start = IsVecArray(ctx->m_pRuntime, args[2]);
	const auto end = IsVecArray(ctx->m_pRuntime, args[3]);

	const auto t = CG_TracePoint(
		IsVecArray(ctx->m_pRuntime, args[0]), IsVecArray(ctx->m_pRuntime, args[1]),
		start, end, args[4]->ToInt());

	const auto hitpos = (start + (end - start) * t.fraction);

	return ToKVObject(ctx->m_pRuntime,
		std::make_pair(VSL("normal"), ToVec3FromObject(ctx->m_pRuntime, t.normal)),
		std::make_pair(VSL("material"), CStringValue::Construct(ctx->m_pRuntime, t.material == nullptr ? "N/A" : t.material)),
		std::make_pair(VSL("fraction"), CDoubleValue::Construct(ctx->m_pRuntime, t.fraction)),
		std::make_pair(VSL("contents"), CIntValue::Construct(ctx->m_pRuntime, t.contents)),
		std::make_pair(VSL("surfaceFlags"), CIntValue::Construct(ctx->m_pRuntime, t.surfaceFlags)),
		std::make_pair(VSL("allsolid"), CBooleanValue::Construct(ctx->m_pRuntime, t.allsolid)),
		std::make_pair(VSL("startsolid"), CBooleanValue::Construct(ctx->m_pRuntime, t.startsolid)),
		std::make_pair(VSL("hitpos"), ToVec3FromObject(ctx->m_pRuntime, hitpos))
	);


}
VARJUS_DEFINE_METHOD(WorldPlayerTrace, ctx, _this, args)
{

	if (args[0]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected mins to be an array"));

	if (args[1]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected maxs to be an array"));

	if (args[2]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected start to be an array"));

	if (args[3]->Type() != t_array)
		throw CRuntimeError(ctx->m_pRuntime, VSL("expected end to be an array"));

	const auto start = IsVecArray(ctx->m_pRuntime, args[2]);
	const auto end = IsVecArray(ctx->m_pRuntime, args[3]);

	const auto t = CG_TracePoint(
		IsVecArray(ctx->m_pRuntime, args[0]), IsVecArray(ctx->m_pRuntime, args[1]),
		start, end, MASK_PLAYERSOLID);

	const auto hitpos = (start + (end - start) * t.fraction);

	return ToKVObject(ctx->m_pRuntime,
		std::make_pair(VSL("normal"), ToVec3FromObject(ctx->m_pRuntime, t.normal)),
		std::make_pair(VSL("material"), CStringValue::Construct(ctx->m_pRuntime, t.material == nullptr ? "N/A" : t.material)),
		std::make_pair(VSL("fraction"), CDoubleValue::Construct(ctx->m_pRuntime, t.fraction)),
		std::make_pair(VSL("contents"), CIntValue::Construct(ctx->m_pRuntime, t.contents)),
		std::make_pair(VSL("surfaceFlags"), CIntValue::Construct(ctx->m_pRuntime, t.surfaceFlags)),
		std::make_pair(VSL("allsolid"), CBooleanValue::Construct(ctx->m_pRuntime, t.allsolid)),
		std::make_pair(VSL("startsolid"), CBooleanValue::Construct(ctx->m_pRuntime, t.startsolid)),
		std::make_pair(VSL("hitpos"), ToVec3FromObject(ctx->m_pRuntime, hitpos))
	);


}
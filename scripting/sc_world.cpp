#include "sc_main.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_trace.hpp"
#include "r/backend/rb_endscene.hpp"

#include "varjus_api/varjus_api.hpp"
#include "varjus_api/internal/exceptions/exception.hpp"
#include "varjus_api/internal/variables.hpp"

#include <ranges>

template<typename ... Args>
CObjectValue* ToKVObject(CProgramRuntime* const rt, Args&&... args) {
	return CObjectValue::Construct(rt, { { CStringValue::Construct(rt, std::get<0>(args)), std::get<1>(args) }... });
}

CObjectValue* ToVec3FromObject(CProgramRuntime* const rt, const fvec3& vec)
{
	return ToKVObject(rt, 
		std::make_pair(VSL("x"), CDoubleValue::Construct(rt, vec.x)), 
		std::make_pair(VSL("y"), CDoubleValue::Construct(rt, vec.y)),
		std::make_pair(VSL("z"), CDoubleValue::Construct(rt, vec.z))
	);
}

CArrayValue* GenerateWindingArray(CProgramRuntime* const rt, [[maybe_unused]]const std::vector<cm_winding>& w)
{
	IValues values;

	for (const auto& winding : w) {
		values.push_back(ToKVObject(rt, 
			std::make_pair(VSL("normal"), ToVec3FromObject(rt, winding.normals)),
			std::make_pair(VSL("origin"), ToVec3FromObject(rt, winding.get_center()))
		));
	}

	return CArrayValue::Construct(rt, std::move(values));
}

VARJUS_DEFINE_PROPERTY(WorldBrushes, ctx, _this)
{
	IValues brushes;

	std::lock_guard<std::mutex> lock(CClipMap::GetLock());

	CClipMap::ForEach([&](GeometryPtr_t& geom) {
		
		if (geom->type() != cm_geomtype::brush)
			return;

		const auto asBrush = geom->AsBrush();

		if (!asBrush || !asBrush->brush)
			return;

		const auto contents = CIntValue::Construct(ctx->m_pRuntime, asBrush->brush->contents);

		brushes.emplace_back(ToKVObject(ctx->m_pRuntime, 
			std::make_pair(VSL("contents"), contents),
			std::make_pair(VSL("windings"), GenerateWindingArray(ctx->m_pRuntime, asBrush->windings))
		));

	});

	return CArrayValue::Construct(ctx->m_pRuntime, std::move(brushes));
}

template<std::size_t count=3>
static fvec3 IsVecArray(CProgramRuntime* const rt, const IValue * v) {
	auto asArray = v->ToArray();

	auto& vars = asArray->GetVariables();
	if (vars.size() != count)
		throw CRuntimeError(rt, VSL("expected 3 elements for the array"));

	fvec3 vec3;
	for (std::size_t i{}; auto & var : vars | std::views::take(count)) {
		if (!var->GetValue()->IsArithmetic())
			throw CRuntimeError(rt, VSL("expected an arithmetic type"));

		vec3[i++] = static_cast<float>(var->GetValue()->ToDouble());
	}

	return vec3;
};



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

		auto& vars = asArray->GetVariables();
		if (vars.size() != 4)
			throw CRuntimeError(ctx->m_pRuntime, VSL("expected 4 elements for the array"));

		for (std::size_t i{}; auto& var : vars | std::views::take(4)) {
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


VARJUS_DEFINE_STATIC_OBJECT(WorldObject, receiver) {
	receiver.AddProperty("brushes", WorldBrushes);

	receiver.AddMethod("trace", WorldTrace, 5);
	receiver.AddMethod("debug_string", WorldDebugString, 5);

}

Success SC_AddWorldObjects(Varjus::State& state)
{
	if (!state.AddNewStaticObject(VSL("world"), WorldObject))
		return failure;

	return success;
}

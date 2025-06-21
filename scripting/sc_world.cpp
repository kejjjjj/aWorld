#include "sc_main.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_typedefs.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cg/cg_trace.hpp"
#include "r/backend/rb_endscene.hpp"
#include "sc_trace.hpp"
#include "sc_debug.hpp"

#include "varjus_api/varjus_api.hpp"
#include "varjus_api/internal/exceptions/exception.hpp"
#include "varjus_api/internal/variables.hpp"

#include <ranges>
#include <cm/cm_model.hpp>


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
CArrayValue* GenerateTriangleArray(CProgramRuntime* const rt, [[maybe_unused]] const std::vector<cm_triangle>& tris)
{
	IValues values;

	const auto generate_pts = [&rt](const cm_triangle& tri) {
		
		IValues pts(3);
		pts[0] = ToVec3FromObject(rt, tri.a);
		pts[1] = ToVec3FromObject(rt, tri.b);
		pts[2] = ToVec3FromObject(rt, tri.c);

		return CArrayValue::Construct(rt, std::move(pts));
	};

	for (const auto& tri : tris) {

		values.push_back(ToKVObject(rt,
			std::make_pair(VSL("normal"), ToVec3FromObject(rt, tri.plane)),
			std::make_pair(VSL("origin"), ToVec3FromObject(rt, tri.get_center())),
			std::make_pair(VSL("material"), CStringValue::Construct(rt, tri.material == nullptr ? "N/A" : tri.material)),
			std::make_pair(VSL("has_collision"), CBooleanValue::Construct(rt, tri.has_collision)),
			std::make_pair(VSL("points"), generate_pts(tri))
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
			std::make_pair(VSL("windings"), GenerateWindingArray(ctx->m_pRuntime, asBrush->windings)),
			std::make_pair(VSL("triangles"), GenerateTriangleArray(ctx->m_pRuntime, asBrush->triangles)),
			std::make_pair(VSL("has_collision"), CBooleanValue::Construct(ctx->m_pRuntime, asBrush->brush->has_collision()))

		));

	});

	return CArrayValue::Construct(ctx->m_pRuntime, std::move(brushes));
}



VARJUS_DEFINE_PROPERTY(WorldTerrain, ctx, _this)
{
	IValues terrain;

	std::lock_guard<std::mutex> lock(CClipMap::GetLock());

	CClipMap::ForEach([&](GeometryPtr_t& geom) {

		if (geom->type() != cm_geomtype::terrain)
			return;

		const auto asTerrain = geom->AsTerrain();

		if (!asTerrain || !asTerrain->leaf)
			return;

		const auto contents = CIntValue::Construct(ctx->m_pRuntime, asTerrain->leaf->terrainContents);

		terrain.emplace_back(ToKVObject(ctx->m_pRuntime,
			std::make_pair(VSL("contents"), contents),
			std::make_pair(VSL("triangles"), GenerateTriangleArray(ctx->m_pRuntime, asTerrain->tris)),
			std::make_pair(VSL("material"), CStringValue::Construct(ctx->m_pRuntime, asTerrain->material == nullptr ? "N/A" : asTerrain->material))
		));

		});

	return CArrayValue::Construct(ctx->m_pRuntime, std::move(terrain));
}
VARJUS_DEFINE_PROPERTY(WorldModels, ctx, _this)
{
	IValues models;

	for (const auto i : std::views::iota(0u, gfxWorld->dpvs.smodelCount)) {
		auto model = CM_MakeModel(&gfxWorld->dpvs.smodelDrawInsts[i]);

		models.emplace_back(ToKVObject(ctx->m_pRuntime, 
			std::make_pair(VSL("name"), CStringValue::Construct(ctx->m_pRuntime, model.name)),
			std::make_pair(VSL("modelscale"), CDoubleValue::Construct(ctx->m_pRuntime, model.modelscale)),
			std::make_pair(VSL("origin"), ToVec3FromObject(ctx->m_pRuntime, model.origin)),
			std::make_pair(VSL("angles"), ToVec3FromObject(ctx->m_pRuntime, model.angles))
		));
	}
	return CArrayValue::Construct(ctx->m_pRuntime, std::move(models));
}
VARJUS_DEFINE_PROPERTY(WorldMapName, ctx, _this) {
	return CStringValue::Construct(ctx->m_pRuntime, cm->name);
}
VARJUS_DEFINE_STATIC_OBJECT(WorldObject, receiver) {
	receiver.AddProperty("brushes", WorldBrushes);
	receiver.AddProperty("terrain", WorldTerrain);
	receiver.AddProperty("models", WorldModels);

	receiver.AddProperty("name", WorldMapName);

	receiver.AddMethod("trace", WorldTrace, 5);
	receiver.AddMethod("debug_string", WorldDebugString, 5);

}

Success SC_AddWorldObjects(Varjus::State& state)
{
	if (!state.AddNewStaticObject(VSL("world"), WorldObject))
		return failure;

	return success;
}

#include "cg/cg_local.hpp"
#include "cm_typedefs.hpp"
#include "cm_brush.hpp"
#include "cm_entity.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cm_renderer.hpp"
#include "com/com_vector.hpp"
#include "net/nvar_table.hpp"
#include "r/backend/rb_endscene.hpp"
#include "r/r_utils.hpp"
#include "utils/hook.hpp"
#include "utils/hook.hpp"

void RB_DrawDebug([[maybe_unused]] GfxViewParms* viewParms)
{

	if (R_NoRender())
#if(DEBUG_SUPPORT)
		return hooktable::find<void, GfxViewParms*>(HOOK_PREFIX(__func__))->call(viewParms);
#else
		return;
#endif

#if(DEBUG_SUPPORT)
	hooktable::find<void, GfxViewParms*>(HOOK_PREFIX(__func__))->call(viewParms);
#endif
	

	CM_ShowCollision(viewParms);

}


/***********************************************************************
 >
***********************************************************************/
void CM_DrawCollisionPoly(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest)
{
	RB_DrawPolyInteriors(points, colorFloat, true, depthtest);
}

GfxPointVertex verts[2075];
void CM_DrawCollisionEdges(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest)
{
	const auto numPoints = points.size();
	auto vert_count = 0;
	auto vert_index_prev = numPoints - 1u;


	for (auto i : std::views::iota(0u, numPoints)) {
		vert_count = RB_AddDebugLine(verts, depthtest, points[i].As<vec_t*>(), points[vert_index_prev].As<vec_t*>(), colorFloat, vert_count);
		vert_index_prev = i;
	}

	RB_DrawLines3D(vert_count / 2, 1, verts, depthtest);

}
void CM_ShowCollision([[maybe_unused]] GfxViewParms* viewParms)
{

	if (CClipMap::Size() == NULL && CGentities::Size() == NULL)
		return;

	auto showCollision = NVar_FindMalleableVar<bool>("Show Collision");

	if (!showCollision->Get())
		return;


	cplane_s frustum_planes[6];
	CreateFrustumPlanes(frustum_planes);


	auto brush = showCollision->GetChild("Brush Filter");


	using FloatChild = ImNVar<float>;
	using BoolChild = ImNVar<bool>;

	cm_renderinfo render_info =
	{
		.frustum_planes = frustum_planes,
		.num_planes = 5,
		.draw_dist = showCollision->GetChildAs<FloatChild>("Draw Distance")->Get(),
		.depth_test = showCollision->GetChildAs<BoolChild>("Depth Test")->Get(),
		.as_polygons = showCollision->GetChildAs<BoolChild>("As Polygons")->Get(),
		.only_colliding = showCollision->GetChildAs<BoolChild>("Ignore Noncolliding")->Get(),
		.only_bounces = brush->GetChildAs<BoolChild>("Only Bounces")->Get(),
		.only_elevators = brush->GetChildAs<BoolChild>("Only Elevators")->Get(),
		.alpha = showCollision->GetChildAs<FloatChild>("Transparency")->Get()
	};

	{
		std::unique_lock<std::mutex> lock(CClipMap::GetLock());

		CClipMap::ForEach([&render_info](const GeometryPtr_t& geom) {

			//if (geom->type() == cm_geomtype::brush)
			//	CM_DrawBrushBounds(dynamic_cast<cm_brush*>(geom.get())->brush, render_info.depth_test);

			geom->render(render_info);
			});
	}

	{
		std::unique_lock<std::mutex> lock(CGentities::GetLock());

		CGentities::ForEach([&render_info](const GentityPtr_t& gent) {
			gent->RB_Render3D(render_info); });
	}

}

void CM_DrawBrushBounds(const cbrush_t* brush, bool depthTest)
{
	RB_DrawBoxEdges(brush->mins, brush->maxs, depthTest, vec4_t{1,0,0,1});
}

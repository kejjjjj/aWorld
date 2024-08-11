#include "cm/cm_entity.hpp"
#include "cm/cm_brush.hpp"
#include "cm/cm_terrain.hpp"
#include "cm/cm_renderer.hpp"

#include "g/g_spawn.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"

#include "com/com_vector.hpp"
#include "com/com_channel.hpp"

#include "dvar/dvar.hpp"

#include "r/backend/rb_endscene.hpp"

#include "scr/scr_functions.hpp"

#include <cassert>
#include <ranges>


void CM_LoadAllEntitiesToClipMapWithFilter([[maybe_unused]]const std::string& filter)
{
	if (!Dvar_FindMalleableVar("sv_running")->current.enabled)
		return Com_Printf("^1Unsupported when sv_running is set to 0");

	CGentities::ClearThreadSafe();
	std::unique_lock<std::mutex> lock(CGentities::GetLock());

	const auto filters = CM_TokenizeFilters(filter);

	for (const auto i : std::views::iota(0, level->num_entities)) {

		auto gentity = &level->gentities[i];

		if(!CM_IsMatchingFilter(filters, Scr_GetString(gentity->classname)))
			continue;

		CGentities::Insert(CGameEntity::CreateEntity(gentity));
	}

}

CGameEntity::CGameEntity(gentity_s* const g) : 
	m_pOwner(g), 
	m_vecOrigin((fvec3&)g->r.currentOrigin),
	m_vecAngles((fvec3&)g->r.currentAngles)
{

	assert(m_pOwner != nullptr);

}
CGameEntity::~CGameEntity()
{

}

std::unique_ptr<CGameEntity> CGameEntity::CreateEntity(gentity_s* const g)
{
	if (g->r.bmodel)
		return std::make_unique<CBrushModel>(g);

	return std::make_unique<CGameEntity>(g);
}

bool CGameEntity::IsBrushModel() const noexcept
{
	assert(m_pOwner != nullptr);
	return m_pOwner->r.bmodel;
}
void CGameEntity::RB_Render3D(const cm_renderinfo& info) const
{

	if (m_vecOrigin.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return;

	if (!BoundsInView(m_pOwner->r.absmin, m_pOwner->r.absmax, info.frustum_planes, info.num_planes))
		return;

	const vec4_t CYAN = { 0.f, 1.f, 1.f, info.alpha };
	(info.as_polygons ? RB_DrawBoxPolygons : RB_DrawBoxEdges)(m_pOwner->r.absmin, m_pOwner->r.absmax, info.depth_test, CYAN);
}

/***********************************************************************
 > BRUSHMODELS
***********************************************************************/

CBrushModel::CBrushModel(gentity_s* const g) : CGameEntity(g) 
{
	assert(IsBrushModel());

	const auto leaf = &cm->cmodels[g->s.index.brushmodel].leaf;
	const auto& leafBrushNode = cm->leafbrushNodes[leaf->leafBrushNode];
	const auto numBrushes = leafBrushNode.leafBrushCount;

	//brush
	if (numBrushes > 0) {
		for (const auto brushIndex : std::views::iota(0, numBrushes)) {
			const auto brushWorldIndex = leafBrushNode.data.leaf.brushes[brushIndex];
			if (brushWorldIndex > cm->numBrushes)
				break;

			m_oBrushModels.emplace_back(std::make_unique<CBrush>(g, &cm->brushes[brushWorldIndex]));
		}
		return;
	}

	//terrain
	cm_terrain terrain;
	if (terrain.CM_LeafToGeometry(leaf, { "all " })) {
		m_oBrushModels.emplace_back(std::make_unique<CTerrain>(g, leaf, terrain));
	}


}
CBrushModel::~CBrushModel() = default;

void CBrushModel::RB_Render3D(const cm_renderinfo& info) const
{
	for (auto& bmodel : m_oBrushModels)
		bmodel->RB_Render3D(info);
}

CBrushModel::CIndividualBrushModel::CIndividualBrushModel(gentity_s* const g) : m_pOwner(g) { assert(g != nullptr); }
CBrushModel::CIndividualBrushModel::~CIndividualBrushModel() = default;

void CBrushModel::CIndividualBrushModel::RB_Render3D(const cm_renderinfo& info) const
{
	GetSource().render(info);
}

fvec3 CBrushModel::CIndividualBrushModel::GetCenter() const noexcept
{
	assert(m_pOwner != nullptr);
	return m_pOwner->r.currentOrigin;
}

CBrushModel::CBrush::CBrush(gentity_s* const g, const cbrush_t* const brush) : CIndividualBrushModel(g), m_pSourceBrush(brush)
{
	assert(m_pSourceBrush != nullptr);

	const vec3_t CYAN = { 0.f, 1.f, 1.f };

	//questionable for sure!
	m_oOriginalGeometry = *dynamic_cast<cm_brush*>(&*CM_GetBrushPoints(m_pSourceBrush, CYAN));
	m_oCurrentGeometry = m_oOriginalGeometry;

	OnPositionChanged(g->r.currentOrigin, g->r.currentAngles);


}
CBrushModel::CBrush::~CBrush() = default;

void CBrushModel::CBrush::OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles)
{
	m_oCurrentGeometry = m_oOriginalGeometry;

	for (auto& winding : m_oCurrentGeometry.windings) {
		for (auto& point : winding.points) {
			point = VectorRotate(point + newOrigin, newAngles, m_oCurrentGeometry.origin);
		}
	}


}

const cm_geometry& CBrushModel::CBrush::GetSource() const noexcept
{
	return m_oCurrentGeometry;
}

void CBrushModel::CBrush::RB_Render3D(const cm_renderinfo& info) const
{
	if (m_oCurrentGeometry.origin.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return;

	const auto center = GetCenter();

	if (BoundsInView(center + m_pSourceBrush->mins, center + m_pSourceBrush->maxs, info.frustum_planes, info.num_planes) == false)
		return;

	const auto func = info.as_polygons ? CM_DrawCollisionPoly : CM_DrawCollisionEdges;

	for (const auto& w : m_oCurrentGeometry.windings) {
		const vec4_t color = { w.color[0], w.color[1], w.color[2], info.alpha };
		func(w.points, color, info.depth_test);
	}

}

CBrushModel::CTerrain::CTerrain(gentity_s* const g, const cLeaf_t* const leaf) : CIndividualBrushModel(g), m_pSourceLeaf(leaf) {}
CBrushModel::CTerrain::CTerrain(gentity_s* const g, const cLeaf_t* const leaf, const cm_terrain& terrain) 
	: CIndividualBrushModel(g), m_pSourceLeaf(leaf), m_oOriginalGeometry(terrain), m_oCurrentGeometry(terrain) 
{
	OnPositionChanged(g->r.currentOrigin, g->r.currentAngles);
}

CBrushModel::CTerrain::~CTerrain() = default;

void CBrushModel::CTerrain::OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles)
{
	m_oCurrentGeometry = m_oOriginalGeometry;

	const auto center = GetCenter();

	for (auto& tri : m_oCurrentGeometry.tris) {
		tri.a = VectorRotate(tri.a + newOrigin, newAngles, center);
		tri.b = VectorRotate(tri.b + newOrigin, newAngles, center);
		tri.c = VectorRotate(tri.c + newOrigin, newAngles, center);

	}

}
const cm_geometry& CBrushModel::CTerrain::GetSource() const noexcept
{
	return m_oCurrentGeometry;
}
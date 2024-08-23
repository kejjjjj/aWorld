#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cm_brush.hpp"
#include "cm_typedefs.hpp"
#include "com/com_vector.hpp"

#include <algorithm>
#include <ranges>

void CM_LoadAllBrushWindingsToClipMapWithFilter(const std::string& filter)
{
	std::unique_lock<std::mutex> lock(CClipMap::GetLock());

	CClipMap::ClearAllOfType(cm_geomtype::brush);

	const auto filters = CM_TokenizeFilters(filter);

	if (filters.empty())
		return;

	for(const auto i : std::views::iota(0u, cm->numBrushes)){

		auto materials = CM_GetBrushMaterials(&cm->brushes[i]);

		bool yes = {};

		for (const auto& material : materials) {
			if (CM_IsMatchingFilter(filters, material.c_str())) {
				yes = true;
				break;
			}
		}

		if (!yes)
			continue;

		CM_LoadBrushWindingsToClipMap(&cm->brushes[i]);
	}

}

SimplePlaneIntersection pts[1024];
void CM_LoadBrushWindingsToClipMap(const cbrush_t* brush)
{
	if (!brush)
		return;


	CClipMap::m_pWipGeometry = CM_GetBrushPoints(brush, { 0.f, 1.f, 0.f });
	CClipMap::Insert(CClipMap::m_pWipGeometry);

}
std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col)
{
	if (!brush)
		return nullptr;

	float outPlanes[40][4]{};
	const auto planeCount = BrushToPlanes(brush, outPlanes);
	const auto intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);
	adjacencyWinding_t windings[40]{};

	std::int32_t intersection = 0;
	std::int32_t num_verts = 0;


	CClipMap::m_pWipGeometry = std::make_unique<cm_brush>();
	CClipMap::m_vecWipGeometryColor = poly_col;

	auto c_brush = dynamic_cast<cm_brush*>(CClipMap::m_pWipGeometry.get());

	c_brush->brush = const_cast<cbrush_t*>(brush);
	c_brush->origin = fvec3(brush->mins) + ((fvec3(brush->maxs) - fvec3(brush->mins)) / 2);
	c_brush->originalContents = brush->contents;

	do {
		if (const auto w = BuildBrushdAdjacencyWindingForSide(intersections, pts, outPlanes[intersection], intersection, &windings[intersection])) {
			num_verts += w->numsides;
		}
		++intersection;
	} while (intersection < planeCount);

	c_brush->num_verts = num_verts;
	c_brush->create_corners();

	return std::move(CClipMap::m_pWipGeometry);

}
void CM_BuildAxialPlanes(float(*planes)[6][4], const cbrush_t* brush)
{

	(*planes)[0][0] = -1.0;
	(*planes)[0][1] = 0.0;
	(*planes)[0][2] = 0.0;
	(*planes)[0][3] = -brush->mins[0];
	(*planes)[1][0] = 1.0;
	(*planes)[1][1] = 0.0;
	(*planes)[1][2] = 0.0;
	(*planes)[1][3] = brush->maxs[0];
	(*planes)[2][0] = 0.0;
	(*planes)[2][2] = 0.0;
	(*planes)[2][1] = -1.0;
	(*planes)[2][3] = -brush->mins[1];
	(*planes)[3][0] = 0.0;
	(*planes)[3][2] = 0.0;
	(*planes)[3][1] = 1.0;
	(*planes)[3][3] = brush->maxs[1];
	(*planes)[4][0] = 0.0;
	(*planes)[4][1] = 0.0;
	(*planes)[4][2] = -1.0;
	(*planes)[4][3] = -brush->mins[2];
	(*planes)[5][0] = 0.0;
	(*planes)[5][1] = 0.0;
	(*planes)[5][2] = 1.0;
	(*planes)[5][3] = brush->maxs[2];
}
void CM_GetPlaneVec4Form(const cbrushside_t* sides, const float(*axialPlanes)[4], unsigned int index, float* expandedPlane)
{
	if (index >= 6u) {
		cplane_s* plane = sides[index - 6u].plane;

		expandedPlane[0] = plane->normal[0];
		expandedPlane[1] = plane->normal[1];
		expandedPlane[2] = plane->normal[2];
		expandedPlane[3] = plane->dist;
		return;
	}

	const float* plane = axialPlanes[index];

	*expandedPlane = plane[0];
	expandedPlane[1] = plane[1];
	expandedPlane[2] = plane[2];
	expandedPlane[3] = plane[3];

}
int GetPlaneIntersections(const float** planes, int planeCount, SimplePlaneIntersection* OutPts)
{
	int r = 0;
	__asm
	{
		push OutPts;
		push planeCount;
		push planes;
		mov esi, 0x58FB00;
		call esi;
		add esp, 12;
		mov r, eax;
	}

	return r;
}
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4])
{
	float planes[6][4]{};
	CM_BuildAxialPlanes((float(*)[6][4])planes, brush);
	auto i = 0u;
	do {
		CM_GetPlaneVec4Form(brush->sides, planes, i, outPlanes[i]);

	} while (++i < brush->numsides + 6);

	return i;
}
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* _pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding)
{
	adjacencyWinding_t* r = 0;

	__asm
	{
		mov edx, ptCount;
		mov ecx, _pts;
		push optionalOutWinding;
		push planeIndex;
		push sideNormal;
		mov esi, 0x57D500;
		call esi;
		add esp, 12;
		mov r, eax;
	}

	return r;
}
void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2)
{
	auto brush = dynamic_cast<cm_brush*>(CClipMap::m_pWipGeometry.get());
	cm_triangle tri;
	std::vector<fvec3> winding_points;

	tri.a = &points[i2];
	tri.b = &points[i1];
	tri.c = &points[i0];

	PlaneFromPointsASM(tri.plane, tri.a, tri.b, tri.c);

	if (DotProduct(tri.plane, normal) <= 0.f) {
		std::swap(tri.a, tri.c);
	}

	tri.material = CM_MaterialForNormal(brush->brush, normal);
	brush->triangles.emplace_back(tri);

	for(const auto winding : std::views::iota(0, w->numsides)){
		winding_points.emplace_back(fvec3{ &points[winding * 3] });
	}

	brush->windings.emplace_back(cm_winding{ winding_points, normal, CClipMap::m_vecWipGeometryColor });

}
__declspec(naked) void __brush::__asm_adjacency_winding()
{
	static constexpr unsigned int dst = 0x57D87C;

	__asm
	{
		mov eax, [esp + 6038h + -601Ch]; //i2
		lea edx, [eax + eax * 2];
		mov eax, [esp + 6038h + -6018h]; //i1
		lea ecx, [eax + eax * 2];
		mov eax, [esp + 6038h + -6024h]; //i0
		lea esi, [eax + eax * 2];

		push edx;
		push ecx;
		push esi;
		push ebp; //normal
		lea eax, [esp + 00003048h];
		push eax;
		push ebx;
		call adjacency_winding;
		add esp, 24;

		fld dword ptr[ebp + 04h];
		fmul dword ptr[esp + 2Ch];
		fld dword ptr[ebp + 00h];
		fmul dword ptr[esp + 28h];
		faddp st(1), st;
		jmp dst;
	}
		
}
char* CM_MaterialForNormal(const cbrush_t* target, const fvec3& normals)
{
	//non-axial!
	for (unsigned int i = 0; i < target->numsides; i++) {

		cbrushside_t* side = &target->sides[i];

		if (normals == side->plane->normal)
			return cm->materials[side->materialNum].material;
	}


	short mtl = -1;

	if (normals.z == 1.f)
		mtl = target->axialMaterialNum[1][2];
	else if (normals.z == -1.f)
		mtl = target->axialMaterialNum[0][2];

	if (normals.x == 1)
		mtl = target->axialMaterialNum[1][0];
	else if (normals.x == -1)
		mtl = target->axialMaterialNum[0][0];

	if (normals.y == 1.f)
		mtl = target->axialMaterialNum[1][1];
	else if (normals.y == -1.f)
		mtl = target->axialMaterialNum[0][1];

	if (mtl >= 0)
		return cm->materials[mtl].material;


	return (char*)"caulk";

}
bool CM_BrushHasCollision(const cbrush_t* brush)
{
	return (brush->contents & MASK_PLAYERSOLID) != 0;
}

//mp_void_v2 made me have to make this so high
float outPlanes[128][4]{};

std::vector<std::string> CM_GetBrushMaterials(const cbrush_t* brush)
{
	std::vector<std::string> result;

	const auto planeCount = BrushToPlanes(brush, outPlanes);
	[[maybe_unused]] const auto intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);

	for (const auto i : std::views::iota(0, planeCount))
	{
		const fvec3 plane = outPlanes[i];

		if (const auto mtl = CM_MaterialForNormal(brush, plane)) {
			result.emplace_back(mtl);
		}
		

	}

	return result;
}

bool CM_BrushInView(const cbrush_t* brush, struct cplane_s* frustumPlanes, int numPlanes)
{
	if (numPlanes <= 0)
		return 1;

	cplane_s* plane = frustumPlanes;
	int idx = 0;
	while ((BoxOnPlaneSide(brush->mins, brush->maxs, plane) & 1) != 0) {
		++plane;
		++idx;

		if (idx >= numPlanes)
			return 1;
	}

	return 0;
}

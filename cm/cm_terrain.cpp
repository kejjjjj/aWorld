#include "cm_terrain.hpp"
#include "cm_typedefs.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include <unordered_map>
#include <ranges>
#include <com/com_vector.hpp>

void CM_LoadAllTerrainToClipMapWithFilter(const std::string& filter)
{
	CM_DiscoverTerrain({ filter });
}

bool CM_AabbTreeHasCollision(const CollisionAabbTree* tree)
{
	const dmaterial_t* materialInfo = &cm->materials[tree->materialIndex];
	return (materialInfo->contentFlags & MASK_PLAYERSOLID) != 0;
}

std::unordered_map<CollisionPartition*, CollisionPartition*> discovered_partitions;


bool CM_DiscoverTerrain([[maybe_unused]]const std::unordered_set<std::string>& filters)
{

	try {
		discovered_partitions.clear();
		for (const auto i : std::views::iota(0u, cm->numLeafs)) {
			cm_terrain terrain;

			if(terrain.CM_LeafToGeometry(&cm->leafs[i], filters))
				CClipMap::Insert(std::make_unique<cm_terrain>(terrain));

		}
		return true;
	}
	catch (...) {
		return false;
	}
}

void cm_terrain::CM_AdvanceAabbTree(const CollisionAabbTree* aabbTree, const std::unordered_set<std::string>& filters, const float* color)
{
	if (aabbTree->childCount) {
		auto child = &cm->aabbTrees[aabbTree->u.firstChildIndex];
		for ([[maybe_unused]]const auto i : std::views::iota(0u, aabbTree->childCount)) {
			CM_AdvanceAabbTree(child, filters, color);
			++child;
		}
		return;
	}

	const auto mat = cm->materials[aabbTree->materialIndex].material;
	if (CM_IsMatchingFilter(filters, mat) == false) {
		return;
	}

	material = mat;


	CollisionAabbTreeIndex fChild = aabbTree->u;
	CollisionPartition* partition = &cm->partitions[fChild.partitionIndex];

	if (discovered_partitions.find(partition) != discovered_partitions.end())
		return;

	discovered_partitions[partition] = partition;

	auto firstTri = partition->firstTri;
	if (firstTri < firstTri + partition->triCount)
	{

		auto triIndice = 3 * firstTri;

		do {
			cm_triangle tri;
			tri.has_collision = CM_AabbTreeHasCollision(aabbTree);
			tri.a = cm->verts[cm->triIndices[triIndice]];
			tri.b = cm->verts[cm->triIndices[triIndice + 1]];
			tri.c = cm->verts[cm->triIndices[triIndice + 2]];

			PlaneFromPointsASM(tri.plane, tri.a, tri.b, tri.c);

			tri.color[0] = color[0];
			tri.color[1] = color[1];
			tri.color[2] = color[2];
			tri.color[3] = 0.3f;

			tris.emplace_back(tri);

			++firstTri;
			triIndice += 3;

		} while (firstTri < partition->firstTri + partition->triCount);

	}
}

bool cm_terrain::CM_LeafToGeometry(const cLeaf_t* aleaf, const std::unordered_set<std::string>& filters)
{
	if (!aleaf || !aleaf->collAabbCount)
		return 0;

	std::int32_t aabbIdx = 0;
	
	leaf = aleaf;

	do {
		const CollisionAabbTree* aabb = &cm->aabbTrees[aabbIdx + leaf->firstCollAabbIndex];
		CM_AdvanceAabbTree(aabb, filters, vec4_t{ 0,0.1f,1.f, 0.8f });
		++aabbIdx;
	} while (aabbIdx < leaf->collAabbCount);


	return true;
}

bool CM_TriangleInView(const cm_triangle* tris, struct cplane_s* frustumPlanes, int numPlanes)
{
	if (numPlanes <= 0)
		return 1;
	cplane_s* plane = frustumPlanes;
	int idx = 0;
	while ((BoxOnPlaneSide(tris->get_mins(), tris->get_maxs(), plane) & 1) != 0) {
		++plane;
		++idx;

		if (idx >= numPlanes)
			return 1;
	}

	return 0;
}
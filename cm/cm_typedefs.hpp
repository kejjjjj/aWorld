#pragma once

#include <unordered_set>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <mutex>

#include "global_macros.hpp"
#include "utils/typedefs.hpp"


struct CollisionAabbTree;
struct adjacencyWinding_t;

class CGameEntity;

struct sc_winding_t
{
	std::vector<fvec3> points;
	bool is_bounce = false;
	bool is_elevator = false;
	fvec3 normals;
	vec4_t color;

};
enum class showCollisionType
{
	DISABLED,
	BRUSHES,
	TERRAIN,
	BOTH
};
enum class polyType
{
	EDGES,
	POLYS,
};

enum class cm_geomtype
{
	brush,
	terrain,
	model
};

struct cplane_s;
struct cbrush_t;
struct cLeaf_t;

struct cm_triangle
{
	fvec3 a;
	fvec3 b;
	fvec3 c;



	vec4_t plane = {};
	vec4_t color = {};
	char* material = {};
	bool has_collision = false;
	//bool edge_walkable = true;

	fvec3 get_mins() const noexcept {
		fvec3 lowest = FLT_MAX;

		lowest.x = a.x;
		if (b.x < lowest.x) lowest.x = b.x;
		if (c.x < lowest.x) lowest.x = c.x;

		lowest.y = a.y;
		if (b.y < lowest.y) lowest.y = b.y;
		if (c.y < lowest.y) lowest.y = c.y;

		lowest.z = a.z;
		if (b.z < lowest.z) lowest.z = b.z;
		if (c.z < lowest.z) lowest.z = c.z;

		return lowest;

	}
	fvec3 get_maxs() const noexcept {
		fvec3 highest = -FLT_MAX;

		highest.x = a.x;
		if (b.x > highest.x) highest.x = b.x;
		if (c.x > highest.x) highest.x = c.x;

		highest.y = a.y;
		if (b.y > highest.y) highest.y = b.y;
		if (c.y > highest.y) highest.y = c.y;

		highest.z = a.z;
		if (b.z > highest.z) highest.z = b.z;
		if (c.z > highest.z) highest.z = c.z;

		return highest;

	}
};
struct cm_winding
{
	cm_winding() = default;
	cm_winding(const std::vector<fvec3>& p, const fvec3& normal, const fvec3& col);

	std::vector<fvec3> points;
	fvec3 mins;
	fvec3 maxs;
	fvec3 normals;
	vec4_t color;
	bool is_bounce = {};
	bool is_elevator = {};

private:

	inline fvec3 get_mins() const noexcept
	{
		std::vector<float> x, y, z;

		for (auto& p : points) {
			x.push_back(p.x);
			y.push_back(p.y);
			z.push_back(p.z);

		}

		const float _x = *std::min_element(x.begin(), x.end());
		const float _y = *std::min_element(y.begin(), y.end());
		const float _z = *std::min_element(z.begin(), z.end());

		return { _x, _y, _z };
	}
	inline fvec3 get_maxs() const noexcept
	{
		std::vector<float> x, y, z;

		for (auto& p : points) {
			x.push_back(p.x);
			y.push_back(p.y);
			z.push_back(p.z);

		}

		const float _x = *std::max_element(x.begin(), x.end());
		const float _y = *std::max_element(y.begin(), y.end());
		const float _z = *std::max_element(z.begin(), z.end());

		return { _x, _y, _z };
	}
};

struct cm_renderinfo
{
	cplane_s* frustum_planes = {};
	int num_planes = {};
	float draw_dist = {};
	bool depth_test = {};
	bool as_polygons = {};
	bool only_colliding = {};
	bool only_bounces = {};
	int only_elevators = {};
	float alpha = 0.7f;


};

class brushModelEntity;

struct cm_geometry
{
	virtual ~cm_geometry() = default;
	virtual void render(const cm_renderinfo& info) const = 0;
	virtual constexpr cm_geomtype type() const noexcept = 0;
	virtual int map_export(std::stringstream& o, int index) = 0;
	virtual void render2d() = 0;
	fvec3 origin;
	bool has_collisions = {};
	int originalContents = {};
	int num_verts = {};
	brushModelEntity* brushmodel = 0;

};


struct cm_brush : public cm_geometry
{
	friend class CBrushModel;

	~cm_brush() = default;

	cm_geomtype type() const noexcept override { return cm_geomtype::brush; }

	void create_corners();
	void render(const cm_renderinfo& info) const override;
	void render2d() override {}

	friend void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2);
	friend std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col);

	cbrush_t* brush = {};

protected:
	int map_export(std::stringstream& o, int index) override;
private:

	std::vector<cm_winding> windings; //used for rendering
	std::vector<cm_triangle> triangles; //used for exporting

	struct ele_corner {
		fvec3 mins;
		fvec3 maxs;
	};

	fvec3 mins;

	std::vector<const cm_winding*> corners;

};

struct cm_terrain : public cm_geometry
{
	friend class CBrushModel;

	~cm_terrain() = default;

	constexpr cm_geomtype type() const noexcept override { return cm_geomtype::terrain; }

	void render(const cm_renderinfo& info) const override;
	void render2d() override;

	bool CM_LeafToGeometry(const cLeaf_t* leaf, const std::unordered_set<std::string>& filters);
	//void sort_tree();

protected:
	int map_export(std::stringstream& o, int index) override;
	int map_export_triangle(std::stringstream& o, const cm_triangle& tri, int index) const;

private:
	const cLeaf_t* leaf = 0;
	std::vector<cm_triangle> tris;
	char* material = {};

	void CM_AdvanceAabbTree(const CollisionAabbTree* aabbTree, const std::unordered_set<std::string>& filters, const float* color);

};

struct cm_model : public cm_geometry
{
	cm_model(const char* _name, const fvec3& _origin, const fvec3& _angles, float _modelscale) : 
		name(_name), origin(_origin), angles(_angles), modelscale(_modelscale) {}
	~cm_model() = default;

	void render([[maybe_unused]] const cm_renderinfo& info) const override {};
	void render2d() override {};

	constexpr cm_geomtype type() const noexcept override { return cm_geomtype::model; }

protected:
	int map_export(std::stringstream& o, int index) override;
private:

	const char* name = {};
	fvec3 origin;
	fvec3 angles;
	float modelscale = {};
};

void CM_LoadMap();
bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, const char* material);


using GeometryPtr_t = std::unique_ptr<cm_geometry>;
using LevelGeometry_t = std::vector<GeometryPtr_t>;

using GentityPtr_t = std::unique_ptr<CGameEntity>;
using LevelGentities_t = std::vector<std::unique_ptr<CGameEntity>>;


class CClipMap
{
	NONCOPYABLE(CClipMap);

public:

	friend void CM_LoadBrushWindingsToClipMap(const cbrush_t* brush);
	friend std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col);
	friend void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2);

	static void Insert(GeometryPtr_t& geom);
	static void Insert(GeometryPtr_t&& geom);
	static void ClearAllOfType(const cm_geomtype t);
	static auto GetAllOfType(const cm_geomtype t);

	static void ClearAllOfTypeThreadSafe(const cm_geomtype t){ std::unique_lock<std::mutex> lock(mtx); ClearAllOfType(t); }

	//NOT thread safe
	static void RemoveBrushCollisionsBasedOnVolume(const float volume);

	//NOT thread safe
	static void RestoreBrushCollisions();

	static auto begin() { return m_pLevelGeometry.begin(); }
	static auto end() { return m_pLevelGeometry.end(); }
	static size_t Size() { return m_pLevelGeometry.size(); }
	static void Clear() { CClipMap::RestoreBrushCollisions(); m_pLevelGeometry.clear(); m_pWipGeometry.reset(); }
	static void ClearThreadSafe() { std::unique_lock<std::mutex> lock(mtx); Clear(); }

	inline static auto& GetLock() { return mtx; }

	template<typename Func>
	static void ForEach(Func func) {

		for (auto& geo : m_pLevelGeometry)
			func(geo);

	}

private:
	static std::unique_ptr<cm_geometry> m_pWipGeometry;
	static fvec3 m_vecWipGeometryColor;
	static LevelGeometry_t m_pLevelGeometry;

	static std::mutex mtx;
};

class CGentities
{
public:

	static void CM_LoadAllEntitiesToClipMapWithFilter(const std::string& filter);

	static void Insert(GentityPtr_t& geom);
	static void Insert(GentityPtr_t&& geom);

	static auto begin();
	static auto end();
	static size_t Size();
	static void Clear();
	static void ClearThreadSafe();

	inline static auto& GetLock() { return mtx; }

	template<typename Func>
	static void ForEach(Func func) {

		for (auto& geo : m_pLevelGentities)
			func(geo);

	}

private:
	static LevelGentities_t m_pLevelGentities;
	static std::mutex mtx;

};

void CM_LoadMap();
std::unordered_set<std::string> CM_TokenizeFilters(const std::string& filters);

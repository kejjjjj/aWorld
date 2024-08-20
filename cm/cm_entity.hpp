#pragma once

#include "cm_typedefs.hpp"

#include "utils/typedefs.hpp"

#include <memory>

struct gentity_s;
struct GfxPointVertex;

struct CGentityConnection
{
	gentity_s* const m_pConnection{};
	fvec3 start, end;
};

class CGameEntity
{
public:

	CGameEntity(gentity_s* const g);
	virtual ~CGameEntity();

	//creates the appropriate derived class
	[[nodiscard]] static std::unique_ptr<CGameEntity> CreateEntity(gentity_s* const g);

	virtual void RB_Render3D(const cm_renderinfo& info) const;
	virtual void CG_Render2D(float drawDist) const;

	void GenerateConnections(const LevelGentities_t& gentities);

protected:

	void RB_RenderConnections(const cm_renderinfo& info) const;

	[[nodiscard]] bool IsBrushModel() const noexcept;

	fvec3& m_vecOrigin;
	fvec3& m_vecAngles;

	mutable fvec3 m_vecOldOrigin;
	mutable fvec3 m_vecOldAngles;

private:
	void ParseEntityFields();

	gentity_s* const m_pOwner{};

	std::unordered_map<std::string, std::string> m_oEntityFields;
	std::vector<CGentityConnection> m_oGentityConnections;
	mutable std::vector<GfxPointVertex> m_oGentityConnectionVertices;
};

class CBrushModel : public CGameEntity
{
public:
	CBrushModel(gentity_s* const g);
	~CBrushModel();

	void RB_Render3D(const cm_renderinfo& info) const override;


	struct CIndividualBrushModel
	{
		CIndividualBrushModel(gentity_s* const g);
		virtual ~CIndividualBrushModel();

		virtual void RB_Render3D(const cm_renderinfo& info) const;

		[[nodiscard]] virtual const cm_geometry& GetSource() const noexcept = 0;
		virtual void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) = 0;

	protected:
		[[nodiscard]] virtual fvec3 GetCenter() const noexcept;

	//private:
		gentity_s* const m_pOwner{};
	};

	struct CBrush : public CIndividualBrushModel
	{
		CBrush(gentity_s* const g, const cbrush_t* const brush);
		~CBrush();

		void RB_Render3D(const cm_renderinfo& info) const override;


		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

	private:
		cm_brush m_oOriginalGeometry;
		cm_brush m_oCurrentGeometry;
		const cbrush_t* const m_pSourceBrush = {};
	};

	struct CTerrain : public CIndividualBrushModel
	{
		CTerrain(gentity_s* const g, const cLeaf_t* const leaf, const cm_terrain& terrain);
		CTerrain(gentity_s* const g, const cLeaf_t* const leaf);
		~CTerrain();

		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

	private:
		cm_terrain m_oOriginalGeometry;
		cm_terrain m_oCurrentGeometry;
		const cLeaf_t* const m_pSourceLeaf = {};
	};

private:
	std::vector<std::unique_ptr<CIndividualBrushModel>> m_oBrushModels;
};



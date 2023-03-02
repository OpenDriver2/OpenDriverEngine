#ifndef REGION_D1
#define REGION_D1
#include "regions.h"

//----------------------------------------------------------------------------------
// DRIVER 1
//----------------------------------------------------------------------------------

#define ROAD_MAP_REGION_CELLS	3600

class CDriver1LevelRegion;
class CDriver1LevelMap;


struct CELL_ITERATOR_D1
{
	CELL_ITERATOR_CACHE*	cache{ nullptr };
	CDriver1LevelRegion*	region;
	CELL_DATA_D1*			pcd;
	CELL_OBJECT*			pco;
};


// Driver 1 region
class CDriver1LevelRegion : public CBaseLevelRegion
{
	friend class CDriver1LevelMap;
public:
	void					FreeAll() override;
	void					LoadRegionData(const SPOOL_CONTEXT& ctx) override;

	// cell iterator
	CELL_OBJECT*			StartIterator(CELL_ITERATOR_D1* iterator, int cellNumber) const;

protected:
	void					LoadRoadHeightMapData(IVirtualStream* pFile);
	void					LoadRoadCellsData(IVirtualStream* pFile);

	CELL_DATA_D1*			m_cells{ nullptr };				// cell data that holding information about cell pointers. 3D world seeks cells first here
	uint*					m_roadMap{ nullptr };
	ushort*					m_surfaceRoads{ nullptr };
};


// Driver 1 level map
class CDriver1LevelMap : public CBaseLevelMap
{
	friend class CDriver1LevelRegion;
public:
	void					FreeAll() override;

	//----------------------------------------

	void 					LoadMapLump(IVirtualStream* pFile) override;
	void					LoadSpoolInfoLump(IVirtualStream* pFile) override;

	void					LoadRoadsLump(IVirtualStream* pFile);
	void					LoadJunctionsLump(IVirtualStream* pFile);
	void					LoadRoadBoundsLump(IVirtualStream* pFile);
	void					LoadJuncBoundsLump(IVirtualStream* pFile);

	void					LoadRoadMapLump(IVirtualStream* pFile); // or NewProcessRoadMapLump in D1 & D2
	void					LoadRoadSurfaceLump(IVirtualStream* pFile, int size);

	bool					SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) override;
	bool					SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) override;

	CBaseLevelRegion*		GetRegion(const XZPAIR& cell) const override;
	CBaseLevelRegion*		GetRegion(int regionIdx) const override;

	void					FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const override;
	int						GetSurfaceIndex(const VECTOR_NOPAD& position) const override;

	//----------------------------------------
	// cell iterator
	CELL_OBJECT*			GetFirstCop(CELL_ITERATOR_D1* iterator, const XZPAIR& cell) const;
	CELL_OBJECT*			GetNextCop(CELL_ITERATOR_D1* iterator) const;

	//----------------------------------------
	// road map stuff
	bool						GetRoadInfo(ROUTE_DATA& outData, const VECTOR_NOPAD& position) const;

	int							GetNumRoads() const { return m_numRoads; }
	int							GetNumJunctions() const { return m_numJunctions; }

	const ROAD_MAP_LUMP_DATA&	GetRoadMapLumpData() const { return m_roadMapLumpData; }

	DRIVER1_ROAD*				GetRoad(int idx) const { return &m_roads[idx]; }
	DRIVER1_ROADBOUNDS*			GetRoadBounds(int idx) const { return &m_roadBounds[idx]; }
	DRIVER1_JUNCTION*			GetJunction(int idx) const { return &m_junctions[idx]; }
	XYPAIR*						GetJunctionBounds(int idx) const { return &m_junctionBounds[idx]; }

protected:

	void					GetSurfaceLocalCoords(const VECTOR_NOPAD& position, int& px, int& py) const;

	ROAD_MAP_LUMP_DATA		m_roadMapLumpData;

	CDriver1LevelRegion*	m_regions{ nullptr };					// map of regions
	SURFACEINFO*			m_surfacePtrs[900];
	char*					m_surfaceData{ nullptr };

	DRIVER1_ROAD*			m_roads{ nullptr };
	DRIVER1_ROADBOUNDS*		m_roadBounds{ nullptr };
	DRIVER1_JUNCTION*		m_junctions{ nullptr };
	XYPAIR*					m_junctionBounds{ nullptr };

	int						m_numRoads{ 0 };
	int						m_numJunctions{ 0 };
};


#endif
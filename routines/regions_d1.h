#ifndef REGION_D1
#define REGION_D1
#include "regions.h"

//----------------------------------------------------------------------------------
// DRIVER 1
//----------------------------------------------------------------------------------

class CDriver1LevelRegion;
class CDriver1LevelMap;


struct CELL_ITERATOR_D1
{
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

	CELL_DATA_D1*			m_cells{ nullptr };				// cell data that holding information about cell pointers. 3D world seeks cells first here
	uint*					m_roadMap{ nullptr };
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

	void					LoadRoadMapLump(IVirtualStream* pFile); // or NewProcessRoadMapLump in D1 & D2
	void					LoadRoadSurfaceLump(IVirtualStream* pFile, int size);

	void					SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) override;
	void					SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) override;

	CBaseLevelRegion*		GetRegion(const XZPAIR& cell) const override;
	CBaseLevelRegion*		GetRegion(int regionIdx) const override;

	int						MapHeight(const VECTOR_NOPAD& position) const override;
	int						FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const override;

	//----------------------------------------
	// cell iterator
	CELL_OBJECT*			GetFirstCop(CELL_ITERATOR_D1* iterator, const XZPAIR& cell) const;
	CELL_OBJECT*			GetNextCop(CELL_ITERATOR_D1* iterator) const;

	//----------------------------------------
	// road map stuff
	bool					GetRoadInfo(ROUTE_DATA& outData, const VECTOR_NOPAD& position) const;

protected:

	ROAD_MAP_LUMP_DATA		m_roadMapLumpData;

	CDriver1LevelRegion*	m_regions{ nullptr };					// map of regions
	SURFACEINFO*			m_surfacePtrs[900];
	char*					m_surfaceData{ nullptr };
};


#endif
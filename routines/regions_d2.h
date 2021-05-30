#ifndef REGIONS_D2
#define REGIONS_D2

#include "regions.h"

//----------------------------------------------------------------------------------
// DRIVER 2 regions and map
//----------------------------------------------------------------------------------

class CDriver2LevelRegion;
class CDriver2LevelMap;

struct CELL_ITERATOR_D2
{	
	CDriver2LevelRegion*	region;
	CELL_DATA*				pcd;
	PACKED_CELL_OBJECT*		ppco;
	XZPAIR					nearCell;
	int						listType;
};

typedef short* (*sdBspCallback)(sdNode* node, XZPAIR* pos);

// standard BSP walker
short* SdGetBSP(sdNode* node, XZPAIR* pos);

// Driver 2 region
class CDriver2LevelRegion : public CBaseLevelRegion
{
	friend class CDriver2LevelMap;
public:
	void					FreeAll() override;
	void					LoadRegionData(const SPOOL_CONTEXT& ctx) override;

	PACKED_CELL_OBJECT*		GetCellObject(int num) const;
	CELL_DATA*				GetCellData(int num) const;

	// cell iterator
	PACKED_CELL_OBJECT*		StartIterator(CELL_ITERATOR_D2* iterator, int cellNumber) const;

	sdPlane*				SdGetCell(const VECTOR_NOPAD& position, int& sdLevel, sdBspCallback bspWalker) const;

protected:

	void					ReadHeightmapData(const SPOOL_CONTEXT& ctx);

	CELL_DATA*				m_cells{ nullptr };				// cell data that holding information about cell pointers. 3D world seeks cells first here
	PACKED_CELL_OBJECT*		m_cellObjects{ nullptr };		// cell objects that represents objects placed in the world

	char*					m_pvsData{ nullptr };

	sdPlane*				m_planeData{ nullptr };
	short*					m_bspData{ nullptr };
	sdNode*					m_nodeData{ nullptr };
	short*					m_surfaceData{ nullptr };
};

// Driver 2 level map
class CDriver2LevelMap : public CBaseLevelMap
{
	friend class CDriver2LevelRegion;
public:
	void					FreeAll() override;
	
	//----------------------------------------

	void 					LoadMapLump(IVirtualStream* pFile) override;
	void					LoadSpoolInfoLump(IVirtualStream* pFile) override;

	void					LoadStraightsLump(IVirtualStream* pFile);
	void					LoadCurvesLump(IVirtualStream* pFile);
	void					LoadJunctionsLump(IVirtualStream* pFile, bool oldFormat);

	//----------------------------------------

	void					SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) override;
	void					SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) override;

	CBaseLevelRegion*		GetRegion(const XZPAIR& cell) const override;
	CBaseLevelRegion*		GetRegion(int regionIdx) const override;

	int						MapHeight(const VECTOR_NOPAD& position) const override;
	
	//----------------------------------------
	// cell iterator
	PACKED_CELL_OBJECT*		GetFirstPackedCop(CELL_ITERATOR_D2* iterator, const XZPAIR& cell) const;
	PACKED_CELL_OBJECT*		GetNextPackedCop(CELL_ITERATOR_D2* iterator) const;
	static bool				UnpackCellObject(CELL_OBJECT& co, PACKED_CELL_OBJECT* pco, const XZPAIR& nearCell);

protected:
	
	// Driver 2 - specific
	CDriver2LevelRegion*	m_regions{ nullptr };					// map of regions
	PACKED_CELL_OBJECT*		m_straddlers { nullptr };				// cell objects between regions

	DRIVER2_STRAIGHT*		m_straights{ nullptr };
	DRIVER2_CURVE*			m_curves{ nullptr };
	DRIVER2_JUNCTION*		m_junctions{ nullptr };

	int						m_numStraights{ 0 };
	int						m_numCurves{ 0 };
	int						m_numJunctions{ 0 };
};


#endif
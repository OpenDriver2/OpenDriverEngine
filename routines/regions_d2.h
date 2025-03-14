#pragma once
#include "regions.h"

//----------------------------------------------------------------------------------
// DRIVER 2 roads
//----------------------------------------------------------------------------------

#define IS_DRIVEABLE_SURFACE(surfid)		(((surfid) & 0xFFFFE000) != 0xFFFFE000)		// is any road surface
#define IS_ROAD_SURFACE(surfid)				(IS_STRAIGHT_SURFACE(surfid) || IS_CURVED_SURFACE(surfid))

// those macros can be applied to straights and junctions
#define ROAD_LANES_COUNT(rd)				((uint)(rd)->NumLanes & 0xF)					// lane count
#define ROAD_WIDTH_IN_LANES(rd)				(ROAD_LANES_COUNT(rd) * 2)						// width in lanes
#define ROAD_IS_AI_LANE(rd, lane)			((uint8)(rd)->AILanes >> ((lane) / 2) & 1U)		// lane AI driveable flag
#define ROAD_IS_LEFTMOST_LANE_PARKING(rd)	(((uint8)(rd)->NumLanes & 0x40) != 0)			// allows parking on leftmost lane
#define ROAD_IS_RIGHTMOST_LANE_PARKING(rd)	(((uint8)(rd)->NumLanes & 0x80) != 0)			// allows parking on rightmost lane
#define ROAD_LANE_DIRECTION_BIT(rd, lane)	((uint8)(rd)->LaneDirs >> ((lane) / 2) & 1U)	// direction bit
#define ROAD_SPEED_LIMIT(rd)				(((uint8)(rd)->NumLanes >> 4) & 3)				// speed limit id
#define ROAD_HAS_FAST_LANES(rd)				(((uint8)(rd)->NumLanes >> 6) & 1)				// & 0x20; in fact speed limit check too

#define ROAD_LANE_DIR(rd, lane) \
	(((uint8)(rd)->LaneDirs == 0xFF && (rd)->NumLanes == 1) ? ((lane) & 1) : ROAD_LANE_DIRECTION_BIT(rd, lane))

#define ROAD_IS_PARKING_ALLOWED_AT(rd, lane)\
	((ROAD_IS_LEFTMOST_LANE_PARKING(rd) && (lane) == 0) || (ROAD_IS_RIGHTMOST_LANE_PARKING(rd) && (lane) == ROAD_WIDTH_IN_LANES(rd) - 1))

// Driver 2 road data.
struct DRIVER2_ROAD_INFO
{
	int surfId;
	short(*ConnectIdx)[4];

	union
	{
		struct
		{
			int8 NumLanes;
			int8 LaneDirs;
			int8 AILanes;
		};
		uint flags;
	};

	DRIVER2_STRAIGHT* straight{ nullptr };
	DRIVER2_CURVE* curve{ nullptr };
	DRIVER2_JUNCTION* junction{ nullptr };
};


//----------------------------------------------------------------------------------
// DRIVER 2 regions and map
//----------------------------------------------------------------------------------

class CDriver2LevelRegion;
class CDriver2LevelMap;

struct CELL_ITERATOR_D2
{	
	CELL_ITERATOR_CACHE*	cache{ nullptr };
	CDriver2LevelRegion*	region;
	CELL_DATA*				pcd;
	PACKED_CELL_OBJECT*		ppco;
	CELL_OBJECT*			co;
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

	PACKED_CELL_OBJECT*		GetPackedCellObject(int num) const;
	CELL_DATA*				GetCellData(int num) const;

	// cell iterator
	PACKED_CELL_OBJECT*		StartIterator(CELL_ITERATOR_D2* iterator, int cellNumber) const;

	sdPlane*				SdGetCell(const VECTOR_NOPAD& position, int& sdLevel, sdBspCallback bspWalker) const;

	// returns road ID based on the heightmap data. Stores surface height in position.vy
	int						RoadInCell(VECTOR_NOPAD& position) const;

protected:

	void					UnpackAllCellObjects();

	void					ReadHeightmapData(const SPOOL_CONTEXT& ctx);

	CELL_DATA*				m_cells{ nullptr };					// cell data that holding information about cell pointers. 3D world seeks cells first here
	PACKED_CELL_OBJECT*		m_packedCellObjects{ nullptr };		// cell objects that represents objects placed in the world

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

	bool					SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) override;
	bool					SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) override;

	CBaseLevelRegion*		GetRegion(const XZPAIR& cell) const override;
	CBaseLevelRegion*		GetRegion(int regionIdx) const override;

	void					FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const override;
	int						GetSurfaceIndex(const VECTOR_NOPAD& position) const override;

	int						GetRoadIndex(VECTOR_NOPAD& position) const;

	// any of road surface
	bool					GetSurfaceRoadInfo(DRIVER2_ROAD_INFO& outRoadInfo, int surfId) const;

	// specific road structures only
	DRIVER2_STRAIGHT*		GetStraight(int index) const;
	DRIVER2_CURVE*			GetCurve(int index) const;
	DRIVER2_JUNCTION*		GetJunction(int index) const;

	int						GetNumStraights() const;
	int						GetNumCurves() const;
	int						GetNumJunctions() const;
	
	//----------------------------------------
	// cell iterator
	PACKED_CELL_OBJECT*		GetFirstPackedCop(CELL_ITERATOR_D2* iterator, const XZPAIR& cell) const;
	PACKED_CELL_OBJECT*		GetNextPackedCop(CELL_ITERATOR_D2* iterator) const;
	static bool				UnpackCellObject(CELL_OBJECT& co, PACKED_CELL_OBJECT* pco, const XZPAIR& nearCell);

protected:
	
	// Driver 2 - specific
	CDriver2LevelRegion*	m_regions{ nullptr };					// map of regions
	PACKED_CELL_OBJECT*		m_packedStraddlers { nullptr };			// cell objects between regions

	DRIVER2_STRAIGHT*		m_straights{ nullptr };
	DRIVER2_CURVE*			m_curves{ nullptr };
	DRIVER2_JUNCTION*		m_junctions{ nullptr };

	int						m_numStraights{ 0 };
	int						m_numCurves{ 0 };
	int						m_numJunctions{ 0 };
};


#include "regions_d2.h"

#include "level.h"
#include "core/cmdlib.h"
#include "core/IVirtualStream.h"

#include "math/isin.h"
#include "math/ratan2.cpp"

#include <string.h>

#define IS_STRAIGHT_SURFACE(surfid)			(((surfid) > -1) && ((surfid) & 0xFFFFE000) == 0 && ((surfid) & 0x1FFF) < m_numStraights)
#define IS_CURVED_SURFACE(surfid)			(((surfid) > -1) && ((surfid) & 0xFFFFE000) == 0x4000 && ((surfid) & 0x1FFF) < m_numCurves)
#define IS_JUNCTION_SURFACE(surfid)			(((surfid) > -1) && ((surfid) & 0xFFFFE000) == 0x2000 && ((surfid) & 0x1FFF) < m_numJunctions)

extern sdPlane g_defaultPlane;
extern sdPlane g_seaPlane;

int SdHeightOnPlane(const VECTOR_NOPAD& position, sdPlane* plane, DRIVER2_CURVE* curves)
{
	int angle;
	int i, d;
	int lx;
	int ly;

	if (plane)
	{
		d = plane->d;

		if ((d >> 1 ^ d) & 0x40000000)
			return d ^ 0x40000000;

		if ((plane->surfaceType & 0xE000) == 0x4000 && plane->b == 0)
		{
			// calculate curve point
			DRIVER2_CURVE& curve = curves[(plane->surfaceType & 0x1fff) - 32];
			angle = ratan2(curve.Midz - position.vz, curve.Midx - position.vx);

			return ((curve.gradient * (angle + 2048 & 4095)) / ONE) - curve.height;
		}

		i = plane->b;

		if (i != 0)
		{
			if (i == 0x4000)
				return -d;

			//const int half_cell_size = m_mapInfo.cell_size / 2;

			lx = (int)plane->a * ((position.vx - 512 & 0xffff) + 512);
			ly = (int)plane->c * ((position.vz - 512 & 0xffff) + 512);

			return -d - (lx + ly) / i;
		}
	}

	return 0;
}

// walk BSP nodes
short* SdGetBSP(sdNode* node, XZPAIR* pos)
{
	while (node->node < 0)
	{
		int ang = node->angle;
		int dot = pos->z * icos(ang) - pos->x * isin(ang);

		if (dot < node->dist * 4096)
			node++;
		else
			node += node->offset;
	}

	return (short*)node;
}

sdPlane* FindRoadInBSP(sdNode* node, sdPlane* base)
{
	sdPlane* plane;

	while (true)
	{
		if (*(int*)node > -1)
		{
			base += *(int*)node;
			return (base->surfaceType < 32) ? nullptr : base;
		}

		plane = FindRoadInBSP(node + 1, base);

		if (plane != nullptr)
			break;

		node += node->offset;
	}

	return plane;
}

// walk the heightmap to get a cPosition
sdPlane* CDriver2LevelRegion::SdGetCell(const VECTOR_NOPAD& cPosition, int& sdLevel, sdBspCallback bspWalker) const
{
	bool nextLevel;
	sdPlane* plane;
	short* surface;
	short* BSPSurface;
	XZPAIR cell;

	plane = nullptr;

	sdLevel = 0;

	if (!m_surfaceData)
		return nullptr;

	// FIXME: divide by cell size??
	surface = &m_surfaceData[(cPosition.vx >> 10 & 63) +
							 (cPosition.vz >> 10 & 63) * 64];

	// initial surface
	if (*surface == -1)
		return &g_defaultPlane;

	const bool simplerMethod = m_owner->m_format == LEV_FORMAT_DRIVER2_ALPHA16;

	// check surface has overlapping planes flag (aka multiple levels)
	if(simplerMethod ?
		(*surface & 0x8000) : 
		(*surface & 0x6000) == 0x2000)
	{
		surface = &m_bspData[*surface & 0x1fff];
		do {
			if (-256 - cPosition.vy > *surface)
			{
				surface += 2;
				sdLevel++;
			}
			else
				break;
		} while (*surface != -0x8000); // end flag

		surface += 1;
	}
	
	// iterate surfaces if BSP
	do {
		nextLevel = false;

		// check if it's has BSP properties
		// basically it determines surface bounds
		if (*surface & 0x4000)
		{
			cell.x = cPosition.vx & 1023;
			cell.z = cPosition.vz & 1023;

			// get closest surface by BSP lookup
			if(simplerMethod)
				BSPSurface = bspWalker(&m_nodeData[*surface & 0x1fff], &cell);
			else
				BSPSurface = bspWalker(&m_nodeData[*surface & 0x3fff], &cell);

			if (*BSPSurface == 0x7fff)
			{
				sdLevel++;
				nextLevel = true;

				BSPSurface = surface + 2; // get to the next node
			}

			surface = BSPSurface;
		}
	} while (nextLevel);

	plane = &m_planeData[*surface];

	if (((int)plane & 3) != 0 || *(int*)plane == -1)
	{
		return nullptr;
	}

	return plane;
}

// walk heightmap for nearest road
int CDriver2LevelRegion::RoadInCell(VECTOR_NOPAD& position) const
{
	int moreLevels;
	sdPlane* plane;
	short* check;

	XYPAIR cellPos;

	//const OUT_CELL_FILE_HEADER& mapInfo = m_owner->m_mapInfo;

	cellPos.x = position.vx - 512;
	cellPos.y = position.vz - 512;

	check = &m_surfaceData[(cellPos.x >> 10 & 63) +
						   (cellPos.y >> 10 & 63) * 64 + 4];

	if (*check == -1)
		return -1;

	if (m_owner->m_format == LEV_FORMAT_DRIVER2_ALPHA16)
	{
		// FIXME: check if this is redundant!
		if (*check & 0xE000)
		{
			if (*check & 0x2000)
			{
				// check surface has overlapping planes flag (aka multiple levels)
				moreLevels = (*check & 0x8000) != 0;

				if (moreLevels)
					check = &m_bspData[(*check & 0x1fff) + 1];

				do
				{
					if (moreLevels && check[-1] == -0x8000)
						moreLevels = 0;

					// check if it's has BSP properties
					// basically it determines surface bounds
					if (*check & 0x4000)
					{
						sdNode* search = &m_nodeData[*check & 0x1fff];		// 0x3fff in final

						while (search->node < 0)
						{
							plane = FindRoadInBSP(search + 1, m_planeData);

							if (plane != nullptr)
								break;

							search += search->offset;
						}

						if (plane != nullptr)
							break;
					}
					else
					{
						plane = &m_planeData[*check];

						if (plane->surfaceType >= 32)
							break;
					}

					check += 2;
				} while (true);
			}
			else
			{
				plane = nullptr;
			}
		}
		else
		{
			plane = &m_planeData[*check];
		}
	}
	else
	{
		if (*check & 0x8000)
		{
			moreLevels = (*check & 0x6000) == 0x2000;

			if (moreLevels)
				check = &m_bspData[(*check & 0x1fff) + 1];

			do
			{
				if (moreLevels && check[-1] == 0x8000)
					moreLevels = 0;

				if (*check & 0x4000)
				{
					plane = FindRoadInBSP(&m_nodeData[*check & 0x3fff], m_planeData);

					if (plane != nullptr)
						break;
				}
				else
				{
					plane = &m_planeData[*check];

					if (plane->surfaceType >= 32)
						break;
				}

				check += 2;
			} while (true);
		}
		else if (!(*check & 0xE000))
		{
			plane = &m_planeData[*check];
		}
		else
			plane = nullptr;
	}

	if (plane == nullptr)
		return -1;

	if (plane->surfaceType >= 32)
	{
		position.vy = SdHeightOnPlane(position, plane, ((CDriver2LevelMap*)m_owner)->m_curves) + 256;
		return plane->surfaceType - 32;
	}

	return -1;
}

void CDriver2LevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	CBaseLevelRegion::FreeAll();

	if (m_cells)
		Memory::free(m_cells);
	m_cells = nullptr;

	if (m_packedCellObjects)
		Memory::free(m_packedCellObjects);
	m_packedCellObjects = nullptr;

	if (m_pvsData)
		Memory::free(m_pvsData);
	m_pvsData = nullptr;
}

void CDriver2LevelRegion::LoadRegionData(const SPOOL_CONTEXT& ctx)
{
	IVirtualStream* pFile = ctx.dataStream;

	DevMsg(SPEW_NORM,"---------\nSpool %d %d\n", m_regionX, m_regionZ);
	DevMsg(SPEW_NORM," - offset: %d\n", m_spoolInfo->offset);

	for (int i = 0; i < m_spoolInfo->num_connected_areas && i < 2; i++)
		DevMsg(SPEW_NORM, " - connected area %d: %d\n", i, m_spoolInfo->connected_areas[i]);

	DevMsg(SPEW_NORM," - pvs_size: %d\n", m_spoolInfo->pvs_size);
	DevMsg(SPEW_NORM," - cell_data_size: %d %d %d\n", m_spoolInfo->cell_data_size[0], m_spoolInfo->cell_data_size[1], m_spoolInfo->cell_data_size[2]);

	DevMsg(SPEW_NORM, " - super_region: %d\n", m_spoolInfo->super_region);

	// LoadRegionData - calculate offsets
	DevMsg(SPEW_NORM, " - cell pointers size: %d\n", m_spoolInfo->cell_data_size[1]);
	DevMsg(SPEW_NORM, " - cell data size: %d\n", m_spoolInfo->cell_data_size[0]);
	DevMsg(SPEW_NORM, " - cell objects size: %d\n", m_spoolInfo->cell_data_size[2]);
	DevMsg(SPEW_NORM, " - PVS data size: %d\n", m_spoolInfo->roadm_size);

	//
	// Driver 2 use PACKED_CELL_OBJECTS - 8 bytes. not wasting, but tricky
	//

	int cellPointersOffset;
	int cellDataOffset;
	int cellObjectsOffset;
	int pvsHeightmapDataOffset;

	if (m_owner->m_format == LEV_FORMAT_DRIVER2_RETAIL) // retail
	{
		cellPointersOffset = m_spoolInfo->offset;
		cellDataOffset = cellPointersOffset + m_spoolInfo->cell_data_size[1];
		cellObjectsOffset = cellDataOffset + m_spoolInfo->cell_data_size[0];
		pvsHeightmapDataOffset = cellObjectsOffset + m_spoolInfo->cell_data_size[2];
	}
	else // 1.6 alpha
	{
		pvsHeightmapDataOffset = m_spoolInfo->offset;
		cellPointersOffset = pvsHeightmapDataOffset + m_spoolInfo->roadm_size;
		cellDataOffset = cellPointersOffset + m_spoolInfo->cell_data_size[1];
		cellObjectsOffset = cellDataOffset + m_spoolInfo->cell_data_size[0];
	}

	char* packed_cell_pointers = new char[m_spoolInfo->cell_data_size[1] * SPOOL_CD_BLOCK_SIZE];

	m_cellPointers = new ushort[m_owner->m_cell_objects_add[5]];
	memset(m_cellPointers, 0xFF, sizeof(ushort) * m_owner->m_cell_objects_add[5]);

	// read packed cell pointers
	pFile->Seek(ctx.lumpInfo->spooled_offset + cellPointersOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	pFile->Read(packed_cell_pointers, m_spoolInfo->cell_data_size[1] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

	// unpack cell pointers so we can use them
	if (UnpackCellPointers(m_cellPointers, packed_cell_pointers, 0, 0) != -1)
	{
		// read cell data
		m_cells = (CELL_DATA*)Memory::alloc(m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cells, m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

		// read cell objects
		m_packedCellObjects = (PACKED_CELL_OBJECT*)Memory::alloc(m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellObjectsOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_packedCellObjects, m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE, sizeof(char));
	}
	else
		MsgError("BAD PACKED CELL POINTER DATA, region = %d\n", m_regionNumber);

	// post-process
	UnpackAllCellObjects();

	delete [] packed_cell_pointers;

	pFile->Seek(ctx.lumpInfo->spooled_offset + pvsHeightmapDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	ReadHeightmapData(ctx);

	// TODO: PVS data for LEV_FORMAT_DRIVER2_ALPHA, which in separate spool offset

	// even if error occured we still need it to be here
	m_loaded = true;

	m_owner->OnRegionLoaded(this);
}

//---------------------------------------------------------------------
// Unpacks all cell objects from PACKED_CELL_OBJECT
//---------------------------------------------------------------------
void CDriver2LevelRegion::UnpackAllCellObjects()
{
	CDriver2LevelMap* owner = (CDriver2LevelMap*)m_owner;
	int numCellObjects = (m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE) / sizeof(PACKED_CELL_OBJECT);

	// alloc and convert
	m_cellObjects = (CELL_OBJECT*)Memory::alloc(numCellObjects * sizeof(CELL_OBJECT));
	memset(m_cellObjects, 0, numCellObjects * sizeof(CELL_OBJECT));

	const OUT_CELL_FILE_HEADER& mapInfo = owner->GetMapInfo();
	const int numStraddlers = owner->m_numStraddlers;
	const int cellObjectsAdd = owner->m_cell_objects_add[m_regionBarrelNumber];

	// walk through all cell data
	for (int i = 0; i < mapInfo.region_size * mapInfo.region_size; i++)
	{
		CELL_ITERATOR_D2 ci;
		PACKED_CELL_OBJECT* pco = StartIterator(&ci, i);

		if (!pco)
			continue;

		while (pco)
		{
			ushort num = ci.pcd->num & 16383;

			if (num >= numStraddlers)
			{
				num -= cellObjectsAdd + numStraddlers;
				
				CELL_OBJECT& co = m_cellObjects[num];
				CDriver2LevelMap::UnpackCellObject(co, pco, ci.nearCell);
			}
			else
			{
				// unpack straddlers
				CDriver2LevelMap::UnpackCellObject(owner->m_straddlers[num], pco, ci.nearCell);
			}

			pco = owner->GetNextPackedCop(&ci);
		}
	}
}

void CDriver2LevelRegion::ReadHeightmapData(const SPOOL_CONTEXT& ctx)
{
	IVirtualStream* pFile = ctx.dataStream;

	int pvsDataSize = 0;
	m_pvsData = (char*)Memory::alloc(m_spoolInfo->roadm_size * SPOOL_CD_BLOCK_SIZE);

	if (m_owner->m_format == LEV_FORMAT_DRIVER2_RETAIL) // retail do have PVS data in the start
		pFile->Read(&pvsDataSize, 1, sizeof(int));

	pFile->Read(m_pvsData, m_spoolInfo->roadm_size * SPOOL_CD_BLOCK_SIZE, sizeof(char));

	// go to heightmap
	sdHeightmapHeader* hdr = (sdHeightmapHeader*)(m_pvsData + pvsDataSize);

	if (hdr->type == 2)
	{
		m_planeData = (sdPlane*)((char*)hdr + hdr->planesOfs);
		m_bspData = (short*)((char*)hdr + hdr->bspOfs);
		m_nodeData = (sdNode*)((char*)hdr + hdr->nodesOfs);
		m_surfaceData = (short*)(hdr + 1);	// surface indexes
	}
	else
	{
		MsgError("Incorrect road map format or read error\n");
	}
}

PACKED_CELL_OBJECT* CDriver2LevelRegion::GetPackedCellObject(int num) const
{
	CDriver2LevelMap* owner = (CDriver2LevelMap*)m_owner;

	int numStraddlers = owner->m_numStraddlers;

	if (num >= numStraddlers)
	{
		num -= owner->m_cell_objects_add[m_regionBarrelNumber] + numStraddlers;
		return &m_packedCellObjects[num];
	}

	return &owner->m_packedStraddlers[num];
}

CELL_DATA* CDriver2LevelRegion::GetCellData(int num) const
{
	return &m_cells[num];
}

// cell iterator
PACKED_CELL_OBJECT* CDriver2LevelRegion::StartIterator(CELL_ITERATOR_D2* iterator, int cellNumber) const
{
	ushort cell_ptr = m_cellPointers[cellNumber];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	CDriver2LevelMap* owner = (CDriver2LevelMap*)m_owner;
	const OUT_CELL_FILE_HEADER& mapInfo = owner->m_mapInfo;

	iterator->region = (CDriver2LevelRegion*)this;

	// IDK if it gonna work correct
	XZPAIR cpos;
	owner->WorldPositionToCellXZ(cpos, {0,0,0});

	// convert cell number to XZ
	int cellx = cellNumber % mapInfo.region_size;
	int cellz = (cellNumber - cellx) / mapInfo.region_size;

	cellx += m_regionX * mapInfo.region_size;
	cellz += m_regionZ * mapInfo.region_size;
	
	// get the near cell cPosition in the world
	iterator->nearCell.x = (cellx - (mapInfo.cells_across / 2))* mapInfo.cell_size;
	iterator->nearCell.z = (cellz - (mapInfo.cells_down / 2))* mapInfo.cell_size;

	// get the packed cell data start and near cell
	CELL_DATA* celld = &m_cells[cell_ptr];

	iterator->listType = -1;

	if (celld->num & 0x4000) // if we immediately got to the typed list
	{
		iterator->listType = celld->num & 16383;
		celld++; // get to the start
	}

	PACKED_CELL_OBJECT* ppco = GetPackedCellObject(celld->num & 16383);
	iterator->pcd = celld;

	iterator->co = GetCellObject(celld->num & 16383);

	if (ppco->value == 0xffff && (ppco->pos.vy & 1))
		ppco = owner->GetNextPackedCop(iterator);

	iterator->ppco = ppco;

	return ppco;
}

//-------------------------------------------------------------------------------------------

void CDriver2LevelMap::FreeAll()
{
	int total_regions = m_regions_across * m_regions_down;

	if (m_regions)
	{
		for (int i = 0; i < total_regions; i++)
			m_regions[i].FreeAll();
	}

	delete[] m_regions;
	m_regions = nullptr;

	delete[] m_packedStraddlers;
	m_packedStraddlers = nullptr;

	delete[] m_straights;
	m_straights = nullptr;

	delete[] m_curves;
	m_curves = nullptr;

	delete[] m_junctions;
	m_junctions = nullptr;

	CBaseLevelMap::FreeAll();
}

//-------------------------------------------------------------
// Loads map lump, Driver 2 version
//-------------------------------------------------------------
void CDriver2LevelMap::LoadMapLump(IVirtualStream* pFile)
{
	CBaseLevelMap::LoadMapLump(pFile);

	// read straddlers
	// Driver 2 PCO
	m_packedStraddlers = new PACKED_CELL_OBJECT[m_numStraddlers];
	pFile->Read(m_packedStraddlers, m_numStraddlers, sizeof(PACKED_CELL_OBJECT));

	m_straddlers = new CELL_OBJECT[m_numStraddlers];
	memset(m_straddlers, 0, m_numStraddlers * sizeof(CELL_OBJECT));
}

//-------------------------------------------------------------
// Load spool info, Driver 2 version
//-------------------------------------------------------------
void CDriver2LevelMap::LoadSpoolInfoLump(IVirtualStream* pFile)
{
	CBaseLevelMap::LoadSpoolInfoLump(pFile);

	// Init regions
	int total_regions = m_regions_across * m_regions_down;

	m_regions = new CDriver2LevelRegion[total_regions];

	for (int i = 0; i < total_regions; i++)
		InitRegion(&m_regions[i], i);
}

void CDriver2LevelMap::LoadStraightsLump(IVirtualStream* pFile)
{
	pFile->Read(&m_numStraights, 1, sizeof(int));
	m_straights = new DRIVER2_STRAIGHT[m_numStraights];

	pFile->Read(m_straights, m_numStraights, sizeof(DRIVER2_STRAIGHT));
}

void CDriver2LevelMap::LoadCurvesLump(IVirtualStream* pFile)
{
	pFile->Read(&m_numCurves, 1, sizeof(int));
	m_curves = new DRIVER2_CURVE[m_numCurves];

	pFile->Read(m_curves, m_numCurves, sizeof(DRIVER2_CURVE));
}

void CDriver2LevelMap::LoadJunctionsLump(IVirtualStream* pFile, bool oldFormat)
{
	pFile->Read(&m_numJunctions, 1, sizeof(int));
	m_junctions = new DRIVER2_JUNCTION[m_numJunctions];

	// convert old format to new format
	if (oldFormat)
	{
		OLD_DRIVER2_JUNCTION* oldJunctions = new OLD_DRIVER2_JUNCTION[m_numJunctions];

		pFile->Read(oldJunctions, m_numJunctions, sizeof(DRIVER2_JUNCTION));

		for (int i = 0; i < m_numJunctions; i++)
		{
			m_junctions[i].flags = oldJunctions[i].flags;

			for (int j = 0; j < 4; j++)
				m_junctions[i].ExitIdx[j] = oldJunctions[i].ExitIdx[j];
		}
	}
	else
		pFile->Read(m_junctions, m_numJunctions, sizeof(DRIVER2_JUNCTION));
}

CBaseLevelRegion* CDriver2LevelMap::GetRegion(const XZPAIR& cell) const
{
	// lookup region
	const int region_x = cell.x / m_mapInfo.region_size;
	const int region_z = cell.z / m_mapInfo.region_size;

	return GetRegion(region_x + region_z * m_regions_across);
}

CBaseLevelRegion* CDriver2LevelMap::GetRegion(int regionIdx) const
{
#if 0
	const int total_regions = m_regions_across * m_regions_down;

	if (regionIdx < 0 && regionIdx >= total_regions)
		return nullptr;
#endif

	return &m_regions[regionIdx];
}

bool CDriver2LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell)
{
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	if (region && !region->m_loaded)
	{
		if (m_regionSpoolInfoOffsets[region->m_regionNumber] != REGION_EMPTY)
		{
			region->LoadRegionData(ctx);
			region->LoadAreaData(ctx);
			return true;
		}
		else
			region->m_loaded = true;
	}
	return false;
}

bool CDriver2LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx)
{
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(regionIdx);

	if (region && !region->m_loaded)
	{
		if (m_regionSpoolInfoOffsets[region->m_regionNumber] != REGION_EMPTY)
		{
			region->LoadRegionData(ctx);
			region->LoadAreaData(ctx);
			return true;
		}
		else
			region->m_loaded = true;
	}
	return false;
}

void CDriver2LevelMap::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const
{
	VECTOR_NOPAD cellPos;
	XZPAIR cell;
	int level = 0;

	cellPos.vx = position.vx - 512;	// FIXME: is that a quarter of a cell?
	cellPos.vy = position.vy;
	cellPos.vz = position.vz - 512;

	WorldPositionToCellXZ(cell, cellPos);
	const CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	if (region)
	{
		const sdPlane* foundPlane = region->SdGetCell(cellPos, level, SdGetBSP);

		if (foundPlane)
			outPlane = *foundPlane;
		else
			outPlane = g_seaPlane;

		outPoint.vx = position.vx;
		outPoint.vz = position.vz;
		outPoint.vy = SdHeightOnPlane(position, &outPlane, m_curves);
	}
}

int	CDriver2LevelMap::GetRoadIndex(VECTOR_NOPAD& position) const
{
	VECTOR_NOPAD cellPos;
	XZPAIR cell;
	int level = 0;

	//const int half_cell_size = m_mapInfo.cell_size / 2;

	cellPos.vx = position.vx - 512;	// FIXME: is that a quarter of a cell?
	cellPos.vy = position.vy;
	cellPos.vz = position.vz - 512;

	WorldPositionToCellXZ(cell, cellPos);
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	return region->RoadInCell(position);
}

// [A] custom function for working with roads in very optimized way
bool CDriver2LevelMap::GetSurfaceRoadInfo(DRIVER2_ROAD_INFO& outRoadInfo, int surfId) const
{
	DRIVER2_CURVE* curve;
	DRIVER2_STRAIGHT* straight;
	DRIVER2_JUNCTION* junction;

	outRoadInfo.surfId = surfId;

	if (IS_CURVED_SURFACE(surfId))
	{
		outRoadInfo.curve = curve = GetCurve(surfId);
		outRoadInfo.ConnectIdx = &curve->ConnectIdx;
		outRoadInfo.NumLanes = curve->NumLanes;
		outRoadInfo.LaneDirs = curve->LaneDirs;
		outRoadInfo.AILanes = curve->AILanes;
		return true;
	}
	else if (IS_STRAIGHT_SURFACE(surfId))
	{
		outRoadInfo.straight = straight = GetStraight(surfId);
		outRoadInfo.ConnectIdx = &straight->ConnectIdx;
		outRoadInfo.NumLanes = straight->NumLanes;
		outRoadInfo.LaneDirs = straight->LaneDirs;
		outRoadInfo.AILanes = straight->AILanes;
		return true;
	}
	else if (IS_JUNCTION_SURFACE(surfId))
	{
		outRoadInfo.junction = junction = GetJunction(surfId);
		outRoadInfo.ConnectIdx = &junction->ExitIdx;
		outRoadInfo.flags = junction->flags;
		return true;
	}

	return false;
}

DRIVER2_STRAIGHT* CDriver2LevelMap::GetStraight(int index) const
{
	if(IS_STRAIGHT_SURFACE(index))
		return &m_straights[index & 0x1fff];

	return nullptr;
}

DRIVER2_CURVE* CDriver2LevelMap::GetCurve(int index) const
{
	if (IS_CURVED_SURFACE(index))
		return &m_curves[index & 0x1fff];

	return nullptr;
}

DRIVER2_JUNCTION* CDriver2LevelMap::GetJunction(int index) const
{
	if (IS_JUNCTION_SURFACE(index))
		return &m_junctions[index & 0x1fff];

	return nullptr;
}

int CDriver2LevelMap::GetNumStraights() const
{
	return m_numStraights;
}

int CDriver2LevelMap::GetNumCurves() const
{
	return m_numCurves;
}

int CDriver2LevelMap::GetNumJunctions() const
{
	return m_numJunctions;
}

//-------------------------------------------------------------
// returns first cell object of cell
//-------------------------------------------------------------
PACKED_CELL_OBJECT* CDriver2LevelMap::GetFirstPackedCop(CELL_ITERATOR_D2* iterator, const XZPAIR& cell) const
{
	// lookup region
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	iterator->region = region;

	// don't do anything on empty or non-spooled regions
	if (!region->m_cells)
		return nullptr;

	// get cell index on the region
	const int region_cell_x = cell.x % m_mapInfo.region_size;
	const int region_cell_z = cell.z % m_mapInfo.region_size;

	// FIXME: might be incorrect
	int cell_index = region_cell_x + region_cell_z * m_mapInfo.region_size;

	ushort cell_ptr = region->m_cellPointers[cell_index];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	// get the near cell cPosition in the world
	iterator->nearCell.x = (cell.x - (m_mapInfo.cells_across / 2)) * m_mapInfo.cell_size;
	iterator->nearCell.z = (cell.z - (m_mapInfo.cells_down / 2)) * m_mapInfo.cell_size;

	// get the packed cell data start and near cell
	CELL_DATA* celld = &region->m_cells[cell_ptr];

	/*
		Data looks like this:

		45,34,773,456    - default list of cell objects
		0x4000 | 100     - list 1 header - type 100
		70,378,4557      - objects of list 1
		0x4000 | 14      - list 2 header - type 14
		8767,555,445,223 - objects of list 2
		0x8000           - end of cell objects
	*/

	iterator->listType = -1;

	if (celld->num & 0x4000) // if we immediately got to the typed list
	{
		iterator->listType = celld->num & 16383;
		celld++; // get to the start
	}

	PACKED_CELL_OBJECT* ppco = region->GetPackedCellObject(celld->num & 16383);
	iterator->co = region->GetCellObject(celld->num & 16383);

	iterator->pcd = celld;

	if (ppco->value == 0xffff && (ppco->pos.vy & 1))
	{
		ppco = GetNextPackedCop(iterator);
	}
	else if (iterator->cache)
	{
		CELL_ITERATOR_CACHE* cache = iterator->cache;
		ushort num = celld->num & 16383;
		uint value = 1 << (num & 7);

		if ((cache->computedValues[num / 8] & value))
		{
			ppco = GetNextPackedCop(iterator);
			iterator->ppco = ppco;

			return ppco;
		}

		cache->computedValues[num / 8] |= value;
	}

	iterator->ppco = ppco;

	return ppco;
}

//-------------------------------------------------------------
// iterates cell objects
//-------------------------------------------------------------
PACKED_CELL_OBJECT* CDriver2LevelMap::GetNextPackedCop(CELL_ITERATOR_D2* iterator) const
{
	PACKED_CELL_OBJECT* ppco;
	CELL_OBJECT* co;
	CDriver2LevelRegion* reg = iterator->region;
	CELL_DATA* celld = iterator->pcd;

	do
	{
		do {
			if (celld->num & 0x8000)	// end of the cell objects?
				return nullptr;

			celld++;

			if (celld->num & 0x4000) // if we got new list
			{
				iterator->listType = celld->num & 16383;
				celld++; // get to the start
			}

			iterator->pcd = celld;

			ppco = reg->GetPackedCellObject(celld->num & 16383);
			co = reg->GetCellObject(celld->num & 16383);
		} while (ppco->value == 0xffff && (ppco->pos.vy & 1));

		if (iterator->cache)
		{
			CELL_ITERATOR_CACHE* cache = iterator->cache;
			ushort num = celld->num & 16383;
			uint value = 1 << (num & 7);

			if ((cache->computedValues[num / 8] & value) == 0)
			{
				cache->computedValues[num / 8] |= value;
				break;
			}
		}
		else
			break;

	} while (true);

	iterator->ppco = ppco;
	iterator->co = co;

	return ppco;
}

//-------------------------------------------------------------
// Unpacks cell object (Driver 2 ONLY)
//-------------------------------------------------------------
bool CDriver2LevelMap::UnpackCellObject(CELL_OBJECT& co, PACKED_CELL_OBJECT* pco, const XZPAIR& nearCell)
{
	if (!pco)
		return false;

	co.pos.vx = nearCell.x + (short)(pco->pos.vx - nearCell.x);
	co.pos.vz = nearCell.z + (short)(pco->pos.vz - nearCell.z);

	// cell height should be negated
	co.pos.vy = (short)pco->pos.vy >> 1;

	co.yang = pco->value & 0x3f;
	co.type = (pco->value >> 6) | ((pco->pos.vy & 1) << 10);

	return true;
}
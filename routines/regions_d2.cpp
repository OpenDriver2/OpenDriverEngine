
#include "regions_d2.h"

#include "level.h"
#include "core/cmdlib.h"
#include "core/IVirtualStream.h"

#include "math/isin.h"
#include "math/ratan2.cpp"

#include <malloc.h>
#include <string.h>

sdPlane g_defaultPlane	= { 0, 0, 0, 0, 2048 };
sdPlane g_seaPlane		= { 9, 0, 16384, 0, 2048 };

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
		return &g_seaPlane;

	bool simplerMethod = m_owner->m_format == LEV_FORMAT_DRIVER2_ALPHA16;

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

	if (((int)plane & 3) == 0 && *(int*)plane != -1)
	{
		// TODO: event surface handling

		//if (plane->surfaceType - 16U < 16)
		//	plane = EventSurface(pos, plane);

		//plane = nullptr;
	}
	else
		plane = &g_seaPlane;

	return plane;
}

void CDriver2LevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	CBaseLevelRegion::FreeAll();

	if (m_cells)
		free(m_cells);
	m_cells = nullptr;

	if (m_cellObjects)
		free(m_cellObjects);
	m_cellObjects = nullptr;

	if (m_pvsData)
		free(m_pvsData);
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
		m_cells = (CELL_DATA*)malloc(m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cells, m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

		// read cell objects
		m_cellObjects = (PACKED_CELL_OBJECT*)malloc(m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellObjectsOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cellObjects, m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE, sizeof(char));
	}
	else
		MsgError("BAD PACKED CELL POINTER DATA, region = %d\n", m_regionNumber);

	delete [] packed_cell_pointers;

	pFile->Seek(ctx.lumpInfo->spooled_offset + pvsHeightmapDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	ReadHeightmapData(ctx);

	// TODO: PVS data for LEV_FORMAT_DRIVER2_ALPHA, which in separate spool offset

	// even if error occured we still need it to be here
	m_loaded = true;

	m_owner->OnRegionLoaded(this);
}

void CDriver2LevelRegion::ReadHeightmapData(const SPOOL_CONTEXT& ctx)
{
	IVirtualStream* pFile = ctx.dataStream;

	int pvsDataSize = 0;
	m_pvsData = (char*)malloc(m_spoolInfo->roadm_size * SPOOL_CD_BLOCK_SIZE);

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
		MsgError("Incorrect format or read error\n");
	}
}

PACKED_CELL_OBJECT* CDriver2LevelRegion::GetCellObject(int num) const
{
	CDriver2LevelMap* owner = (CDriver2LevelMap*)m_owner;

	int numStraddlers = owner->m_numStraddlers;

	if (num >= numStraddlers)
	{
		num -= owner->m_cell_objects_add[m_regionBarrelNumber] + numStraddlers;
		return &m_cellObjects[num];
	}

	return &owner->m_straddlers[num];
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

	const OUT_CELL_FILE_HEADER& mapInfo = m_owner->m_mapInfo;

	iterator->region = (CDriver2LevelRegion*)this;

	// IDK if it gonna work correct
	XZPAIR cpos;
	m_owner->WorldPositionToCellXZ(cpos, {0,0,0});

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

	iterator->listType = 0;

	if (celld->num & 0x4000) // if we immediately got to the typed list
	{
		iterator->listType = celld->num;
		celld++; // get to the start
	}

	PACKED_CELL_OBJECT* ppco = GetCellObject(celld->num & 0x3fff);
	iterator->pcd = celld;

	if (ppco->value == 0xffff && (ppco->pos.vy & 1))
		ppco = ((CDriver2LevelMap*)m_owner)->GetNextPackedCop(iterator);

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

	delete[] m_straddlers;
	m_straddlers = nullptr;

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
	m_straddlers = new PACKED_CELL_OBJECT[m_numStraddlers];
	pFile->Read(m_straddlers, m_numStraddlers, sizeof(PACKED_CELL_OBJECT));
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

	return &m_regions[region_x + region_z * m_regions_across];
}

CBaseLevelRegion* CDriver2LevelMap::GetRegion(int regionIdx) const
{
	return &m_regions[regionIdx];
}

void CDriver2LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell)
{
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	if (!region->m_loaded)
	{
		if (m_regionSpoolInfoOffsets[region->m_regionNumber] != REGION_EMPTY)
		{
			region->LoadRegionData(ctx);
			region->LoadAreaData(ctx);
		}
		else
			region->m_loaded = true;
	}
}

void CDriver2LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx)
{
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(regionIdx);

	if (!region->m_loaded)
	{
		if (m_regionSpoolInfoOffsets[region->m_regionNumber] != REGION_EMPTY)
		{
			region->LoadRegionData(ctx);
			region->LoadAreaData(ctx);
		}
		else
			region->m_loaded = true;
	}
}

int CDriver2LevelMap::MapHeight(const VECTOR_NOPAD& position) const
{
	VECTOR_NOPAD cellPos;
	XZPAIR cell;
	int level = 0;

	cellPos.vx = position.vx - 512;	// FIXME: is that a quarter of a cell?
	cellPos.vy = position.vy;
	cellPos.vz = position.vz - 512;

	WorldPositionToCellXZ(cell, cellPos);
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)GetRegion(cell);

	sdPlane* plane = region->SdGetCell(cellPos, level, SdGetBSP);

	if (plane)
		return SdHeightOnPlane(position, plane, m_curves);

	return 0;
}

//-------------------------------------------------------------
// returns first cell object of cell
//-------------------------------------------------------------
PACKED_CELL_OBJECT* CDriver2LevelMap::GetFirstPackedCop(CELL_ITERATOR_D2* iterator, const XZPAIR& cell) const
{
	// lookup region
	const int region_x = cell.x / m_mapInfo.region_size;
	const int region_z = cell.z / m_mapInfo.region_size;

	int regionIdx = region_x + region_z * m_regions_across;

	CDriver2LevelRegion& region = m_regions[regionIdx];

	iterator->region = &region;

	// don't do anything on empty or non-spooled regions
	if (!region.m_cells)
		return nullptr;

	// get cell index on the region
	const int region_cell_x = cell.x % m_mapInfo.region_size;
	const int region_cell_z = cell.z % m_mapInfo.region_size;

	// FIXME: might be incorrect
	int cell_index = region_cell_x + region_cell_z * m_mapInfo.region_size;

	ushort cell_ptr = region.m_cellPointers[cell_index];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	// get the near cell cPosition in the world
	iterator->nearCell.x = (cell.x - (m_mapInfo.cells_across / 2)) * m_mapInfo.cell_size;
	iterator->nearCell.z = (cell.z - (m_mapInfo.cells_down / 2)) * m_mapInfo.cell_size;

	// get the packed cell data start and near cell
	CELL_DATA* celld = &region.m_cells[cell_ptr];

	/*
		Data looks like this:

		45,34,773,456    - default list of cell objects
		0x4000 | 100     - list 1 header - type 100
		70,378,4557      - objects of list 1
		0x4000 | 14      - list 2 header - type 14
		8767,555,445,223 - objects of list 2
		0x8000           - end of cell objects
	*/

	iterator->listType = 0;

	if (celld->num & 0x4000) // if we immediately got to the typed list
	{
		iterator->listType = celld->num;
		celld++; // get to the start
	}

	PACKED_CELL_OBJECT* ppco = region.GetCellObject(celld->num & 0x3fff);

	iterator->pcd = celld;

	if (ppco->value == 0xffff && (ppco->pos.vy & 1))
		ppco = GetNextPackedCop(iterator);

	iterator->ppco = ppco;

	return ppco;
}

//-------------------------------------------------------------
// iterates cell objects
//-------------------------------------------------------------
PACKED_CELL_OBJECT* CDriver2LevelMap::GetNextPackedCop(CELL_ITERATOR_D2* iterator) const
{
	ushort num;
	PACKED_CELL_OBJECT* ppco;

	do {
		CELL_DATA* celld = iterator->pcd;

		if (celld->num & 0x8000)	// end of the cell objects?
			return nullptr;

		celld++;

		if (celld->num & 0x4000) // if we got new list
		{
			iterator->listType = celld->num;
			celld++; // get to the start
		}

		iterator->pcd = celld;

		/*
		celld++;
		num = celld->num;

		if (celld->num & 0x4000) // if we immediately got to the typed list
		{
			iterator->listType = celld->num;
			celld++; // get to the start
		}
		
		if(num & 0x4000)	// start of new sub list?
			return nullptr;
		*/
		ppco = iterator->region->GetCellObject(celld->num & 0x3fff);

	} while (ppco->value == 0xffff && (ppco->pos.vy & 1));

	iterator->ppco = ppco;

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
#include "regions_d1.h"

#include "level.h"
#include <string.h>
#include "core/cmdlib.h"
#include "core/IVirtualStream.h"

#include <malloc.h>

void CDriver1LevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	CBaseLevelRegion::FreeAll();

	if (m_cells)
		free(m_cells);
	m_cells = nullptr;
}

void CDriver1LevelRegion::LoadRegionData(const SPOOL_CONTEXT& ctx)
{
	IVirtualStream* pFile = ctx.dataStream;

	DevMsg(SPEW_NORM, "---------\nSpool %d %d\n", m_regionX, m_regionZ);
	DevMsg(SPEW_NORM, " - offset: %d\n", m_spoolInfo->offset);

	for (int i = 0; i < m_spoolInfo->num_connected_areas && i < 2; i++)
		DevMsg(SPEW_NORM, " - connected area %d: %d\n", i, m_spoolInfo->connected_areas[i]);

	DevMsg(SPEW_NORM, " - pvs_size: %d\n", m_spoolInfo->pvs_size);
	DevMsg(SPEW_NORM, " - cell_data_size: %d %d %d\n", m_spoolInfo->cell_data_size[0], m_spoolInfo->cell_data_size[1], m_spoolInfo->cell_data_size[2]);

	DevMsg(SPEW_NORM, " - super_region: %d\n", m_spoolInfo->super_region);

	// LoadRegionData - calculate offsets
	DevMsg(SPEW_NORM, " - cell pointers size: %d\n", m_spoolInfo->cell_data_size[1]);
	DevMsg(SPEW_NORM, " - cell data size: %d\n", m_spoolInfo->cell_data_size[0]);
	DevMsg(SPEW_NORM, " - cell objects size: %d\n", m_spoolInfo->cell_data_size[2]);
	DevMsg(SPEW_NORM, " - PVS data size: %d\n", m_spoolInfo->pvs_size);
	DevMsg(SPEW_NORM, " - roadmap data size: %dx%d\n", m_spoolInfo->roadm_size, m_spoolInfo->roadh_size);

	//
	// Driver 1 use CELL_OBJECTS directly - 16 bytes, wasteful in RAM
	//

	int cellPointersOffset = m_spoolInfo->offset + m_spoolInfo->roadm_size + m_spoolInfo->roadh_size; // SKIP road map
	int cellDataOffset = cellPointersOffset + m_spoolInfo->cell_data_size[1];
	int cellObjectsOffset = cellDataOffset + m_spoolInfo->cell_data_size[0];
	int pvsDataOffset = cellObjectsOffset + m_spoolInfo->cell_data_size[2]; // FIXME: is it even there in Driver 1?

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
		m_cells = (CELL_DATA_D1*)malloc(m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cells, m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

		// read cell objects
		m_cellObjects = (CELL_OBJECT*)malloc(m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE * 2);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellObjectsOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cellObjects, m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE, sizeof(char));
	}
	else
		MsgError("BAD PACKED CELL POINTER DATA, region = %d\n", m_regionNumber);

	delete [] packed_cell_pointers;

	// even if error occured we still need it to be here
	m_loaded = true;

	m_owner->OnRegionLoaded(this);

	// TODO: PVS and heightmap data
}

//----------------------------------------
// cell iterator
CELL_OBJECT* CDriver1LevelRegion::StartIterator(CELL_ITERATOR_D1* iterator, int cellNumber) const
{
	ushort cell_ptr = m_cellPointers[cellNumber];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	// get the packed cell data start and near cell
	CELL_DATA_D1& cell = m_cells[cell_ptr];
	CELL_OBJECT* pco = GetCellObject(cell.num & 0x3fff);

	iterator->region = (CDriver1LevelRegion*)this;
	iterator->pcd = &cell;
	iterator->pco = pco;

	return pco;
}

//-------------------------------------------------------------------------------------------

void CDriver1LevelMap::FreeAll()
{
	int total_regions = m_regions_across * m_regions_down;

	if (m_regions)
	{
		for (int i = 0; i < total_regions; i++)
			m_regions[i].FreeAll();
	}

	delete[] m_regions;
	m_regions = nullptr;

	CBaseLevelMap::FreeAll();
}

//-------------------------------------------------------------
// Loads map lump, Driver 2 version
//-------------------------------------------------------------
void CDriver1LevelMap::LoadMapLump(IVirtualStream* pFile)
{
	CBaseLevelMap::LoadMapLump(pFile);

	// read straddlers
	// Driver 1 CO
	m_straddlers = new CELL_OBJECT[m_numStraddlers];
	pFile->Read(m_straddlers, m_numStraddlers, sizeof(CELL_OBJECT));
}

//-------------------------------------------------------------
// Load spool info, Driver 2 version
//-------------------------------------------------------------
void CDriver1LevelMap::LoadSpoolInfoLump(IVirtualStream* pFile)
{
	CBaseLevelMap::LoadSpoolInfoLump(pFile);

	// Init regions
	const int total_regions = m_regions_across * m_regions_down;

	m_regions = new CDriver1LevelRegion[total_regions];

	for (int i = 0; i < total_regions; i++)
		InitRegion(&m_regions[i], i);
}

CBaseLevelRegion* CDriver1LevelMap::GetRegion(const XZPAIR& cell) const
{
	// lookup region
	const int region_x = cell.x / m_mapInfo.region_size;
	const int region_z = cell.z / m_mapInfo.region_size;

	return GetRegion(region_x + region_z * m_regions_across);;
}

CBaseLevelRegion* CDriver1LevelMap::GetRegion(int regionIdx) const
{
#if 0
	const int total_regions = m_regions_across * m_regions_down;

	if (regionIdx < 0 && regionIdx >= total_regions)
		return nullptr;
#endif
	return &m_regions[regionIdx];
}

void CDriver1LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell)
{
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(cell);

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

void CDriver1LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx)
{
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(regionIdx);

	if (region && !region->m_loaded)
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

extern sdPlane g_defaultPlane;

int	CDriver1LevelMap::MapHeight(const VECTOR_NOPAD& position) const
{
	MsgWarning("UNIMPLEMENTED MapHeight for D1\n");
	return 0;
}

int CDriver1LevelMap::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane** outPlane) const
{
	{
		*outPlane = &g_defaultPlane;

		outPoint.vx = position.vx;
		outPoint.vz = position.vz;
		outPoint.vy = 0;

		if (*outPlane == NULL || (*outPlane)->b == 0)
		{
			outNormal.vx = 0;
			outNormal.vy = 4096;
			outNormal.vz = 0;
		}
		else
		{
			outNormal.vx = (int)(*outPlane)->a >> 2;
			outNormal.vy = (int)(*outPlane)->b >> 2;
			outNormal.vz = (int)(*outPlane)->c >> 2;
		}
	}
	return 4096;
}

//-------------------------------------------------------------
// returns first cell object of cell
//-------------------------------------------------------------
CELL_OBJECT* CDriver1LevelMap::GetFirstCop(CELL_ITERATOR_D1* iterator, const XZPAIR& cell) const
{
	// lookup region
	const int region_x = cell.x / m_mapInfo.region_size;
	const int region_z = cell.z / m_mapInfo.region_size;

	int regionIdx = region_x + region_z * m_regions_across;

	CDriver1LevelRegion& region = m_regions[regionIdx];

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

	// get the packed cell data start and near cell
	CELL_DATA_D1& pcd = region.m_cells[cell_ptr];
	
	CELL_OBJECT* pco = region.GetCellObject(pcd.num & 0x3fff);

	iterator->pcd = &pcd;
	iterator->pco = pco;

	return pco;
}

//-------------------------------------------------------------
// iterates cell objects
//-------------------------------------------------------------
CELL_OBJECT* CDriver1LevelMap::GetNextCop(CELL_ITERATOR_D1* iterator) const
{
	ushort cell_ptr = iterator->pcd->next_ptr;

	if(cell_ptr != 0xFFFF)
	{
		CDriver1LevelRegion* region = iterator->region;
		cell_ptr -= m_cell_slots_add[region->m_regionBarrelNumber];
		
		// get the packed cell data start and near cell
		CELL_DATA_D1& cell = region->m_cells[cell_ptr];
		iterator->pcd = &cell;
		
		CELL_OBJECT* pco = region->GetCellObject(cell.num & 0x3fff);
		iterator->pco = pco;

		return pco;
	}

	return nullptr;
}
#include "core/core_common.h"
#include "regions_d1.h"
#include "level.h"

#include "math/isin.h"

extern sdPlane g_defaultPlane;
extern sdPlane g_seaPlane;

void CDriver1LevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	CBaseLevelRegion::FreeAll();

	if (m_cells)
		PPFree(m_cells);
	m_cells = nullptr;

	SAFE_DELETE_ARRAY(m_roadMap);
	SAFE_DELETE_ARRAY(m_surfaceRoads);
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

	const int roadMOffset = m_spoolInfo->offset;
	const int roadHOffset = roadMOffset + m_spoolInfo->roadm_size;
	const int cellPointersOffset = roadHOffset + m_spoolInfo->roadh_size;
	const int cellDataOffset = cellPointersOffset + m_spoolInfo->cell_data_size[1];
	const int cellObjectsOffset = cellDataOffset + m_spoolInfo->cell_data_size[0];
	const int pvsDataOffset = cellObjectsOffset + m_spoolInfo->cell_data_size[2]; // FIXME: is it even there in Driver 1?

	// read roadm (map?)
	pFile->Seek(ctx.lumpInfo->spooled_offset + roadMOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	LoadRoadCellsData(pFile);

	// read roadh (heights?)
	pFile->Seek(ctx.lumpInfo->spooled_offset + roadHOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	LoadRoadHeightMapData(pFile);

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
		m_cells = (CELL_DATA_D1*)PPAlloc(m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cells, m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

		// read cell objects
		m_cellObjects = (CELL_OBJECT*)PPAlloc(m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE * 2);
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

void CDriver1LevelRegion::LoadRoadHeightMapData(IVirtualStream* pFile)
{
	const OUT_CELL_FILE_HEADER& mapInfo = m_owner->m_mapInfo;

	int roadMapSize = (m_spoolInfo->roadm_size + m_spoolInfo->roadh_size) * SPOOL_CD_BLOCK_SIZE;

	char* roadMapData = new char[m_owner->m_PVS_size[m_regionBarrelNumber]];
	pFile->Read(roadMapData, m_spoolInfo->roadh_size * SPOOL_CD_BLOCK_SIZE, sizeof(char));

	// roadh needs to be post-processed
	const int double_region_size = mapInfo.region_size * 2;
	int i = double_region_size * double_region_size;

	// road map is in cell size
	m_roadMap = new uint[double_region_size * double_region_size];
	memset(m_roadMap, 0, sizeof(m_roadMap));

	uint* src = (uint*)roadMapData;
	uint* pRoadMap = m_roadMap;
	do {
		uint len = *src;
		uint value = src[1];

		src += 2;
		i -= len;

		while (len != 0)
		{
			*pRoadMap++ = value;
			len--;
		}
	} while (i != 0);

	// not needed anymore since roadm and roadh are converted
	delete[] roadMapData;
}

void CDriver1LevelRegion::LoadRoadCellsData(IVirtualStream* pFile)
{
	m_surfaceRoads = new ushort[ROAD_MAP_REGION_CELLS];
	ushort* pRoadIds = m_surfaceRoads;
	int i = ROAD_MAP_REGION_CELLS;

	do {
		short length;
		pFile->Read(&length, 1, sizeof(short));

		if (length == -1)
			break;

		ushort value;
		pFile->Read(&value, 1, sizeof(short));

		int count = length;
		for (i = i - count; count > 0; count--)
		{
			*pRoadIds++ = value;
		}
	} while (i > 0);
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

	PPFree(m_surfaceData);
	m_surfaceData = nullptr;

	SAFE_DELETE_ARRAY(m_regions);
	SAFE_DELETE_ARRAY(m_roads);
	SAFE_DELETE_ARRAY(m_roadBounds);
	SAFE_DELETE_ARRAY(m_junctions);
	SAFE_DELETE_ARRAY(m_junctionBounds);

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

// or NewProcessRoadMapLump in D1 & D2
void CDriver1LevelMap::LoadRoadMapLump(IVirtualStream* pFile)
{
	int width, height;
	pFile->Read(&width, 1, sizeof(int));
	pFile->Read(&height, 1, sizeof(int));

	m_roadMapLumpData.width = width;
	m_roadMapLumpData.height = height;

	m_roadMapLumpData.unitXMid = (width + 1) * 750;
	m_roadMapLumpData.unitZMid = height * 750;

	// from PC reversing
	//m_roadMapLumpData.unitXMid = 1500 * width / 2;
	//m_roadMapLumpData.unitZMid = 1500 * height / 2;
}

void CDriver1LevelMap::LoadRoadsLump(IVirtualStream* pFile)
{
	pFile->Read(&m_numRoads, 1, sizeof(int));
	m_roads = new DRIVER1_ROAD[m_numRoads];
	pFile->Read(m_roads, m_numRoads, sizeof(DRIVER1_ROAD));
}

void CDriver1LevelMap::LoadJunctionsLump(IVirtualStream* pFile)
{
	pFile->Read(&m_numJunctions, 1, sizeof(int));
	m_junctions = new DRIVER1_JUNCTION[m_numJunctions];
	pFile->Read(m_junctions, m_numJunctions, sizeof(DRIVER1_JUNCTION));
}

void CDriver1LevelMap::LoadRoadBoundsLump(IVirtualStream* pFile)
{
	int numRoadBounds;
	pFile->Read(&numRoadBounds, 1, sizeof(int));
	m_roadBounds = new DRIVER1_ROADBOUNDS[numRoadBounds];
	pFile->Read(m_roadBounds, numRoadBounds, sizeof(DRIVER1_ROADBOUNDS));
}

void CDriver1LevelMap::LoadJuncBoundsLump(IVirtualStream* pFile)
{
	int numJuncBounds;
	pFile->Read(&numJuncBounds, 1, sizeof(int));
	m_junctionBounds = new XYPAIR[numJuncBounds];
	pFile->Read(m_junctionBounds, numJuncBounds, sizeof(XYPAIR));
}

void CDriver1LevelMap::LoadRoadSurfaceLump(IVirtualStream* pFile, int size)
{
	int numSurfaces;

	m_surfaceData = (char*)PPAlloc(size);
	pFile->Read(m_surfaceData, 1, size);

	// get the surface count
	numSurfaces = *(int*)m_surfaceData;

	memset(m_surfacePtrs, 0, numSurfaces * sizeof(SURFACEINFO*));

	// then flatten linked list into array
	SURFACEINFO* si = (SURFACEINFO*)(m_surfaceData + sizeof(int));
	for (int i = 0; i < numSurfaces; i++)
	{
		m_surfacePtrs[si->type] = si;
		si = (SURFACEINFO*)si->GetPoly(si->numpolys);
	}
}

CBaseLevelRegion* CDriver1LevelMap::GetRegion(const XZPAIR& cell) const
{
	// lookup region
	const int region_x = cell.x / m_mapInfo.region_size;
	const int region_z = cell.z / m_mapInfo.region_size;

	if (region_x < 0 ||
		region_z < 0 ||
		region_x >= m_regions_across ||
		region_z >= m_regions_down)
		return nullptr;

	return GetRegion(region_x + region_z * m_regions_across);
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

bool CDriver1LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell)
{
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(cell);

	if (!region->m_loaded)
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

bool CDriver1LevelMap::SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx)
{
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(regionIdx);

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

static int PointInTri2d(int tx, int ty, const XYPAIR* verts)
{
	// from PC reversing
	if ((verts[1].y - verts[0].y) * (tx - verts[0].x) - (verts[1].x - verts[0].x) * (ty - verts[0].y) < 0)
		return 0;

	if ((verts[2].y - verts[1].y) * (tx - verts[1].x) - (verts[2].x - verts[1].x) * (ty - verts[1].y) >= 0)
		return (verts[0].y - verts[2].y) * (tx - verts[2].x) - (verts[0].x - verts[2].x) * (ty - verts[2].y) >= 0;

	return 0;
}

static int PointInQuad2d(int tx, int ty, const XYPAIR* verts)
{
	// from PC reversing
	if ((tx - verts[0].x) * (verts[1].y - verts[0].y) -
		(ty - verts[0].y) * (verts[1].x - verts[0].x) < 0)
		return 0;

	if ((tx - verts[1].x) * (verts[2].y - verts[1].y) -
		(ty - verts[1].y) * (verts[2].x - verts[1].x) < 0)
		return 0;

	if ((tx - verts[2].x) * (verts[3].y - verts[2].y) -
		(ty - verts[2].y) * (verts[3].x - verts[2].x) < 0)
		return 0;

	if ((tx - verts[3].x) * (verts[0].y - verts[3].y) -
		(ty - verts[3].y) * (verts[0].x - verts[3].x) < 0)
		return 0;

	return 1;
}

bool CDriver1LevelMap::GetRoadInfo(ROUTE_DATA& outData, const VECTOR_NOPAD& position) const
{
	const int road_region_size = m_mapInfo.cell_size / 1500 * m_mapInfo.region_size;

	XZPAIR cpos;
	cpos.x = (m_roadMapLumpData.unitXMid + position.vx) / 1500;
	cpos.z = (m_roadMapLumpData.unitZMid - (position.vz - 750)) / 1500;

	// from PC reversing
	//cpos.x = (position_x + RoadMapUnitXMid + 750) / 1500;
	//cpos.z = (RoadMapUnitZMid + 750 - position_z) / 1500;

	if (cpos.x < 0 ||
		cpos.z < 0 ||
		cpos.x >= m_roadMapLumpData.width ||
		cpos.z >= m_roadMapLumpData.height)
	{
		return false;
	}

	// weird hack. But it's actually working
	VECTOR_NOPAD cposition = position;
	cposition.vx += 750;
	cposition.vz -= 750;

	XZPAIR cell;
	WorldPositionToCellXZ(cell, cposition);
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(cell);

	if (!region || !region->m_roadMap)
	{
		return false;
	}

	const int cellIdx = (cpos.x % road_region_size) + (cpos.z % road_region_size) * road_region_size;

	const uint value = region->m_roadMap[cellIdx];

	outData.height = *(short*)&value;
	outData.type = value >> 16 & 1023;
	outData.objectAngle = (value >> 30) * 1024;
	outData.value = value;
	outData.roadIndex = region->m_surfaceRoads[cellIdx];

	return true;
}

static void RotatePoint(const ROUTE_DATA& routeData, int& px, int& py)
{
	// NOTE: if you uncommend PC reversing code, it breaks rotation
	if ((routeData.objectAngle & 0x400) == 0)
	{
		int tmp = -px;
		px = py;
		py = tmp;
	}

	if ((routeData.objectAngle & 0x800) != 0)
	{
		px = -px;
		py = -py;
	}
}

static void RotateNormal(const ROUTE_DATA& routeData, int& nx, int& ny)
{
	// NOTE: if you uncommend PC reversing code, it breaks rotation
	if ((routeData.objectAngle & 0x400) != 0)
	{
		int tmp = ny;
		ny = -nx;
		nx = tmp;
	}

	if ((routeData.objectAngle & 0x800) == 0)
	{
		nx = -nx;
		ny = -ny;
	}
}

void CDriver1LevelMap::GetSurfaceLocalCoords(const VECTOR_NOPAD& position, int& px, int& py) const
{
	XZPAIR cpos;
	cpos.x = m_roadMapLumpData.unitXMid + position.vx;
	cpos.z = m_roadMapLumpData.unitZMid - (position.vz - 750);

	py = cpos.x % 1500 - 750;
	px = cpos.z % 1500 - 750;

	// from PC reversing
	//px = 750 - abs((position.vx + m_roadMapLumpData.unitXMid + 750) % 1500);
	//py = 750 - abs((m_roadMapLumpData.unitZMid + 750 - position.vz) % 1500);
}

// on PSX San Francisco has a WEIRD hack
static int GetHardcodedFriscoHeight(const VECTOR_NOPAD& position)
{
	if (position.vx > -345000 && position.vx < -328500 && 
		position.vz < 267000 && position.vz > 250500)
	{
		return -1500;
	}

	return 0;
}

void CDriver1LevelMap::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const
{
	outPlane = g_defaultPlane;

	outPoint.vx = position.vx;
	outPoint.vz = position.vz;
	outPoint.vy = outPlane.d;

	// oh, there is a trouble with D1...
	ROUTE_DATA routeData;
	if (!GetRoadInfo(routeData, position))
	{
		return;
	}

	outPlane.d = -routeData.height;

	outPoint.vx = position.vx;
	outPoint.vz = position.vz;
	outPoint.vy = -routeData.height;

	// check collision
	if (routeData.type < 900)
	{
		const SURFACEINFO* si = m_surfacePtrs[routeData.type];

		if (si)
		{
			int px, py;
			GetSurfaceLocalCoords(position, px, py);
			RotatePoint(routeData, px, py);

			for (int i = 0; i < si->numpolys; i++)
			{
				const SIPOLY* poly = si->GetPoly(i);

				int res = (poly->num_vertices == 4) ?
					PointInQuad2d(px, py, poly->xz) :
					PointInTri2d(px, py, poly->xz);

				if (res != 0)
				{
					const int normFac = px * poly->normals.a + py * poly->normals.c;
					const int height = ((normFac / ONE) - poly->normals.d) - routeData.height;

					int n_vx = poly->normals.a;
					int n_vz = poly->normals.c;
					const int n_vy = poly->normals.b;

					RotateNormal(routeData, n_vx, n_vz);

					outPlane.a = n_vx << 2;
					outPlane.b = n_vy << 2;
					outPlane.c = n_vz << 2;

					outPoint.vy = height;
				}
			}
		} // si != null

		ModelRef_t* ref = m_models->GetModelByIndex(routeData.type);

		if (ref && ref->baseInstance)
			ref = ref->baseInstance;

		MODEL* model = ref ? ref->model : nullptr;

		if (model)
		{
			if (model->shape_flags & SHAPE_FLAG_WATER)
				outPlane.surfaceType = (int)SurfaceType::Water;

			if (model->flags2 & MODEL_FLAG_GRASS)
				outPlane.surfaceType = (int)SurfaceType::Grass;
		}
	}
#if 0
	// FRISCO hack
	if (position.vx > -345000 && position.vx < -328500 && position.vz < 267000 && position.vz > 250500)
	{
		outPoint.vy = -1500;
	}
#endif
}

int	CDriver1LevelMap::GetSurfaceIndex(const VECTOR_NOPAD& position) const
{
	ROUTE_DATA routeData;
	if (!GetRoadInfo(routeData, position))
	{
		return -32;
	}

	if (routeData.roadIndex >= SHRT_MAX)
		return -32;

	return routeData.roadIndex;
}

//-------------------------------------------------------------
// returns first cell object of cell
//-------------------------------------------------------------
CELL_OBJECT* CDriver1LevelMap::GetFirstCop(CELL_ITERATOR_D1* iterator, const XZPAIR& cell) const
{
	// lookup region
	CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(cell);

	iterator->region = region;

	// don't do anything on empty or non-spooled regions
	if (!region->m_cells)
		return nullptr;

	// get cell index on the region
	const int region_cell_x = cell.x % m_mapInfo.region_size;
	const int region_cell_z = cell.z % m_mapInfo.region_size;

	// FIXME: might be incorrect
	const int cell_index = region_cell_x + region_cell_z * m_mapInfo.region_size;

	ushort cell_ptr = region->m_cellPointers[cell_index];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	// get the packed cell data start and near cell
	CELL_DATA_D1* pcd = &region->m_cells[cell_ptr];
	
	iterator->pcd = pcd;

	CELL_OBJECT* pco = region->GetCellObject(pcd->num & 16383);

	if (iterator->cache)
	{
		CELL_ITERATOR_CACHE* cache = iterator->cache;
		ushort num = pcd->num & 16383;
		uint value = 1 << (num & 7);

		if ((cache->computedValues[num / 8] & value))
		{
			pco = GetNextCop(iterator);
			iterator->pco = pco;

			return pco;
		}

		cache->computedValues[num / 8] |= value;
	}

	iterator->pco = pco;

	return pco;
}

//-------------------------------------------------------------
// iterates cell objects
//-------------------------------------------------------------
CELL_OBJECT* CDriver1LevelMap::GetNextCop(CELL_ITERATOR_D1* iterator) const
{
	CELL_OBJECT* pco = nullptr;

	do
	{
		CDriver1LevelRegion* region = iterator->region;

		CELL_DATA_D1* pcd = iterator->pcd;

		do
		{
			ushort cell_ptr = pcd->next_ptr;

			if (cell_ptr == 0xffff)
				return nullptr;

			// get the packed cell data start and near cell
			pcd = &region->m_cells[cell_ptr - m_cell_slots_add[region->m_regionBarrelNumber]];

			iterator->pcd = pcd;

			pco = region->GetCellObject(pcd->num & 16383);
			iterator->pco = pco;
		} while (pco->pos.vx == -1 && pco->pos.vy == -1 && pco->pos.vz == -1 && pco->type == 0xffff);

		if (iterator->cache)
		{
			ushort num = pcd->num & 16383;
			CELL_ITERATOR_CACHE* cache = iterator->cache;

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

	return pco;
}
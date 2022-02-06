#include "regions_d1.h"

#include "level.h"
#include <string.h>
#include "core/cmdlib.h"
#include "core/IVirtualStream.h"
#include "math/isin.h"

void CDriver1LevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	CBaseLevelRegion::FreeAll();

	if (m_cells)
		Memory::free(m_cells);
	m_cells = nullptr;

	delete[] m_roadMap;
	m_roadMap = nullptr;
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
	//pFile->Seek(ctx.lumpInfo->spooled_offset + roadMOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
	//pFile->Read(roadMapData, m_spoolInfo->roadm_size * SPOOL_CD_BLOCK_SIZE, sizeof(char));

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
		m_cells = (CELL_DATA_D1*)Memory::alloc(m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE);
		pFile->Seek(ctx.lumpInfo->spooled_offset + cellDataOffset * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);
		pFile->Read(m_cells, m_spoolInfo->cell_data_size[0] * SPOOL_CD_BLOCK_SIZE, sizeof(char));

		// read cell objects
		m_cellObjects = (CELL_OBJECT*)Memory::alloc(m_spoolInfo->cell_data_size[2] * SPOOL_CD_BLOCK_SIZE * 2);
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
		int len = *src;
		int value = src[1];

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

	Memory::free(m_surfaceData);
	m_surfaceData = nullptr;

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
}

void CDriver1LevelMap::LoadRoadSurfaceLump(IVirtualStream* pFile, int size)
{
	int numSurfaces;

	m_surfaceData = (char*)Memory::alloc(size);
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

int PointInTri2d(int tx, int ty, XYPAIR* verts)
{
#if 1
	// D1 PC version
	if ((verts[1].y - verts[0].y) * (tx - verts[0].x) - (verts[1].x - verts[0].x) * (ty - verts[0].y) < 0)
		return 0;

	if ((verts[2].y - verts[1].y) * (tx - verts[1].x) - (verts[2].x - verts[1].x) * (ty - verts[1].y) >= 0)
		return (verts[0].y - verts[2].y) * (tx - verts[2].x) - (verts[0].x - verts[2].x) * (ty - verts[2].y) >= 0;

	return 0;
#else
	// D1 PSX version
	int iVar1, iVar2, iVar3;
	int iVar4, iVar5, iVar6;

	iVar6 = verts[0].x;
	iVar5 = verts[0].y;

	iVar4 = verts[1].y;
	iVar2 = verts[1].x;

	iVar3 = verts[2].y;
	iVar1 = verts[2].x;

	if ((tx - iVar6) * (iVar4 - iVar5) - (ty - iVar5) * (iVar2 - iVar6) > -1)
	{
		if ((tx - iVar2) * (iVar3 - iVar4) - (ty - iVar4) * (iVar1 - iVar2) > -1)
		{
			return (uint)~((tx - iVar1) * (iVar5 - iVar3) - (ty - iVar3) * (iVar6 - iVar1)) >> 0x1f;
		}
	}
	return 0;
#endif
}

int PointInQuad2d(int tx, int ty, XYPAIR* verts)
{
#if 1
	// D1 PC version
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
	else
		return 1;
#else
	// D1 PSX version
	int ret;
	int bMaxX, bMaxY;
	int _bMinX, _bMinY;
	int bMinX, bMinY;

	_bMinX = verts[0].x;
	bMaxX = verts[2].x;

	bMinX = _bMinX;

	if (_bMinX < bMaxX)
	{
		bMinX = bMaxX;
		bMaxX = _bMinX;
	}

	ret = 0;

	if ((bMaxX <= tx) && (ret = 0, tx <= bMinX))
	{
		_bMinY = verts[0].y;
		bMaxY = verts[2].y;

		bMinY = _bMinY;

		if (_bMinY < bMaxY)
		{
			bMinY = bMaxY;
			bMaxY = _bMinY;
		}

		ret = 0;

		if ((bMaxY <= ty) && (ret = 0, ty <= bMinY))
		{
			ret = 1;
		}
	}
	return ret;
#endif
}

bool CDriver1LevelMap::GetRoadInfo(ROUTE_DATA& outData, const VECTOR_NOPAD& position) const
{
	const int double_region_size = m_mapInfo.region_size * 2;

	XZPAIR cpos;
	cpos.x = m_roadMapLumpData.unitXMid + position.vx;
	cpos.z = m_roadMapLumpData.unitZMid - (position.vz - 750);

	int cell_x = cpos.x / 1500;
	int cell_z = cpos.z / 1500;

	if (cell_x < m_roadMapLumpData.width && cell_z < m_roadMapLumpData.height)
	{
		int loc_x, loc_z;

		// does it have any effect?
		// cell_x % double_region_size;
		// cell_z % double_region_size;
		loc_x = ((cell_x & 127) + ((int)(cell_x & 0xffffff80) >> 4)) % double_region_size;
		loc_z = ((cell_z & 127) + ((int)(cell_z & 0xffffff80) >> 4)) % double_region_size;

		XZPAIR cell;
		VECTOR_NOPAD cposition = position;

		// weird hack. But it's actually working
		cposition.vx += 750;
		cposition.vz -= 750;

		WorldPositionToCellXZ(cell, cposition);
		CDriver1LevelRegion* region = (CDriver1LevelRegion*)GetRegion(cell);

		if (region && region->m_roadMap)
		{
			uint value = region->m_roadMap[loc_x + loc_z * double_region_size];

			outData.height = *(short*)&value;
			outData.type = value >> 16 & 0x3ff;
			outData.objectAngle = value >> 30;
			outData.value = value;

			return true;
		}
	}

	return false;
}

int	CDriver1LevelMap::MapHeight(const VECTOR_NOPAD& position) const
{
	ROUTE_DATA routeData;
	if (GetRoadInfo(routeData, position))
	{
		// check collision
		if (routeData.type < 900)
		{
			SURFACEINFO* si = m_surfacePtrs[routeData.type];

			if (si)
			{
				XZPAIR cpos;
				cpos.x = m_roadMapLumpData.unitXMid + position.vx;
				cpos.z = m_roadMapLumpData.unitZMid - (position.vz - 750);

				int py = cpos.x % 1500 - 750;
				int px = cpos.z % 1500 - 750;

				if ((routeData.value & 0x40000000) == 0)
				{
					int tmp = -px;
					px = py;
					py = tmp;
				}

				if ((int)routeData.value < 0)
				{
					px = -px;
					py = -py;
				}

				for (int i = 0; i < si->numpolys; i++)
				{
					SIPOLY* poly = si->GetPoly(i);

					int res = (poly->num_vertices == 4) ?
						PointInQuad2d(px, py, poly->xz) :
						PointInTri2d(px, py, poly->xz);

					if (res != 0)
					{
						int normFac = px * poly->normals.a + py * poly->normals.c;

						int _height = ((normFac / ONE) - poly->normals.d) - routeData.height;

						return _height;
					}
				}
			}
		}

		return -routeData.height;
	}

	return 0;
}

int CDriver1LevelMap::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const
{
	// oh, there is a trouble with D1...
	ROUTE_DATA routeData;
	if (GetRoadInfo(routeData, position))
	{
		outNormal.vx = 0;
		outNormal.vy = 4096;
		outNormal.vz = 0;

		outPoint.vx = position.vx;
		outPoint.vz = position.vz;
		outPoint.vy = -routeData.height;

		outPlane.surfaceType = 0;

		// check collision
		if (routeData.type < 900)
		{
			ModelRef_t* ref = m_models->GetModelByIndex(routeData.type);

			if (ref && ref->baseInstance)
				ref = ref->baseInstance;

			MODEL* model = ref ? ref->model : nullptr;

			SURFACEINFO* si = m_surfacePtrs[routeData.type];

			if (si)
			{
				XZPAIR cpos;
				cpos.x = m_roadMapLumpData.unitXMid + position.vx;
				cpos.z = m_roadMapLumpData.unitZMid - (position.vz - 750);

				int py = cpos.x % 1500 - 750;
				int px = cpos.z % 1500 - 750;

				if ((routeData.value & 0x40000000) == 0)
				{
					int tmp = -px;
					px = py;
					py = tmp;
				}

				if ((int)routeData.value < 0)
				{
					px = -px;
					py = -py;
				}

				for (int i = 0; i < si->numpolys; i++)
				{
					SIPOLY* poly = si->GetPoly(i);

					int res = (poly->num_vertices == 4) ?
						PointInQuad2d(px, py, poly->xz) :
						PointInTri2d(px, py, poly->xz);

					if (res != 0)
					{
						int normFac = px * poly->normals.a + py * poly->normals.c;

						int _height = ((normFac / ONE) - poly->normals.d) - routeData.height;

						int n_vx = poly->normals.a;
						int _n_vz = poly->normals.c;
						int n_vy = poly->normals.b;

						int n_vz = _n_vz;

						if ((routeData.value & 0x40000000) != 0)
						{
							n_vz = -n_vx;
							n_vx = _n_vz;
						}

						if ((routeData.value & 0x80000000) == 0)
						{
							n_vx = -n_vx;
							n_vz = -n_vz;
						}

						outNormal.vx = n_vx;
						outNormal.vy = n_vy;
						outNormal.vz = n_vz;

						outPoint.vy = _height;
					}
				}
			} // si != null

			if (model)
			{
				if (model->shape_flags & SHAPE_FLAG_WATER)
					outPlane.surfaceType = (int)SurfaceType::Water;

				if (model->flags2 & MODEL_FLAG_GRASS)
					outPlane.surfaceType = (int)SurfaceType::Grass;
			}
		}

		return 4096;
	}

	return 4096;
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
	int cell_index = region_cell_x + region_cell_z * m_mapInfo.region_size;

	ushort cell_ptr = region->m_cellPointers[cell_index];

	if (cell_ptr == 0xFFFF)
		return nullptr;

	// get the packed cell data start and near cell
	CELL_DATA_D1* pcd = &region->m_cells[cell_ptr];
	
	iterator->pcd = pcd;

	CELL_OBJECT* pco = region->GetCellObject(pcd->num & 0x3fff);

	if (iterator->cache)
	{
		CELL_ITERATOR_CACHE* cache = iterator->cache;
		ushort num = pcd->num;
		uint value = 1 << (num & 7) & 0xffff;

		if ((cache->computedValues[(num & 0x3fff) >> 3] & value))
		{
			pco = GetNextCop(iterator);
			iterator->pco = pco;

			return pco;
		}

		cache->computedValues[(num & 0x3fff) >> 3] |= value;
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

			pco = region->GetCellObject(pcd->num & 0x3fff);
			iterator->pco = pco;
		} while (pco->pos.vx == -1 && pco->pos.vy == -1 && pco->pos.vz == -1 && pco->type == 0xffff);

		if (!iterator->cache)
			break;

		ushort num = pcd->num;

		CELL_ITERATOR_CACHE* cache = iterator->cache;

		uint value = 1 << (num & 7) & 0xffff;

		if ((cache->computedValues[(num & 0x3fff) >> 3] & value) == 0)
		{
			cache->computedValues[(num & 0x3fff) >> 3] |= value;
			break;
		}
	} while (true);



	return pco;
}
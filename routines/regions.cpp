#include "regions.h"

#include <string.h>
#include <malloc.h>

#include "models.h"
#include "textures.h"

#include "core/IVirtualStream.h"
#include "core/cmdlib.h"

// loaded headers
CBaseLevelRegion::CBaseLevelRegion()
{
}

CBaseLevelRegion::~CBaseLevelRegion()
{
	FreeAll();
}

void CBaseLevelRegion::FreeAll()
{
	if (!m_loaded)
		return;

	m_owner->OnRegionFreed(this);
	
	// do I need that?
	if(m_spoolInfo && m_spoolInfo->super_region != 0xFF)
	{
		int areaDataNum = m_spoolInfo->super_region;
		int numAreaTpages = m_owner->m_areaData[areaDataNum].num_tpages;
		AreaTpageList& areaTPages = m_owner->m_areaTPages[areaDataNum];

		for (int i = 0; numAreaTpages; i++)
		{
			if (areaTPages.pageIndexes[i] == 0xFF)
				break;

			if (areaTPages.tpage[i])
			{
				areaTPages.tpage[i]->FreeBitmap();
				areaTPages.tpage[i] = nullptr;
			}
		}
	}
	m_spoolInfo = nullptr;

	delete[] m_cellPointers;
	m_cellPointers = nullptr;
	
	m_pvsData = nullptr;	
	m_roadmapData = nullptr;

	m_loaded = false;
}

bool CBaseLevelRegion::IsEmpty() const
{
	return m_spoolInfo == nullptr;
}

int	CBaseLevelRegion::GetNumber() const
{
	return m_regionNumber;
}

//-------------------------------------------------------------
// Region unpacking function
//-------------------------------------------------------------
int CBaseLevelRegion::UnpackCellPointers(ushort* dest_ptrs, char* src_data, int cell_slots_add, int targetRegion)
{
	ushort cell;
	ushort* short_ptr;
	ushort* source_packed_data;
	int loop;
	uint bitpos;
	uint pcode;
	int numCellPointers;

	int packtype;

	packtype = *(int*)(src_data + 4);
	source_packed_data = (ushort*)(src_data + 8);

	numCellPointers = 0;

	if (packtype == 0)
	{
		short_ptr = dest_ptrs + targetRegion * 1024;

		for (loop = 0; loop < 1024; loop++)
			*short_ptr++ = 0xffff;
	}
	else if (packtype == 1)
	{
		short_ptr = dest_ptrs + targetRegion * 1024;

		for (loop = 0; loop < 1024; loop++)
		{
			cell = *source_packed_data++;

			if (cell != 0xffff)
				cell += cell_slots_add;

			*short_ptr++ = cell;
			numCellPointers++;
		}
	}
	else if (packtype == 2)
	{
		bitpos = 0x8000;
		pcode = (uint)*source_packed_data;
		source_packed_data++;
		short_ptr = dest_ptrs + targetRegion * 1024;

		for (loop = 0; loop < 1024; loop++)
		{
			if (pcode & bitpos)
			{
				cell = *source_packed_data++;
				cell += cell_slots_add;
			}
			else
				cell = 0xffff;

			bitpos >>= 1;
			*short_ptr++ = cell;
			numCellPointers++;

			if (bitpos == 0)
			{
				bitpos = 0x8000;
				pcode = *source_packed_data++;
			}
		}
	}
	else
	{
		numCellPointers = -1;
	}

	return numCellPointers;
}


void CBaseLevelRegion::LoadAreaData(const SPOOL_CONTEXT& ctx)
{
	if (!m_spoolInfo || m_spoolInfo && m_spoolInfo->super_region == 0xFF)
		return;

	int areaDataNum = m_spoolInfo->super_region;

	if (m_owner->m_areaDataStates[areaDataNum])
		return;

	m_owner->LoadInAreaTPages(ctx, areaDataNum);
	m_owner->LoadInAreaModels(ctx, areaDataNum);

	m_owner->m_areaDataStates[areaDataNum] = true;
}


//-------------------------------------------------------------------------------------------


CBaseLevelMap::CBaseLevelMap()
{
}

CBaseLevelMap::~CBaseLevelMap()
{
}

void CBaseLevelMap::FreeAll()
{
	if (m_regionSpoolInfo)
		free(m_regionSpoolInfo);
	m_regionSpoolInfo = nullptr;

	delete[] m_regionSpoolInfoOffsets;
	m_regionSpoolInfoOffsets = nullptr;

	delete[] m_areaTPages;
	m_areaTPages = nullptr;

	delete[] m_areaData;
	m_areaData = nullptr;

	delete[] m_areaDataStates;
	m_areaDataStates = nullptr;
}

int	CBaseLevelMap::GetAreaDataCount() const
{
	return m_numAreas;
}

const OUT_CELL_FILE_HEADER& CBaseLevelMap::GetMapInfo() const
{
	return m_mapInfo;
}

int	CBaseLevelMap::GetCellsAcross() const
{
	return m_mapInfo.cells_across;
}

int	CBaseLevelMap::GetCellsDown() const
{
	return m_mapInfo.cells_down;
}

int CBaseLevelMap::GetRegionsAcross() const
{
	return m_regions_across;
}

int CBaseLevelMap::GetRegionsDown() const
{
	return m_regions_down;
}

void CBaseLevelMap::WorldPositionToCellXZ(XZPAIR& cell, const VECTOR_NOPAD& position) const
{
	// @TODO: constants
	int units_across_halved = m_mapInfo.cells_across / 2 * m_mapInfo.cell_size;
	int units_down_halved = m_mapInfo.cells_down / 2 * m_mapInfo.cell_size;

	cell.x = (position.vx + units_across_halved) / m_mapInfo.cell_size;
	cell.z = (position.vz + units_down_halved) / m_mapInfo.cell_size;
}

//-------------------------------------------------------------
// parses LUMP_MAP and it's straddler objects
//-------------------------------------------------------------
void CBaseLevelMap::LoadMapLump(IVirtualStream* pFile)
{
	pFile->Read(&m_mapInfo, 1, sizeof(OUT_CELL_FILE_HEADER));

	Msg("Level dimensions[%d %d], cell size: %d\n", m_mapInfo.cells_across, m_mapInfo.cells_down, m_mapInfo.cell_size);
	DevMsg(SPEW_NORM," - num_regions: %d\n", m_mapInfo.num_regions);
	DevMsg(SPEW_NORM, " - region_size in cells: %d\n", m_mapInfo.region_size);

	int dim_x = m_mapInfo.cells_across / m_mapInfo.region_size;
	int dim_y = m_mapInfo.cells_down / m_mapInfo.region_size;

	Msg("World size:\n [%dx%d] cells\n [%dx%d] regions\n", m_mapInfo.cells_across, m_mapInfo.cells_down, dim_x, dim_y);

	DevMsg(SPEW_NORM, " - num_cell_objects : %d\n", m_mapInfo.num_cell_objects);
	DevMsg(SPEW_NORM, " - num_cell_data: %d\n", m_mapInfo.num_cell_data);

	// ProcessMapLump
	pFile->Read(&m_numStraddlers, 1, sizeof(m_numStraddlers));
	DevMsg(SPEW_NORM, " - num straddler cells: %d\n", m_numStraddlers);

	const int pvs_square = 21;
	const int pvs_square_sq = pvs_square * pvs_square;

	// initialize regions
	m_regions_across = m_mapInfo.cells_across / m_mapInfo.region_size;
	m_regions_down = m_mapInfo.cells_down / m_mapInfo.region_size;
}

//-------------------------------------------------------------
// parses LUMP_SPOOLINFO, and also loads region data
//-------------------------------------------------------------
void CBaseLevelMap::LoadSpoolInfoLump(IVirtualStream* pFile)
{
	int model_spool_buffer_size;
	pFile->Read(&model_spool_buffer_size, 1, sizeof(int));
	DevMsg(SPEW_NORM, "model_spool_buffer_size = %d * SPOOL_CD_BLOCK_SIZE\n", model_spool_buffer_size);

	int Music_And_AmbientOffsetsSize;
	pFile->Read(&Music_And_AmbientOffsetsSize, 1, sizeof(int));
	DevMsg(SPEW_NORM, "Music_And_AmbientOffsetsSize = %d\n", Music_And_AmbientOffsetsSize);

	// move further
	// this was probably used in early D1 level files for sound banks
	pFile->Seek(Music_And_AmbientOffsetsSize, VS_SEEK_CUR);

	pFile->Read(&m_numAreas, 1, sizeof(int));
	DevMsg(SPEW_NORM, "NumAreas = %d\n", m_numAreas);

	m_areaData = new AreaDataStr[m_numAreas];
	m_areaTPages = new AreaTpageList[m_numAreas];

	// read area data stream infos
	pFile->Read(m_areaData, m_numAreas, sizeof(AreaDataStr));

	// read tpage ids
	for (int i = 0; i < m_numAreas; i++)
	{
		pFile->Read(m_areaTPages[i].pageIndexes, 1, sizeof(AreaTpageList::pageIndexes));
		memset(m_areaTPages[i].tpage, 0, sizeof(AreaTpageList::tpage));
	}

	for (int i = 0; i < 5; i++)
	{
		m_cell_slots_add[i] = 0;
		m_cell_objects_add[i] = 0;
	}

	// read slots count
	for (int i = 0; i < 4; i++)
	{
		int slots_count;
		pFile->Read(&slots_count, 1, sizeof(int));
		m_cell_slots_add[i] = m_cell_slots_add[4];
		m_cell_slots_add[4] += slots_count;
	}

	// read objects count
	for (int i = 0; i < 4; i++)
	{
		int objects_count;
		pFile->Read(&objects_count, 1, sizeof(int));
		m_cell_objects_add[i] = m_cell_objects_add[4];
		m_cell_objects_add[4] += objects_count;
	}

	// read pvs sizes
	for (int i = 0; i < 4; i++)
	{
		int pvs_size;
		pFile->Read(&pvs_size, 1, sizeof(int));

		m_PVS_size[i] = pvs_size + 2047 & 0xfffff800;
	}

	DevMsg(SPEW_NORM, "cell_slots_add = {%d,%d,%d,%d}\n", m_cell_slots_add[0], m_cell_slots_add[1], m_cell_slots_add[2], m_cell_slots_add[3]);
	DevMsg(SPEW_NORM, "cell_objects_add = {%d,%d,%d,%d}\n", m_cell_objects_add[0], m_cell_objects_add[1], m_cell_objects_add[2], m_cell_objects_add[3]);
	DevMsg(SPEW_NORM, "PVS_size = {%d,%d,%d,%d}\n", m_PVS_size[0], m_PVS_size[1], m_PVS_size[2], m_PVS_size[3]);

	// ... but InitCellData is here
	{
		int maxCellData = m_numStraddlers + m_cell_slots_add[4];
		DevMsg(SPEW_NORM, "*** MAX cell slots = %d\n", maxCellData);

		// I don't have any idea
		pFile->Read(&m_numSpoolInfoOffsets, 1, sizeof(int));
		DevMsg(SPEW_NORM, "numRegionOffsets: %d\n", m_numSpoolInfoOffsets);
	}

	m_regionSpoolInfoOffsets = new ushort[m_numSpoolInfoOffsets];
	pFile->Read(m_regionSpoolInfoOffsets, m_numSpoolInfoOffsets, sizeof(short));

	int regionsInfoSize;
	pFile->Read(&regionsInfoSize, 1, sizeof(int));
	m_numRegionSpools = regionsInfoSize / sizeof(AreaDataStr);

	//ASSERT(regionsInfoSize % sizeof(REGIONINFO) == 0);

	DevMsg(SPEW_NORM, "Region spool count %d (size=%d bytes)\n", m_numRegionSpools, regionsInfoSize);

	m_regionSpoolInfo = (Spool*)malloc(regionsInfoSize);
	pFile->Read(m_regionSpoolInfo, 1, regionsInfoSize);

	m_areaDataStates = new bool[m_numAreas];
	memset(m_areaDataStates, 0, m_numAreas);
}

void CBaseLevelMap::LoadInAreaTPages(const SPOOL_CONTEXT& ctx, int areaDataNum) const
{
	if (areaDataNum == 255)
		return;

	AreaDataStr& areaData = m_areaData[areaDataNum];
	AreaTpageList& areaTPages = m_areaTPages[areaDataNum];

	const int texturesOffset = ctx.lumpInfo->spooled_offset + SPOOL_CD_BLOCK_SIZE * areaData.gfx_offset;

	ctx.dataStream->Seek(texturesOffset, VS_SEEK_SET);

	for (int i = 0; areaData.num_tpages; i++)
	{
		if (areaTPages.pageIndexes[i] == 0xFF)
			break;

		CTexturePage* tpage = ctx.textures->GetTPage(areaTPages.pageIndexes[i]);

		// assign
		areaTPages.tpage[i] = tpage;

		tpage->LoadTPageAndCluts(ctx.dataStream, true);

		if (ctx.dataStream->Tell() % SPOOL_CD_BLOCK_SIZE)
			ctx.dataStream->Seek(SPOOL_CD_BLOCK_SIZE - (ctx.dataStream->Tell() % SPOOL_CD_BLOCK_SIZE), VS_SEEK_CUR);
	}
}

void CBaseLevelMap::LoadInAreaModels(const SPOOL_CONTEXT& ctx, int areaDataNum) const
{
	if (areaDataNum == -1)
		return;

	AreaDataStr& areaData = m_areaData[areaDataNum];

	int length = areaData.model_size;

	const int modelsCountOffset = ctx.lumpInfo->spooled_offset + (areaData.model_offset + length - 1) * SPOOL_CD_BLOCK_SIZE;
	const int modelsOffset = ctx.lumpInfo->spooled_offset + areaData.model_offset * SPOOL_CD_BLOCK_SIZE;

	ushort numModels;

	ctx.dataStream->Seek(modelsCountOffset, VS_SEEK_SET);
	ctx.dataStream->Read(&numModels, 1, sizeof(ushort));

	// read model indexes
	ushort* new_model_numbers = new ushort[numModels];
	ctx.dataStream->Read(new_model_numbers, numModels, sizeof(short));

	DevMsg(SPEW_INFO, "	model count: %d\n", numModels);
	ctx.dataStream->Seek(modelsOffset, VS_SEEK_SET);

	for (int i = 0; i < numModels; i++)
	{
		int modelSize;
		ctx.dataStream->Read(&modelSize, sizeof(int), 1);

		if (modelSize > 0)
		{
			ModelRef_t* ref = ctx.models->GetModelByIndex(new_model_numbers[i]);

			// @FIXME: is that correct? Analyze duplicated models...
			if (ref->model)
			{
				// maybe there is a simple case of area data model duplication?
				if (ref->size != modelSize)
					MsgError("Spool model in slot %d OVERLAP!\n", new_model_numbers[i]);

				ctx.dataStream->Seek(modelSize, VS_SEEK_CUR);
				continue;
			}

			ref->model = (MODEL*)malloc(modelSize);
			ref->size = modelSize;

			ctx.dataStream->Read(ref->model, modelSize, 1);

			ctx.models->OnModelLoaded(ref);
		}
	}

	delete[] new_model_numbers;
}

void CBaseLevelMap::InitRegion(CBaseLevelRegion* region, int index) const
{
	ushort spoolOffset = m_regionSpoolInfoOffsets[index];

	if (spoolOffset != REGION_EMPTY)
		region->m_spoolInfo = (Spool*)((ubyte*)m_regionSpoolInfo + spoolOffset);

	const int region_x = index % m_regions_across;
	const int region_z = (index - region_x) / m_regions_across;

	region->m_owner = (CBaseLevelMap*)this;
	region->m_regionX = region_x;
	region->m_regionZ = region_z;
	region->m_regionNumber = index;
	region->m_regionBarrelNumber = (region_x & 1) + (region_z & 1) * 2;
}

void CBaseLevelMap::SetLoadingCallbacks(OnRegionLoaded_t onLoaded, OnRegionFreed_t onFreed)
{
	m_onRegionLoaded = onLoaded;
	m_onRegionFreed = onFreed;
}

void CBaseLevelMap::SetFormat(ELevelFormat format)
{
	m_format = format;
}

ELevelFormat CBaseLevelMap::GetFormat() const
{
	return m_format;
}

void CBaseLevelMap::OnRegionLoaded(CBaseLevelRegion* region)
{
	if (m_onRegionLoaded)
		m_onRegionLoaded(region);
}

void CBaseLevelMap::OnRegionFreed(CBaseLevelRegion* region)
{
	if (m_onRegionFreed)
		m_onRegionFreed(region);
}
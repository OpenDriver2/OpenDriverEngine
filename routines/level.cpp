#include "core/dktypes.h"
#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include <nstd/String.hpp>
#include <nstd/File.hpp>

#include "regions_d1.h"
#include "regions_d2.h"
#include "textures.h"

#include "level.h"


//--------------------------------------------------------------------------------------------------------------------------

char*						g_overlayMapData = nullptr;

//-------------------------------------------------------------
// Loads overhead map lump
//-------------------------------------------------------------
void LoadOverlayMapLump(IVirtualStream* pFile, int lumpSize)
{
	g_overlayMapData = new char[lumpSize];
	pFile->Read(g_overlayMapData, 1, lumpSize);
}

//-------------------------------------------------------------
// Auto-detects level format
//-------------------------------------------------------------
ELevelFormat CDriverLevelLoader::DetectLevelFormat(IVirtualStream* pFile)
{
	long curPos = pFile->Tell();
	int lump_count = 255;

	LUMP lump;
	OUT_CITYLUMP_INFO cityLumps;

	for (int i = 0; i < lump_count; i++)
	{
		// read lump info
		pFile->Read(&lump, sizeof(LUMP), 1);

		// stop marker
		if (lump.type == 255)
			break;

		switch (lump.type)
		{
			case LUMP_MODELS:
			case LUMP_MAP:
			case LUMP_TEXTURENAMES:
			case LUMP_MODELNAMES:
			case LUMP_LOWDETAILTABLE:
			case LUMP_MOTIONCAPTURE:
			case LUMP_OVERLAYMAP:
			case LUMP_PALLET:
			case LUMP_SPOOLINFO:
			case LUMP_CHAIR:
			case LUMP_CAR_MODELS:
			case LUMP_TEXTUREINFO:
			case LUMP_STRAIGHTS2:
			case LUMP_CURVES2:
				break;
			case LUMP_JUNCTIONS2:
			{
				MsgInfo("Detected 'Driver 2 DEMO' 1.6 alpha LEV file\n");
				pFile->Seek(curPos, VS_SEEK_SET);
				return LEV_FORMAT_DRIVER2_ALPHA16; // as it is an old junction format - it's clearly a alpha 1.6 level
				break;
			}
			case LUMP_JUNCTIONS2_NEW:
			{
				MsgInfo("Detected 'Driver 2' final LEV file\n");
				pFile->Seek(curPos, VS_SEEK_SET);
				return LEV_FORMAT_DRIVER2_RETAIL; // most recent LEV file
				break;
			}
			case LUMP_LOADTIME_DATA:
			case LUMP_INMEMORY_DATA:
				break;
			case LUMP_LUMPDESC:
			{
				int loadtime_data_ofs;
				pFile->Read(&cityLumps, 1, sizeof(cityLumps));
				pFile->Seek(cityLumps.inmem_offset, VS_SEEK_SET);
				break;
			}
			case LUMP_ROADMAP:
			case LUMP_ROADS:
			case LUMP_JUNCTIONS:
			case LUMP_ROADSURF:
			case LUMP_ROADBOUNDS:
			case LUMP_JUNCBOUNDS:
			case LUMP_SUBDIVISION:
			default: // maybe Lump 11?
			{
				MsgInfo("Detected 'Driver 1' LEV file\n");
				pFile->Seek(curPos, VS_SEEK_SET);
				return LEV_FORMAT_DRIVER1;
				break;
			}
		}

		if (lump.type == LUMP_LOADTIME_DATA || 
			lump.type == LUMP_INMEMORY_DATA ||
			lump.type == LUMP_LUMPDESC)
		{
			// restart
			i = -1;
			lump_count = 255;
			continue;
		}

		// skip lump
		pFile->Seek(lump.size, VS_SEEK_CUR);

		// position alignment
		if ((pFile->Tell() % 4) != 0)
			pFile->Seek(4 - (pFile->Tell() % 4), VS_SEEK_CUR);
	}

	pFile->Seek(curPos, VS_SEEK_SET);

	return LEV_FORMAT_INVALID;
}

//-------------------------------------------------------------
// Iterates LEV file lumps and loading data from them
//-------------------------------------------------------------
void CDriverLevelLoader::ProcessLumps(IVirtualStream* pFile)
{
	int lump_count = 255; // Driver 2 difference: you not need to read lump count

	// Driver 1 has lump count
	if (m_format == LEV_FORMAT_DRIVER1)
		pFile->Read(&lump_count, sizeof(int), 1);

	LUMP lump;
	for (int i = 0; i < lump_count; i++)
	{
		// read lump info
		pFile->Read(&lump, sizeof(LUMP), 1);

		// stop marker
		if (lump.type == 255)
			break;

		int l_ofs = pFile->Tell();

		switch (lump.type)
		{
			// Lumps shared between formats
			// almost identical
			case LUMP_MODELS:
				DevMsg(SPEW_WARNING, "LUMP_MODELS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_models)
					m_models->LoadLevelModelsLump(pFile);
				break;
			case LUMP_MAP:
				DevMsg(SPEW_WARNING, "LUMP_MAP ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_map)
					m_map->LoadMapLump(pFile);
				break;
			case LUMP_TEXTURENAMES:
				DevMsg(SPEW_WARNING, "LUMP_TEXTURENAMES ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_textures)
					m_textures->LoadTextureNamesLump(pFile, lump.size);
				break;
			case LUMP_MODELNAMES:
				DevMsg(SPEW_WARNING, "LUMP_MODELNAMES ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_models)
					m_models->LoadModelNamesLump(pFile, lump.size);
				break;
			case LUMP_LOWDETAILTABLE:
				if(m_models)
					m_models->LoadLowDetailTableLump(pFile, lump.size);
				DevMsg(SPEW_WARNING, "LUMP_LOWDETAILTABLE ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_MOTIONCAPTURE:
				DevMsg(SPEW_WARNING, "LUMP_MOTIONCAPTURE ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_OVERLAYMAP:
				DevMsg(SPEW_WARNING, "LUMP_OVERLAYMAP ofs=%d size=%d\n", pFile->Tell(), lump.size);
				LoadOverlayMapLump(pFile, lump.size);
				break;
			case LUMP_PALLET:
				DevMsg(SPEW_WARNING, "LUMP_PALLET ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_textures)
					m_textures->ProcessPalletLump(pFile);
				break;
			case LUMP_SPOOLINFO:
				DevMsg(SPEW_WARNING, "LUMP_SPOOLINFO ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_map)
					m_map->LoadSpoolInfoLump(pFile);
				break;
			case LUMP_CHAIR:
				DevMsg(SPEW_WARNING, "LUMP_CHAIR ofs=%d size=%d\n", pFile->Tell(), lump.size);
				// TODO: get chairs
				break;
			case LUMP_CAR_MODELS:
				DevMsg(SPEW_WARNING, "LUMP_CAR_MODELS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_models)
					m_models->LoadCarModelsLump(pFile, lump.size);
				break;
			case LUMP_TEXTUREINFO:
				DevMsg(SPEW_WARNING, "LUMP_TEXTUREINFO ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if(m_textures)
					m_textures->LoadTextureInfoLump(pFile);
				break;
			// Driver 2 - only lumps
			case LUMP_STRAIGHTS2:
				DevMsg(SPEW_WARNING, "LUMP_STRAIGHTS2 ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if (m_map)
					((CDriver2LevelMap*)m_map)->LoadStraightsLump(pFile);
				break;
			case LUMP_CURVES2:
				DevMsg(SPEW_WARNING, "LUMP_CURVES2 ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if (m_map)
					((CDriver2LevelMap*)m_map)->LoadCurvesLump(pFile);
				break;
			case LUMP_JUNCTIONS2:
				DevMsg(SPEW_WARNING, "LUMP_JUNCTIONS2 ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if (m_map)
					((CDriver2LevelMap*)m_map)->LoadJunctionsLump(pFile, true);
				break;
			case LUMP_JUNCTIONS2_NEW:
				DevMsg(SPEW_WARNING, "LUMP_JUNCTIONS2_NEW ofs=%d size=%d\n", pFile->Tell(), lump.size);
				if (m_map)
					((CDriver2LevelMap*)m_map)->LoadJunctionsLump(pFile, false);
				break;
			// Driver 1 - only lumps
			case LUMP_ROADMAP:
				DevMsg(SPEW_WARNING, "LUMP_ROADMAP ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_ROADS:
				DevMsg(SPEW_WARNING, "LUMP_ROADS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_JUNCTIONS:
				DevMsg(SPEW_WARNING, "LUMP_JUNCTIONS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_ROADSURF:
				DevMsg(SPEW_WARNING, "LUMP_ROADSURF ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_ROADBOUNDS:
				DevMsg(SPEW_WARNING, "LUMP_ROADBOUNDS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_JUNCBOUNDS:
				DevMsg(SPEW_WARNING, "LUMP_JUNCBOUNDS ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			case LUMP_SUBDIVISION:
				DevMsg(SPEW_WARNING, "LUMP_SUBDIVISION ofs=%d size=%d\n", pFile->Tell(), lump.size);
				break;
			default:
				DevMsg(SPEW_WARNING, "LUMP type: %d (0x%X) ofs=%d size=%d\n", lump.type, lump.type, pFile->Tell(), lump.size);
		}

		// seek back to initial position
		pFile->Seek(l_ofs, VS_SEEK_SET);

		// skip lump
		pFile->Seek(lump.size, VS_SEEK_CUR);

		// position alignment
		if ((pFile->Tell() % 4) != 0)
			pFile->Seek(4 - (pFile->Tell() % 4), VS_SEEK_CUR);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------

CDriverLevelLoader::CDriverLevelLoader()
{
}

CDriverLevelLoader::~CDriverLevelLoader()
{
	Release();
}

ELevelFormat CDriverLevelLoader::GetFormat() const
{
	return m_format;
}

void CDriverLevelLoader::Initialize(OUT_CITYLUMP_INFO& lumpInfo, CDriverLevelTextures* textures, CDriverLevelModels* models, CBaseLevelMap* map)
{
	m_lumpInfo = &lumpInfo;
	m_textures = textures;
	m_models = models;
	m_map = map;
}

void CDriverLevelLoader::Release()
{
	delete[] g_overlayMapData;
}

//-------------------------------------------------------------
// Loads the LEV file data
//-------------------------------------------------------------
bool CDriverLevelLoader::LoadFromFile(const char* fileName)
{
	// try load driver2 lev file
	FILE* pReadFile = fopen(fileName, "rb");

	if (!pReadFile)
	{
		MsgError("Failed to open LEV file!\n");
		return false;
	}

	CFileStream stream(pReadFile);

	// seek to begin
	MsgWarning("-----------\nLoading LEV file '%s'\n", fileName);

	//-------------------------------------------------------------------

	// perform auto-detection if format is not specified
	if (m_format == LEV_FORMAT_AUTODETECT)
		m_format = DetectLevelFormat(&stream);

	if (m_map)
		m_map->SetFormat(m_format);

	if (m_textures)
		m_textures->SetFormat(m_format);

	LUMP curLump;
	stream.Read(&curLump, sizeof(curLump), 1);

	if (curLump.type != LUMP_LUMPDESC)
	{
		MsgError("Not a LEV file!\n");
		fclose(pReadFile);
		return false;
	}

	// read chunk offsets
	stream.Read(m_lumpInfo, sizeof(OUT_CITYLUMP_INFO), 1);

	DevMsg(SPEW_NORM, "data1_offset = %d\n", m_lumpInfo->loadtime_offset);
	DevMsg(SPEW_NORM, "data1_size = %d\n", m_lumpInfo->loadtime_size);

	DevMsg(SPEW_NORM, "tpage_offset = %d\n", m_lumpInfo->tpage_offset);
	DevMsg(SPEW_NORM, "tpage_size = %d\n", m_lumpInfo->tpage_size);

	DevMsg(SPEW_NORM, "data2_offset = %d\n", m_lumpInfo->inmem_offset);
	DevMsg(SPEW_NORM, "data2_size = %d\n", m_lumpInfo->inmem_size);

	DevMsg(SPEW_NORM, "spooled_offset = %d\n", m_lumpInfo->spooled_offset);
	DevMsg(SPEW_NORM, "spooled_size = %d\n", m_lumpInfo->spooled_size);

	// read cells

	//-----------------------------------------------------
	// seek to section 1 - lump data 1
	stream.Seek(m_lumpInfo->loadtime_offset, VS_SEEK_SET);

	// read lump
	stream.Read(&curLump, sizeof(curLump), 1);

	if (curLump.type != LUMP_LOADTIME_DATA)
	{
		MsgError("Not a LUMP_LOADTIME_DATA!\n");
		fclose(pReadFile);
		return false;
	}

	DevMsg(SPEW_INFO, "entering LUMP_LOADTIME_DATA size = %d\n--------------\n", curLump.size);

	// read sublumps
	ProcessLumps(&stream);

	//-----------------------------------------------------
	// read global textures

	if (m_textures)
	{
		stream.Seek(m_lumpInfo->tpage_offset, VS_SEEK_SET);
		m_textures->LoadPermanentTPages(&stream);
	}

	//-----------------------------------------------------
	// seek to section 3 - lump data 2
	stream.Seek(m_lumpInfo->inmem_offset, VS_SEEK_SET);

	// read lump
	stream.Read(&curLump, sizeof(curLump), 1);

	if (curLump.type != LUMP_INMEMORY_DATA)
	{
		MsgError("Not a lump LUMP_INMEMORY_DATA!\n");
		fclose(pReadFile);
		return false;
	}

	DevMsg(SPEW_INFO, "entering LUMP_INMEMORY_DATA size = %d\n--------------\n", curLump.size);

	// read sublumps
	ProcessLumps(&stream);

	// completed!
	fclose(pReadFile);

	return true;
}
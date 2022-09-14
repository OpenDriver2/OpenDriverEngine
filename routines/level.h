//////////////////////////////////////////////////////////////////////////////////
// Description: Driver 2 LEVel main structures
//////////////////////////////////////////////////////////////////////////////////

#ifndef LEVEL_H
#define LEVEL_H

#include "d2_types.h"

#define SPOOL_CD_BLOCK_SIZE		2048

// known lumps indexes
enum LevLumpType
{
	// known lumps indexes
	LUMP_PERMANENTPAGES		= 0,		// Driver 1 old format of texture pages
	LUMP_MODELS				= 1,		// level models
	LUMP_MAP				= 2,		// map info

	LUMP_TEXTURENAMES		= 5,		// texture name strings

	LUMP_ROADMAP			= 7,		// Driver 1 road map indexes?
	LUMP_ROADS				= 8,		// Driver 1 straight roads
	LUMP_JUNCTIONS			= 9,		// Driver 1 junctions
	LUMP_ROADSURF			= 10,		// Driver 1 road map height information

	LUMP_MODELNAMES			= 12,		// model name strings

	LUMP_ROADBOUNDS			= 16,		// Driver 1 road bounds
	LUMP_JUNCBOUNDS			= 17,		// Driver 1 junction bounds
	LUMP_SUBDIVISION		= 20,		// Driver 1 subdivisions (of roads?)

	LUMP_LOWDETAILTABLE		= 21,		// LOD tables for models
	LUMP_MOTIONCAPTURE		= 22,		// motion capture data for pedestrians and Tanner in Driver 2
	LUMP_OVERLAYMAP			= 24,		// overhead map palette and chunks packed with RNC2
	LUMP_PALLET				= 25,		// car palettes
	LUMP_SPOOLINFO			= 26,		// region spooling information
	LUMP_CAR_MODELS			= 28,		// car models

	LUMP_CHAIR				= 33,		// seatable objects
	LUMP_TEXTUREINFO		= 34,		// texture page info and details (atlases)

	LUMP_LOADTIME_DATA		= 35,
	LUMP_INMEMORY_DATA		= 36,
	LUMP_LUMPDESC			= 37,

	LUMP_STRAIGHTS2			= 40,		// Driver 2 straights
	LUMP_CURVES2			= 41,		// Driver 2 curves
	LUMP_JUNCTIONS2			= 42,		// Driver 2 junctions (alpha 1.6)
	LUMP_JUNCTIONS2_NEW		= 43,		// Driver 2 junctions (retail)
};

enum ELevelFormat
{
	LEV_FORMAT_AUTODETECT = -1,

	LEV_FORMAT_INVALID = 0,

	LEV_FORMAT_DRIVER1_OLD,			// driver 1, old lump version
	LEV_FORMAT_DRIVER1,				// driver 1
	LEV_FORMAT_DRIVER2_ALPHA16,		// driver 2 alpha 1.6 format
	LEV_FORMAT_DRIVER2_RETAIL,		// driver 2 retail format
};

// forward
class IVirtualStream;
class CDriverLevelTextures;
class CDriverLevelModels;
class CBaseLevelMap;

//------------------------------------------------------------------------------------------------------------

class CDriverLevelLoader
{
public:
	static ELevelFormat		DetectLevelFormat(IVirtualStream* pFile);

	//------------------------------------------------------

	CDriverLevelLoader();
	virtual ~CDriverLevelLoader();

	void					Initialize(OUT_CITYLUMP_INFO& lumpInfo, CDriverLevelTextures* textures, CDriverLevelModels* models, CBaseLevelMap* map);
	void					Release();

	ELevelFormat			GetFormat() const;

	bool					Load(IVirtualStream* pStream);

protected:
	void					ProcessLumps(IVirtualStream* pFile);

	ELevelFormat			m_format{ LEV_FORMAT_AUTODETECT };
	String					m_fileName;

	OUT_CITYLUMP_INFO*		m_lumpInfo;

	CBaseLevelMap*			m_map{ nullptr };
	CDriverLevelTextures*	m_textures{ nullptr };
	CDriverLevelModels*		m_models{ nullptr };
};

#endif // LEVEL_H

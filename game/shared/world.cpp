#include <malloc.h>

#include <nstd/String.hpp>

#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include "renderer/debug_overlay.h"
#include "renderer/gl_renderer.h"

#include "routines/d2_types.h"
#include "routines/level.h"
#include "routines/models.h"
#include "routines/regions_d1.h"
#include "routines/regions_d2.h"
#include "routines/textures.h"

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include "world.h"
#include "camera.h"

#include "game/render/render_level.h"
#include "game/render/render_model.h"
#include "game/render/render_sky.h"

#include "math/convert.h"
#include "math/Volume.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"

OUT_CITYLUMP_INFO		g_levInfo;
CDriverLevelTextures	g_levTextures;
CDriverLevelModels		g_levModels;
CBaseLevelMap*			g_levMap = nullptr;

void CWorld::Lua_Init(sol::state& lua)
{
	auto engine = lua["engine"].get_or_create<sol::table>();

	auto world = engine["World"].get_or_create<sol::table>();

	world["GetHWTexture"] = &GetHWTexture;
	world["LoadLevelFile"] = &LoadLevelFile;
	world["FreeLevelData"] = &FreeLevelData;
	world["SpoolAllAreaDatas"] = &SpoolAllAreaDatas;
}

//-----------------------------------------------------------------

TextureID g_hwTexturePages[128][16];
extern TextureID g_whiteTexture;

// Creates hardware texture
void CWorld::InitHWTexturePage(CTexturePage* tpage)
{
	const TexBitmap_t& bitmap = tpage->GetBitmap();

	if (bitmap.data == nullptr)
		return;

	int imgSize = TEXPAGE_SIZE * 4;
	uint* color_data = (uint*)malloc(imgSize);

	memset(color_data, 0, imgSize);

	// Dump whole TPAGE indexes
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			ubyte clindex = bitmap.data[y * 128 + (x >> 1)];

			if (0 != (x & 1))
				clindex >>= 4;

			clindex &= 0xF;

			int ypos = (TEXPAGE_SIZE_Y - y - 1) * TEXPAGE_SIZE_Y;

			color_data[ypos + x] = clindex * 32;
		}
	}

	int numDetails = tpage->GetDetailCount();

	// FIXME: load indexes instead?

	for (int i = 0; i < numDetails; i++)
		tpage->ConvertIndexedTextureToRGBA(color_data, i, &bitmap.clut[i], false, false);

	int tpageId = tpage->GetId();
	
	g_hwTexturePages[tpageId][0] = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);

	// also load different palettes
	int numPalettes = 0;
	for (int pal = 0; pal < 16; pal++)
	{
		bool anyMatched = false;

		for (int j = 0; j < numDetails; j++)
		{
			TexDetailInfo_t* detail = tpage->GetTextureDetail(j);

			if (detail->extraCLUTs[pal])
			{
				tpage->ConvertIndexedTextureToRGBA(color_data, j, detail->extraCLUTs[pal], false, false);
				anyMatched = true;
			}
		}

		if (anyMatched)
		{
			g_hwTexturePages[tpageId][numPalettes + 1] = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
			numPalettes++;
		}
	}
	
	
	// no longer need in RGBA data
	free(color_data);
}

void CWorld::FreeHWTexturePage(CTexturePage* tpage)
{
	int tpageId = tpage->GetId();
	GR_DestroyTexture(g_hwTexturePages[tpageId][0]);

	for (int pal = 0; pal < 16; pal++)
	{
		if(g_hwTexturePages[tpageId][pal + 1] != g_whiteTexture)
			GR_DestroyTexture(g_hwTexturePages[tpageId][pal + 1]);
	}
}

// returns hardware texture
TextureID CWorld::GetHWTexture(int tpage, int pal)
{
	if (tpage < 0 || tpage >= 128 ||
		pal < 0 || pal >= 16)
		return g_whiteTexture;

	return g_hwTexturePages[tpage][pal];
}

// Dummy texture initilization
void CWorld::InitHWTextures()
{
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 16; j++)
			g_hwTexturePages[i][j] = g_whiteTexture;
	}

	// set loading callbacks
	g_levTextures.SetLoadingCallbacks(InitHWTexturePage, FreeHWTexturePage);

}

void CWorld::InitHWModels()
{
	CRenderModel::InitModelShader();
	g_levModels.SetModelLoadingCallbacks(CRenderModel::OnModelLoaded, CRenderModel::OnModelFreed);
}

//-----------------------------------------------------------------

// extern some vars
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

FILE* g_levFile = nullptr;

//-------------------------------------------------------
// Perorms level loading and renderer data initialization
//-------------------------------------------------------
bool CWorld::LoadLevelFile(const char* fileName)
{
	g_levFile = fopen(fileName, "rb");
	if (!g_levFile)
	{
		MsgError("Cannot open '%s'\n", fileName);
		return false;
	}

	// seek to begin
	MsgWarning("-----------\nLoading LEV file '%s'\n", fileName);

	CFileStream stream(g_levFile);
	ELevelFormat levFormat = CDriverLevelLoader::DetectLevelFormat(&stream);

	stream.Seek(0, VS_SEEK_SET);

	// create map accordingly
	if (levFormat >= LEV_FORMAT_DRIVER2_ALPHA16 || levFormat == LEV_FORMAT_AUTODETECT)
		g_levMap = new CDriver2LevelMap();
	else
		g_levMap = new CDriver1LevelMap();

	CDriverLevelLoader loader;
	loader.Initialize(g_levInfo, &g_levTextures, &g_levModels, g_levMap);

	return loader.Load(&stream);
}

//-------------------------------------------------------
// Frees all data
//-------------------------------------------------------
void CWorld::FreeLevelData()
{
	MsgWarning("Freeing level data ...\n");

	g_levMap->FreeAll();
	g_levTextures.FreeAll();
	g_levModels.FreeAll();

	delete g_levMap;

	fclose(g_levFile);
}

//-------------------------------------------------------
// Render level viewer
//-------------------------------------------------------
void CWorld::RenderLevelView()
{
	Volume frustumVolume;

	// setup standard camera
	CRenderModel::SetupModelShader();
	SetupCameraViewAndMatrices(g_cameraPosition, g_cameraAngles, frustumVolume);

	GR_SetDepth(1);
	GR_SetCullMode(CULL_FRONT);

	// reset lighting
	CRenderModel::SetupLightingProperties();
	
	if(g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16)
		DrawLevelDriver2(g_cameraPosition, g_cameraAngles.y, frustumVolume);
	else
		DrawLevelDriver1(g_cameraPosition, g_cameraAngles.y, frustumVolume);
}

//-------------------------------------------------------------
// Forcefully spools entire level regions and area datas
//-------------------------------------------------------------
void CWorld::SpoolAllAreaDatas()
{
	Msg("Spooling ALL regions...\n");

	CFileStream stream(g_levFile);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &stream;
	spoolContext.lumpInfo = &g_levInfo;
	spoolContext.models = &g_levModels;
	spoolContext.textures = &g_levTextures;

	int totalRegions = g_levMap->GetRegionsAcross() * g_levMap->GetRegionsDown();
		
	for (int i = 0; i < totalRegions; i++)
	{
		g_levMap->SpoolRegion(spoolContext, i);
	}
}


//-------------------------------------------------------------


#include "game/pch.h"
#include "world.h"

#include "routines/regions_d1.h"
#include "routines/regions_d2.h"

OUT_CITYLUMP_INFO		g_levInfo;
CDriverLevelTextures	g_levTextures;
CDriverLevelModels		g_levModels;
CBaseLevelMap*			g_levMap = nullptr;

Array<CELL_OBJECT>		CWorld::m_CellObjects;
int						CWorld::StepCount = 0;

Matrix4x4	g_objectMatrix[64];
MATRIX		g_objectMatrixFixed[64];

void CWorld::Lua_Init(sol::state& lua)
{
	auto engine = lua["engine"].get_or_create<sol::table>();

	LUADOC_GLOBAL();
	{
		LUADOC_TYPE("World");

		auto world = engine["World"].get_or_create<sol::table>();

		world[LUADOC_M("GetHWTexture")]
			= &GetHWTexture;

		world[LUADOC_M("LoadLevel", "loads level from file")] 
			= &LoadLevel;

		world[LUADOC_M("UnloadLevel")] 
			= &UnloadLevel;

		world[LUADOC_M("SpoolAllRegions", "load all models, regions and textures")] 
			= &SpoolAllRegions;

		world[LUADOC_M("SpoolRegions", "spool regions at specified point and radius")]
			= &SpoolRegions;

		world[LUADOC_M("IsLevelLoaded")] 
			= &IsLevelLoaded;

		world[LUADOC_M("GetModelByIndex", "returns model reference by specified index")] 
			= &GetModelByIndex;

		world[LUADOC_M("GetModelByName", "returns model reference by specified name")]
			= &GetModelByName;

		world[LUADOC_M("MapHeight", "returns height value at specified 3D point")]
			= &MapHeight;

		world[LUADOC_M("PushCellObject", "push event cell object. Any collision checks afterwards will have an effect with it")]
			= &PushCellObject;

		world[LUADOC_M("PurgeCellObjects", "purges list of recently added objects by PushCellObject")]
			= &PurgeCellObjects;

		world[LUADOC_M("EndStep", "finalizes the game step, incrementing step count by 1.")]
			= &EndStep;

		world[LUADOC_M("ResetStep", "resets world step count")]
			= &ResetStep;

		world[LUADOC_M("StepCount", "world step count")]
			= &StepCount;
	}

	// level properties
	{
		LUADOC_TYPE();
		lua.new_usertype<LevelRenderProps>(
			LUADOC_T("LevelRenderProps"),

			LUADOC_P("ambientColor"), &LevelRenderProps::ambientColor,
			LUADOC_P("lightColor"), &LevelRenderProps::lightColor,
			LUADOC_P("nightAmbientScale"), &LevelRenderProps::nightAmbientScale,
			LUADOC_P("nightLightScale"), &LevelRenderProps::nightLightScale,
			LUADOC_P("ambientScale"), &LevelRenderProps::ambientScale,
			LUADOC_P("lightScale"), &LevelRenderProps::lightScale,
			LUADOC_P("nightMode"), &LevelRenderProps::nightMode,
			LUADOC_P("noLod"), &LevelRenderProps::noLod,

			LUADOC_P("displayCollisionBoxes"), &LevelRenderProps::displayCollisionBoxes,
			LUADOC_P("displayHeightMap"), &LevelRenderProps::displayHeightMap,
			LUADOC_P("displayAllCellLevels"), &LevelRenderProps::displayAllCellLevels
		);
	}

	{
		MAKE_PROPERTY_REF(lua, ModelRef_t*);
		LUADOC_TYPE();
		lua.new_usertype<ModelRef_t>(
			LUADOC_T("ModelRef"),

			LUADOC_P("name"), &ModelRef_t::name,
			LUADOC_P("index"), &ModelRef_t::index,
			LUADOC_P("highDetailId"), &ModelRef_t::highDetailId,
			LUADOC_P("lowDetailId"), &ModelRef_t::lowDetailId,
			LUADOC_P("enabled"), &ModelRef_t::enabled
		);
	}

	// level properties
	{
		MAKE_PROPERTY_REF(lua, CELL_OBJECT);
		MAKE_PROPERTY_REF(lua, CELL_OBJECT*);
		LUADOC_TYPE();
		lua.new_usertype<CELL_OBJECT>(
			LUADOC_T("CELL_OBJECT"),

			sol::call_constructor, sol::factories(
				[](const VECTOR_NOPAD& position, const ubyte& yang, const ushort& type) {
					return CELL_OBJECT{ position, 0, yang, type };
				},
				[](const sol::table& table) {
					return CELL_OBJECT{ (VECTOR_NOPAD&)table["position"], 0, table["yang"], table["type"] };
				},
				[]() { return CELL_OBJECT{ 0 }; }),
			LUADOC_P("pos"), &CELL_OBJECT::pos,
			LUADOC_P("yang"), &CELL_OBJECT::yang,
			LUADOC_P("type", "model index"), &CELL_OBJECT::type
		);
	}

	engine["LevelRenderProps"] = &CRender_Level::RenderProps;
}

//-----------------------------------------------------------------

void CWorld::InitObjectMatrix()
{
	for (int i = 0; i < 64; i++)
	{
		MATRIX& m = g_objectMatrixFixed[i];

		InitMatrix(m);
		RotMatrixY(i * 64, &m);

		const float cellRotationRad = -i / 64.0f * PI_F * 2.0f;
		g_objectMatrix[i] = rotateY4(cellRotationRad);
	}
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
	uint* color_data = (uint*)Memory::alloc(imgSize);

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
			g_hwTexturePages[tpageId][++numPalettes] = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
		}
	}
	
	
	// no longer need in RGBA data
	Memory::free(color_data);
}

void CWorld::FreeHWTexturePage(CTexturePage* tpage)
{
	int tpageId = tpage->GetId();

	for (int pal = 0; pal < 16; pal++)
	{
		if(g_hwTexturePages[tpageId][pal] != g_whiteTexture)
			GR_DestroyTexture(g_hwTexturePages[tpageId][pal]);

		g_hwTexturePages[tpageId][pal] = g_whiteTexture;
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

CTexturePage* CWorld::GetTPage(int tpage)
{
	
	return g_levTextures.GetTPage(tpage);
}

TexDetailInfo_t* CWorld::FindTextureDetail(const char* name)
{
	return g_levTextures.FindTextureDetail(name);
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
bool CWorld::LoadLevel(const char* fileName)
{
	FILE* fp = fopen(fileName, "rb");
	if (!fp)
	{
		MsgError("Cannot open '%s'\n", fileName);
		return false;
	}

	UnloadLevel();

	g_levFile = fp;

	// seek to begin
	MsgWarning("-----------\nLoading LEV file '%s'\n", fileName);

	CFileStream stream(g_levFile, false);
	ELevelFormat levFormat = CDriverLevelLoader::DetectLevelFormat(&stream);

	stream.Seek(0, VS_SEEK_SET);

	// create map accordingly
	if (levFormat >= LEV_FORMAT_DRIVER2_ALPHA16 || levFormat == LEV_FORMAT_AUTODETECT)
		g_levMap = new CDriver2LevelMap();
	else
		g_levMap = new CDriver1LevelMap();

	CDriverLevelLoader loader;
	loader.Initialize(g_levInfo, &g_levTextures, &g_levModels, g_levMap);

	bool result = loader.Load(&stream);

	CRender_Cars::InitRender();
	CRender_Level::InitRender();
	ResetStep();

	return result;
}

//-------------------------------------------------------
// Frees all data
//-------------------------------------------------------
void CWorld::UnloadLevel()
{
	CRender_Cars::TerminateRender();
	CRender_Level::TerminateRender();
	CManager_Players::RemoveAllPlayers();
	if (g_levMap)
	{
		MsgWarning("Freeing level data ...\n");
		g_levMap->FreeAll();

		g_levTextures.FreeAll();
		g_levModels.FreeAll();

		delete g_levMap;
		g_levMap = nullptr;

		fclose(g_levFile);
		g_levFile = nullptr;
	}
}

//-------------------------------------------------------
// Render level viewer
//-------------------------------------------------------
void CWorld::RenderLevelView(const CameraViewParams& view)
{
	Volume frustumVolume;

	// setup standard camera
	CRenderModel::SetupModelShader();
	CCamera::SetupViewAndMatrices(view, frustumVolume);

	GR_SetDepthMode(1, 1);
	GR_SetCullMode(CULL_FRONT);

	// reset lighting
	CRenderModel::SetupLightingProperties();

	bool driver2Map = g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;
	
	CRender_Level::DrawMap(view.position, view.angles.y, frustumVolume);

	for (usize i = 0; i < m_CellObjects.size(); i++)
	{
		CRender_Level::DrawCellObject(m_CellObjects[i], view.position, view.angles.y, frustumVolume, driver2Map);
	}
}

void CWorld::SpoolRegions(const VECTOR_NOPAD& position, int radius)
{
	if (!IsLevelLoaded())
		return;

	CFileStream stream(g_levFile, false);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &stream;
	spoolContext.lumpInfo = &g_levInfo;

	int regionsAcross = g_levMap->GetRegionsAcross();
	int regionsDown = g_levMap->GetRegionsDown();
	int regionSize = g_levMap->GetMapInfo().region_size;
	int cellSize = g_levMap->GetMapInfo().cell_size;

	// get center region
	XZPAIR cell;
	g_levMap->WorldPositionToCellXZ(cell, position);

	int midRegion = g_levMap->GetRegionIndex(cell);

	// convert index to XZ
	XZPAIR region;
	region.x = midRegion % regionsAcross;
	region.z = (midRegion - region.x) / regionsAcross;

	for (int z = -radius; z <= radius; z++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			// lookup region
			XZPAIR iregion;
			iregion.x = region.x + x;
			iregion.z = region.z + z;

			if (iregion.x >= 0 && iregion.x < regionsAcross &&
				iregion.z >= 0 && iregion.z < regionsDown)
			{
				g_levMap->SpoolRegion(spoolContext, iregion.x + iregion.z * regionsAcross);
			}
		}
	}
}

bool CWorld::IsLevelLoaded()
{
	return g_levMap;
}

//-------------------------------------------------------------
// Forcefully spools entire level regions and area datas
//-------------------------------------------------------------
void CWorld::SpoolAllRegions()
{
	if (!IsLevelLoaded())
		return;

	Msg("Spooling ALL regions...\n");

	CFileStream stream(g_levFile, false);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &stream;
	spoolContext.lumpInfo = &g_levInfo;

	int totalRegions = g_levMap->GetRegionsAcross() * g_levMap->GetRegionsDown();
		
	for (int i = 0; i < totalRegions; i++)
	{
		g_levMap->SpoolRegion(spoolContext, i);
	}
}

ModelRef_t* CWorld::GetModelByIndex(int modelIndex)
{
	return g_levModels.GetModelByIndex(modelIndex);
}

ModelRef_t* CWorld::GetModelByName(const char* name)
{
	int modelIndex = g_levModels.FindModelIndexByName(name);
	return GetModelByIndex(modelIndex);
}

int CWorld::MapHeight(const VECTOR_NOPAD& position)
{
	return g_levMap->MapHeight(position);
}

int CWorld::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane)
{
	int fr = g_levMap->FindSurface(position, outNormal, outPoint, outPlane);

	 if (outPlane.surfaceType == (int)SurfaceType::Grass)
	{
		// TODO: move this hardcoding away
#if 0
		if (gInGameCutsceneActive && gCurrentMissionNumber == 23 && gInGameCutsceneID == 0)
			outPoint.vy += isin((pos->vx + pos->vz) * 2) >> 9;
		else
#endif
			outPoint.vy += (isin((position.vx + position.vz) * 2) >> 8) / 3;

		return fr >> 1;
	}

	return fr;
}

//-------------------------------------------------------------

void CWorld::QueryCollision(const VECTOR_NOPAD& queryPos, int queryDist, const BoxCollisionFn& func, void* object)
{
	if (!func)
		return;

	auto& mapInfo = g_levMap->GetMapInfo();
	const int squared_reg_size = mapInfo.region_size * mapInfo.region_size;

	XZPAIR initial, cell;
	g_levMap->WorldPositionToCellXZ(initial, queryPos, XZPAIR{ -squared_reg_size, -squared_reg_size });

	static Array<CELL_OBJECT*> collisionObjects;
	collisionObjects.reserve(32);
	collisionObjects.clear();

	CELL_ITERATOR_CACHE iteratorCache;

	// collect objects
	cell.x = initial.x;
	for (int i = 0; i < 2; i++)
	{
		cell.z = initial.z;
		for (int j = 0; j < 2; j++)
		{
			CWorld::ForEachCellObjectAt(cell, [](int listType, CELL_OBJECT* co) {
				if (listType != -1) // TODO: check event objects too!
					return false;

				ModelRef_t* ref = g_levModels.GetModelByIndex(co->type);

				if (!ref->enabled)
					return true;

				if (ref && ref->baseInstance)
					ref = ref->baseInstance;

				if (ref->model && ref->model->GetCollisionBoxCount())
				{
					collisionObjects.append(co);
				}

				return true;
			}, &iteratorCache);

			cell.z++;
		}

		cell.x++;
	}

	// add event cell objects to list
	// TODO: check distance!
	for(usize i = 0; i < m_CellObjects.size(); i++)
		collisionObjects.append(&m_CellObjects[i]);

	// check collisions
	for (usize i = 0; i < collisionObjects.size(); i++)
	{
		CELL_OBJECT* co = collisionObjects[i];
		co->pad = 255;
		ModelRef_t* ref = g_levModels.GetModelByIndex(co->type);

		if (ref && ref->baseInstance)
			ref = ref->baseInstance;

		if (!ref)
			continue;

		MODEL* model = ref->model;

		if (!model)
			return;

		int numCollisionBoxes = model->GetCollisionBoxCount();

		int dx = co->pos.vx - queryPos.vx;
		int dz = co->pos.vz - queryPos.vz;
		int yang = -co->yang & 63;

		int sphereSq = model->bounding_sphere + queryDist;

		if (dx * dx + dz * dz > sphereSq * sphereSq)
			continue;

		for (int j = 0; j < numCollisionBoxes; j++)
		{
			COLLISION_PACKET* collide = model->pCollisionBox(j);

			BUILDING_BOX bbox;
			// box 'rotated' by matrix
			// [A] FIXME: replace add+shift by division
			bbox.pos.vx = co->pos.vx + FIXEDH(collide->xpos * g_objectMatrixFixed[yang].m[0][0] + collide->zpos * g_objectMatrixFixed[yang].m[2][0]);
			bbox.pos.vy = co->pos.vy + collide->ypos;
			bbox.pos.vz = co->pos.vz + FIXEDH(collide->xpos * g_objectMatrixFixed[yang].m[0][2] + collide->zpos * g_objectMatrixFixed[yang].m[2][2]);

#if 0
			// [A] purposely make chair box smaller for Tanner
			if (cp->controlType == CONTROL_TYPE_TANNERCOLLIDER && (model->flags2 & MODEL_FLAG_CHAIR))
			{
				bbox.xsize = (collide->zsize >> 1) - 20;
				bbox.zsize = (collide->xsize >> 1) - 20;
			}
			else
#endif
			{
				bbox.xsize = collide->zsize >> 1;
				bbox.zsize = collide->xsize >> 1;
			}

			bbox.height = collide->ysize;
			bbox.theta = (co->yang + collide->yang) * 64 & 0xfff;
			bbox.modelRef = ref;

			if (!func(bbox, co, object))
				return;
		}
	}
}

// push event cell object
// any collision checks afterwards will have an effect with it
int CWorld::PushCellObject(const CELL_OBJECT& object)
{
	int num = m_CellObjects.size();
	m_CellObjects.append(object);
	return num;
}

// purges list of recently added objects by PushCellObject
void CWorld::PurgeCellObjects()
{
	m_CellObjects.clear();
}

void CWorld::ForEachCellObjectAt(const XZPAIR& cell, const CellObjectIterateFn& func, CELL_ITERATOR_CACHE* iteratorCache /*= nullptr*/)
{
	CBaseLevelMap* levMap = g_levMap;
	bool driver2Map = levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	if (driver2Map)
	{
		CDriver2LevelMap* levMapDriver2 = (CDriver2LevelMap*)levMap;

		// Driver 2 map iteration
		CELL_ITERATOR_D2 ci;
		ci.cache = iteratorCache;

		PACKED_CELL_OBJECT* ppco = levMapDriver2->GetFirstPackedCop(&ci, cell);

		// walk each cell object in cell
		while (ppco)
		{
			if (!func(ci.listType, ci.co))
				break;

			ppco = levMapDriver2->GetNextPackedCop(&ci);
		}
	}
	else
	{
		CDriver1LevelMap* levMapDriver1 = (CDriver1LevelMap*)levMap;

		// Driver 1 map iteration
		CELL_ITERATOR_D1 ci;
		ci.cache = iteratorCache;

		CELL_OBJECT* pco = levMapDriver1->GetFirstCop(&ci, cell);

		// walk each cell object in cell
		while (pco)
		{
			if (!func(-1, pco))
				break;
			pco = levMapDriver1->GetNextCop(&ci);
		}
	}
}

void CWorld::EndStep()
{
	StepCount++;
}

void CWorld::ResetStep()
{
	StepCount = 0;
}
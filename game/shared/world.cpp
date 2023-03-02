#include "game/pch.h"
#include "world.h"

#include "routines/regions_d1.h"
#include "routines/regions_d2.h"

OUT_CITYLUMP_INFO		g_levInfo;
CDriverLevelTextures	g_levTextures;
CDriverLevelModels		g_levModels;
CBaseLevelMap*			g_levMap = nullptr;

Map<int, Array<CELL_OBJECT>>	CWorld::CellObjects;
Array<DRAWABLE>					CWorld::Drawables;
Map<int, CELL_LIST_DESC>		CWorld::CellLists;
int								CWorld::StepCount = 0;

Matrix4x4	g_objectMatrix[64];
MATRIX		g_objectMatrixFixed[64];
extern int g_cellsDrawDistance;

inline int PackXZCell(const XZPAIR& cell)
{
	return (cell.x & 65535) | (cell.z & 65535) << 16;
}

inline void UnpackXZCell(XZPAIR& cell, int packedCellId)
{
	cell.x = packedCellId & 65535;
	cell.z = packedCellId >> 16 & 65535;
}

void CWorld::Lua_Init(sol::state& lua)
{
	auto engine = lua["engine"].get_or_create<sol::table>();

	LUADOC_GLOBAL();
	{
		LUADOC_TYPE("World");

		auto world = engine["World"].get_or_create<sol::table>();

		world[LUADOC_M("FindTextureDetail", "(name: string) : TexDetailInfo - returns texture detail with specific name")]
			= &FindTextureDetail;

		world[LUADOC_M("StepTextureDetailPalette", "(detail: TexDetailInfo, start: int, end: int) - steps the texture detail palette at specific range")]
			= &StepTextureDetailPalette;

		world[LUADOC_M("LoadLevel", "(filename: string) : boolean - loads level from file")] 
			= &LoadLevel;

		world[LUADOC_M("UnloadLevel", "(void)")]
			= &UnloadLevel;

		world[LUADOC_M("SpoolAllRegions", "(void) - load all models, regions and textures")] 
			= &SpoolAllRegions;

		world[LUADOC_M("SpoolRegions", "(position: fix.VECTOR, radius: int) : int - spool regions at specified point and radius")]
			= &SpoolRegions;

		world[LUADOC_M("IsLevelLoaded", "(void) : boolean")]
			= &IsLevelLoaded;

		world[LUADOC_M("GetModelByIndex", "(index: int) : ModelRef - returns model reference by specified index")] 
			= &GetModelByIndex;

		world[LUADOC_M("GetModelByName", "(name: string) : ModelRef - returns model reference by specified name")]
			= &GetModelByName;

		world[LUADOC_M("MapHeight", "(position: fix.VECTOR) : int - returns height value at specified 3D point")]
			= &MapHeight;

		world[LUADOC_M("GetSurfaceIndex", "(position: fix.VECTOR) : int - returns surface (road) index value at specified 3D point")]
			= &GetSurfaceIndex;

		world[LUADOC_M("QueryCollision", "(queryPos: fix.VECTOR, queryDist: int, func: function(box: BUILDING_BOX, cellObj: CELL_OBJECT)) - performs cell object query")]
			= [](const VECTOR_NOPAD& queryPos, int queryDist, sol::function& func) {
			QueryCollision(queryPos, queryDist, [&func](const BUILDING_BOX& box, CELL_OBJECT* co) {
				bool result = func.call(box, co);
				return result;
			});
		},

		world[LUADOC_M("PushCellObject", "(cellObj: CELL_OBJECT) - push event cell object. Any collision checks afterwards will have an effect with it")]
			= &PushCellObject;

		world[LUADOC_M("PurgeCellObjects", "(void) - purges list of recently added objects by PushCellObject")]
			= &PurgeCellObjects;

		world[LUADOC_M("AddDrawable", "(drawable: DRAWABLE) - add a DRAWABLE object. No collisions will be made with it")]
			= &AddDrawable;

		world[LUADOC_M("CreateCellList", "(listNumber: int) : CELL_LIST_DESC - add cell list handler for renderer")]
			= &CreateCellList;

		world[LUADOC_M("RemoveCellList", "(listNumber: int) - removes cell list. All objects will not be rendered")]
			= &RemoveCellList;

		world[LUADOC_M("EndStep", "(void) - finalizes the game step, incrementing step count by 1.")]
			= &EndStep;

		world[LUADOC_M("ResetStep", "(void) - resets world step count")]
			= &ResetStep;

		world[LUADOC_M("StepCount", "(void) : int - world step count")]
			= []() {return StepCount; };
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<DRAWABLE>(
			LUADOC_T("DRAWABLE"),
			sol::call_constructor, sol::factories(
				[](const Vector3D& position, const Vector3D& angles, const Vector3D& scale, const int& model) {
					return DRAWABLE{ position, angles, scale, model };
				},
				[](const sol::table& table) {
					Vector3D default(1.0f);
					Vector3D scale = table.get_or<Vector3D>("scale", default);
					return DRAWABLE{ (Vector3D&)table["position"], (Vector3D&)table["angles"], scale, table["model"] };
				},
				[]() { return DRAWABLE{ 0 }; }),
			LUADOC_P("position", "<vec.vec3>"), 
			&DRAWABLE::position,

			LUADOC_P("scale", "<vec.vec3>"),
			&DRAWABLE::scale,

			LUADOC_P("angles", "<vec.vec3> - radian angles"),
			&DRAWABLE::angles,

			LUADOC_P("model", "<int> - model index"),
			&DRAWABLE::model
		);
	}

	// collision query box
	{
		LUADOC_TYPE();
		lua.new_usertype<BUILDING_BOX>(
			LUADOC_T("BUILDING_BOX"),

			LUADOC_P("name"), &BUILDING_BOX::pos,
			LUADOC_P("xsize", "<int>"), & BUILDING_BOX::xsize,
			LUADOC_P("zsize", "<int>"), & BUILDING_BOX::zsize,
			LUADOC_P("theta", "<int>"), & BUILDING_BOX::theta,
			LUADOC_P("height", "<int>"), & BUILDING_BOX::height,
			LUADOC_P("modelRef", "<int>"), & BUILDING_BOX::modelRef
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<CELL_LIST_DESC>(
			LUADOC_T("CELL_LIST_DESC"),
			LUADOC_P("position", "<fix.VECTOR>"), 
			sol::property([](const CELL_LIST_DESC& self) {return self.position; }, [](CELL_LIST_DESC& dest, const VECTOR_NOPAD& vec) {dest.position = vec; dest.dirty = true; }),

			LUADOC_P("rotation", "<fix.VECTOR>"), 
			sol::property([](const CELL_LIST_DESC& self) {return self.rotation; }, [](CELL_LIST_DESC& dest, const VECTOR_NOPAD& vec) {dest.rotation = vec; dest.dirty = true; }),

			LUADOC_P("pivotMatrix", "<vec.mat4> - offset matrix which makes pivot point"), 
			sol::property([](const CELL_LIST_DESC& self) {return self.pivotMatrix; }, [](CELL_LIST_DESC& dest, const Matrix4x4& newMat) {dest.pivotMatrix = newMat; dest.dirty = true; }),

			LUADOC_P("visible", "<boolean>"), 
			&CELL_LIST_DESC::visible
		);
	}

	// level properties
	{
		LUADOC_TYPE();
		lua.new_usertype<LevelRenderProps>(
			LUADOC_T("LevelRenderProps"),

			LUADOC_P("ambientColor", "<vec.vec3>"), &LevelRenderProps::ambientColor,
			LUADOC_P("lightColor", "<vec.vec3>"), &LevelRenderProps::lightColor,

			LUADOC_P("fogColor", "<vec.vec3>"), &LevelRenderProps::fogColor,
			LUADOC_P("fogParams", "<vec.vec3>"), &LevelRenderProps::fogParams,

			LUADOC_P("nightAmbientScale", "<float>"), &LevelRenderProps::nightAmbientScale,
			LUADOC_P("nightLightScale", "<float>"), &LevelRenderProps::nightLightScale,
			LUADOC_P("ambientScale", "<float>"), &LevelRenderProps::ambientScale,
			LUADOC_P("lightScale", "<float>"), &LevelRenderProps::lightScale,
			LUADOC_P("nightMode", "<boolean>"), &LevelRenderProps::nightMode,
			LUADOC_P("noLod", "<boolean>"), &LevelRenderProps::noLod,

			LUADOC_P("displayCollisionBoxes", "<boolean>"), &LevelRenderProps::displayCollisionBoxes,
			LUADOC_P("displayHeightMap", "<boolean>"), &LevelRenderProps::displayHeightMap,
			LUADOC_P("displayAllCellLevels", "<boolean>"), &LevelRenderProps::displayAllCellLevels,
			LUADOC_P("displayCellObjectList", "<int>"), & LevelRenderProps::displayCellObjectList
		);
	}

	{
		MAKE_PROPERTY_REF(lua, ModelRef_t*);
		LUADOC_TYPE();
		lua.new_usertype<ModelRef_t>(
			LUADOC_T("ModelRef"),

			LUADOC_P("name"), &ModelRef_t::name,
			LUADOC_P("index", "<int>"), &ModelRef_t::index,
			LUADOC_P("highDetailId", "<int>"), &ModelRef_t::highDetailId,
			LUADOC_P("lowDetailId", "<int>"), &ModelRef_t::lowDetailId,
			LUADOC_P("enabled", "<boolean>"), &ModelRef_t::enabled,
			LUADOC_P("lightingLevel", "<float>"), &ModelRef_t::lightingLevel,
			LUADOC_P("shapeFlags", "<ShapeFlags>"), sol::property([](const ModelRef_t& thisRef) {
				return thisRef.model ? thisRef.model->shape_flags : 0;
			}),
			LUADOC_P("modelFlags", "<ModelFlags>"), sol::property([](const ModelRef_t& thisRef) {
				return thisRef.model ? thisRef.model->flags2 : 0;
			})
		);
	}

	{
		LUADOC_TYPE();
		LUA_BEGIN_ENUM(ModelFlags2);
		lua.new_enum<ModelFlags2>(LUADOC_T("ModelFlags"), {
			LUA_ENUM(MODEL_FLAG_MEDIAN, "Median"),
			LUA_ENUM(MODEL_FLAG_JUNC, "Junction"),
			LUA_ENUM(MODEL_FLAG_ALLEY, "Alley"),
			LUA_ENUM(MODEL_FLAG_INDOORS, "Indoors"),
			LUA_ENUM(MODEL_FLAG_CHAIR, "Chair"),
			LUA_ENUM(MODEL_FLAG_BARRIER, "Barrier"),
			LUA_ENUM(MODEL_FLAG_SMASHABLE, "Smashable"),
			LUA_ENUM(MODEL_FLAG_LAMP, "Lamp"),
			LUA_ENUM(MODEL_FLAG_TREE, "Tree"),
			LUA_ENUM(MODEL_FLAG_GRASS, "Grass"),
			LUA_ENUM(MODEL_FLAG_PATH, "Path"),
		});
	}

	{
		LUADOC_TYPE();
		LUA_BEGIN_ENUM(ModelShapeFlags);
		lua.new_enum<ModelShapeFlags>(LUADOC_T("ShapeFlags"), {
			LUA_ENUM(SHAPE_FLAG_LITPOLY, "LitPoly"),
			LUA_ENUM(SHAPE_FLAG_BSPDATA, "BSPData"),
			LUA_ENUM(SHAPE_FLAG_TRANS, "Trans"),
			LUA_ENUM(SHAPE_FLAG_NOCOLLIDE, "NoCollide"),
			LUA_ENUM(SHAPE_FLAG_WATER, "Water"),
			LUA_ENUM(SHAPE_FLAG_AMBIENT2, "Ambient2"),
			LUA_ENUM(SHAPE_FLAG_AMBIENT1, "Ambient1"),
			LUA_ENUM(SHAPE_FLAG_TILE, "Tile"),
			LUA_ENUM(SHAPE_FLAG_SHADOW, "Shadow"),
			LUA_ENUM(SHAPE_FLAG_ALPHA, "Alpha"),
			LUA_ENUM(SHAPE_FLAG_ROAD, "Road"),
			LUA_ENUM(SHAPE_FLAG_SPRITE, "Sprite"),
		});
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
			LUADOC_P("pos", "<fix.VECTOR>"), & CELL_OBJECT::pos,
			LUADOC_P("yang", "<int> angle (0 - 63)"), &CELL_OBJECT::yang,
			LUADOC_P("type", "<int> model index"), &CELL_OBJECT::type
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
ushort g_hwTexturePagesDirty[128] = { 0xFFFF };
extern TextureID g_whiteTexture;

// Creates hardware texture
void CWorld::InitHWTexturePage(CTexturePage* tpage)
{
	if (!tpage)
		return;

	const int tpageId = tpage->GetId();
	if (!g_hwTexturePagesDirty[tpageId])
		return;

	const TexBitmap_t& bitmap = tpage->GetBitmap();

	if (bitmap.data == nullptr)
		return;

	// alloc 4 channels
	const int imgSize = TEXPAGE_SIZE * 4;
	uint* color_data = (uint*)Memory::alloc(imgSize);

	memset(color_data, 0, imgSize);

	const int numDetails = tpage->GetDetailCount();
	for (int i = 0; i < numDetails; i++)
		tpage->ConvertIndexedTextureToRGBA(color_data, i, &bitmap.clut[i], false, false);

	if (g_hwTexturePagesDirty[tpageId] & 1)
	{
		TextureID& texture = g_hwTexturePages[tpageId][0];
		// create new or update
		if (texture == g_whiteTexture)
			texture = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
		else
			GR_UpdateRGBATexture(texture, TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
	}

	// also load different palettes
	for (int pal = 1; pal < 16; pal++)
	{
		bool anyMatched = false;

		for (int i = 0; i < numDetails; i++)
		{
			const int extraPal = pal - 1;
			TexDetailInfo_t* detail = tpage->GetTextureDetail(i);

			if (detail->extraCLUTs[extraPal])
			{
				tpage->ConvertIndexedTextureToRGBA(color_data, i, detail->extraCLUTs[extraPal], false, false);
				anyMatched = true;
			}
		}

		if (anyMatched && (g_hwTexturePagesDirty[tpageId] & (1 << pal)))
		{
			TextureID& texture = g_hwTexturePages[tpageId][pal];
			if (texture == g_whiteTexture)
				texture = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
			else
				GR_UpdateRGBATexture(texture, TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
		}
	}

	g_hwTexturePagesDirty[tpageId] = 0;
	
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
		g_hwTexturePagesDirty[tpageId] = 0xffff;
	}
}

// returns hardware texture
TextureID CWorld::GetHWTexture(int tpage, int pal)
{
	if (tpage < 0 || tpage >= 128 ||
		pal < 0 || pal >= 16)
		return g_whiteTexture;

	if (g_hwTexturePagesDirty[tpage] & (1 << pal))
	{
		// TODO: do not recalc entire tpage!!!
		InitHWTexturePage(GetTPage(tpage));
	}

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
		g_hwTexturePagesDirty[i] = 0xffff;
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

void CWorld::StepTextureDetailPalette(const TexDetailInfo_t* detail, int start, int stop)
{
	if (!detail || start == -1 || stop == -1)
		return;

	ASSERT(start < 16);
	ASSERT(stop < 16);

	const int tpageId = detail->pageNum;
	CTexturePage* tpage = GetTPage(tpageId);

	const TexBitmap_t& bitmap = tpage->GetBitmap();
	if (bitmap.data == nullptr)
		return;

	ushort* bufaddr = bitmap.clut[detail->detailNum].colors;

	ushort temp = bufaddr[start];
	memmove(bufaddr + start, bufaddr + start + 1, (stop - start) * sizeof(ushort));
	bufaddr[stop] = temp;

	g_hwTexturePagesDirty[tpageId] |= 1;
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
		CellLists.clear();

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

	const bool driver2Map = g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;
	
	CRender_Level::DrawMap(view.position, view.angles.y, frustumVolume);
	GR_SetCullMode(CULL_FRONT);
	{
		const VECTOR_NOPAD cameraPosition = ToFixedVector(view.position);
		XZPAIR cameraPosCell;
		g_levMap->WorldPositionToCellXZ(cameraPosCell, cameraPosition);

		const int drawDistInCell = SquareRoot0(g_cellsDrawDistance >> 1);

		for (auto it = CellObjects.begin(); it != CellObjects.end(); ++it)
		{
			XZPAIR cell;
			UnpackXZCell(cell, it.key());
			if (abs(cameraPosCell.x - cell.x) > drawDistInCell ||
				abs(cameraPosCell.z - cell.z) > drawDistInCell)
			{
				continue;
			}

			Array<CELL_OBJECT>& objs = *it;
			for (usize i = 0; i < objs.size(); i++)
				CRender_Level::DrawCellObject(objs[i], view.position, view.angles.y, frustumVolume, driver2Map);
		}
	}

	for (usize i = 0; i < Drawables.size(); i++)
	{
		CRender_Level::DrawObject(Drawables[i], view.position, frustumVolume, driver2Map);
	}
	Drawables.clear();
}

int CWorld::SpoolRegions(const VECTOR_NOPAD& position, int radius)
{
	if (!IsLevelLoaded())
		return 0;

	CFileStream stream(g_levFile, false);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &stream;
	spoolContext.lumpInfo = &g_levInfo;

	const int regionsAcross = g_levMap->GetRegionsAcross();
	const int regionsDown = g_levMap->GetRegionsDown();

	// get center region
	XZPAIR cell;
	g_levMap->WorldPositionToCellXZ(cell, position);

	const int midRegion = g_levMap->GetRegionIndex(cell);

	// convert index to XZ
	XZPAIR region;
	region.x = midRegion % regionsAcross;
	region.z = (midRegion - region.x) / regionsAcross;

	int numSpooled = 0;
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
				if (g_levMap->SpoolRegion(spoolContext, iregion.x + iregion.z * regionsAcross))
				{
					numSpooled++;
				}
			}
		}
	}
	return numSpooled;
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
	sdPlane outPlane;
	VECTOR_NOPAD outPoint;
	g_levMap->FindSurface(position, outPoint, outPlane);

	return outPoint.vy;
}

void CWorld::FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane)
{
	g_levMap->FindSurface(position, outPoint, outPlane);

	if (outPlane.b == 0)
	{
		outNormal.vx = 0;
		outNormal.vy = 4096;
		outNormal.vz = 0;
	}
	else
	{
		outNormal.vx = (int)outPlane.a >> 2;
		outNormal.vy = (int)outPlane.b >> 2;
		outNormal.vz = (int)outPlane.c >> 2;
	}

	const bool isEventSurface = outPlane.surfaceType - 16U < 16; // in range of 16-31
	if (isEventSurface)
	{
		const int eventId = outPlane.surfaceType & ~16;

		// TODO: 2D polygon shape for bounds and plane normal
	}
}

int CWorld::GetSurfaceIndex(const VECTOR_NOPAD& position)
{
	return g_levMap->GetSurfaceIndex(position);
}

//-------------------------------------------------------------

void CWorld::QueryCollision(const VECTOR_NOPAD& queryPos, int queryDist, const BoxCollisionFn& func)
{
	if (!func)
		return;

	const OUT_CELL_FILE_HEADER& mapInfo = g_levMap->GetMapInfo();
	const int squared_reg_size = mapInfo.region_size * mapInfo.region_size;

	XZPAIR initial;
	g_levMap->WorldPositionToCellXZ(initial, queryPos, XZPAIR{ -squared_reg_size, -squared_reg_size });

	static Array<CELL_OBJECT*> collisionObjects;
	static Array<const ModelRef_t*> collisionObjectModels;
	collisionObjects.reserve(32);
	collisionObjectModels.reserve(32);
	collisionObjects.clear();
	collisionObjectModels.clear();

	CELL_ITERATOR_CACHE iteratorCache;

	// collect objects
	XZPAIR cell = initial;
	for (int i = 0; i < 2; i++)
	{
		cell.z = initial.z;
		for (int j = 0; j < 2; j++)
		{
			auto checkAndAddCellObj = [](CELL_OBJECT* co)
			{
				const ModelRef_t* ref = g_levModels.GetModelByIndex(co->type);

				if (!ref || !ref->enabled)
					return true;

				if (ref->baseInstance)
					ref = ref->baseInstance;

				// spooled?
				if (!ref->model)
					return true;

				if (ref->model && ref->model->GetCollisionBoxCount())
				{
					collisionObjects.append(co);
					collisionObjectModels.append(ref);
				}

				return true;
			};

			// add map objects
			CWorld::ForEachCellObjectAt(cell, [&checkAndAddCellObj](int listType, CELL_OBJECT* co) {
				if (listType != -1)
				{
					auto& foundCellList = CWorld::CellLists.find(listType);
					if (foundCellList != CWorld::CellLists.end())
					{
						CELL_LIST_DESC& cellList = *foundCellList;
						if (!cellList.visible)
							return true;
					}
					else
					{
						return true;
					}
				}

				return checkAndAddCellObj(co);
			}, &iteratorCache);

			// add event cell objects to list
			const int cellIndex = PackXZCell(cell);
			auto& cellObjList = CellObjects.find(cellIndex);

			if (cellObjList != CellObjects.end())
			{
				Array<CELL_OBJECT>& objs = *cellObjList;
				for (usize i = 0; i < objs.size(); i++)
				{
					checkAndAddCellObj(&objs[i]);
				}
			}

			cell.z++;
		}

		cell.x++;
	}

	// check collisions
	for (usize i = 0; i < collisionObjects.size(); i++)
	{
		CELL_OBJECT* co = collisionObjects[i];
		const ModelRef_t* ref = collisionObjectModels[i];

		if (!ref)
			continue;

		const MODEL* model = ref->model;

		const int dx = co->pos.vx - queryPos.vx;
		const int dz = co->pos.vz - queryPos.vz;

		const int sphereSq = model->bounding_sphere + queryDist;

		if (dx * dx + dz * dz > sphereSq * sphereSq)
			continue;

		const int yang = -co->yang & 63;
		const int numCollisionBoxes = model->GetCollisionBoxCount();

		for (int j = 0; j < numCollisionBoxes; j++)
		{
			const COLLISION_PACKET* collide = model->pCollisionBox(j);

			BUILDING_BOX bbox;
			// box 'rotated' by matrix
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

			if (!func(bbox, co))
				return;
		}
	}
}

// push event cell object
// any collision checks afterwards will have an effect with it
int CWorld::PushCellObject(const CELL_OBJECT& object)
{
	XZPAIR cell;
	g_levMap->WorldPositionToCellXZ(cell, object.pos);

	const int cellIndex = PackXZCell(cell);
	auto& cellObjList = CellObjects.find(cellIndex);

	if (cellObjList != CellObjects.end())
	{
		Array<CELL_OBJECT>& objs = *cellObjList;
		objs.append(object);
	}
	else
	{
		// alloc new list
		Array<CELL_OBJECT> objs;
		objs.append(object);

		CellObjects.insert(cellIndex, objs);
	}

	return cellIndex;
}

// purges list of recently added objects by PushCellObject
void CWorld::PurgeCellObjects()
{
	for (auto it = CellObjects.begin(); it != CellObjects.end(); ++it)
	{
		Array<CELL_OBJECT>& objs = *it;
		objs.clear();
	}
}

// adds a drawable object for one draw frame
void CWorld::AddDrawable(const DRAWABLE& drawable)
{
	Drawables.append(drawable);
}

CELL_LIST_DESC& CWorld::CreateCellList(int list)
{
	ASSERT(m_CellLists.contains(list) == false);

	auto& found = CellLists.find(list);
	if (found != CellLists.end())
	{
		return *found;
	}

	auto& newList = CellLists.insert(list, CELL_LIST_DESC{});
	return *newList;
}

void CWorld::RemoveCellList(int list)
{
	auto& found = CellLists.find(list);
	if (found != CellLists.end())
		CellLists.remove(found);
}

void CWorld::ForEachCellObjectAt(const XZPAIR& cell, const CellObjectIterateFn& func, CELL_ITERATOR_CACHE* iteratorCache /*= nullptr*/)
{
	const CBaseLevelMap* levMap = g_levMap;
	const bool driver2Map = levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	if (driver2Map)
	{
		const CDriver2LevelMap* levMapDriver2 = (CDriver2LevelMap*)levMap;

		// Driver 2 map iteration
		CELL_ITERATOR_D2 ci;
		ci.cache = iteratorCache;

		// walk each cell object in cell
		for (PACKED_CELL_OBJECT* ppco = levMapDriver2->GetFirstPackedCop(&ci, cell); 
			ppco != nullptr; ppco = levMapDriver2->GetNextPackedCop(&ci))
		{
			if (!func(ci.listType, ci.co))
				break;
		}
	}
	else
	{
		const CDriver1LevelMap* levMapDriver1 = (CDriver1LevelMap*)levMap;

		// Driver 1 map iteration
		CELL_ITERATOR_D1 ci;
		ci.cache = iteratorCache;

		// walk each cell object in cell
		for (CELL_OBJECT* pco = levMapDriver1->GetFirstCop(&ci, cell); 
			pco != nullptr; pco = levMapDriver1->GetNextCop(&ci))
		{
			if (!func(-1, pco))
				break;
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
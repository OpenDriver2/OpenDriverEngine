#include "core/core_common.h"
#include "core/IFileSystem.h"

#include "math/squareroot0.h"
#include "math/convert.h"
#include "math/psx_matrix.h"
#include "world.h"

#include "routines/regions_d1.h"
#include "routines/regions_d2.h"
#include "routines/textures.h"
#include "game/render/render_level.h"
#include "game/shared/players.h"
#include "game/render/render_model.h"
#include "game/render/render_cars.h"

#include "materialsystem1/IMaterialSystem.h"

OUT_CITYLUMP_INFO		g_levInfo;
CDriverLevelTextures	g_levTextures;
CDriverLevelModels		g_levModels;
CBaseLevelMap*			g_levMap = nullptr;

Array<DRAWABLE>					CWorld::Drawables{ PP_SL };
Map<int, Array<CELL_OBJECT>>	CWorld::CellObjects{ PP_SL };
Map<int, CELL_LIST_DESC>		CWorld::CellLists{ PP_SL };
int								CWorld::StepCount = 0;

Matrix4x4	g_objectMatrix[64];
MATRIX		g_objectMatrixFixed[64];
extern int	g_cellsDrawDistance;

inline int PackXZCell(const XZPAIR& cell)
{
	return (cell.x & 65535) | (cell.z & 65535) << 16;
}

inline void UnpackXZCell(XZPAIR& cell, int packedCellId)
{
	cell.x = packedCellId & 65535;
	cell.z = packedCellId >> 16 & 65535;
}

DRAWABLE::DRAWABLE(const Vector3D& position, const Vector3D& angles, const Vector3D& scale, const int model)
	: position(position), angles(angles), scale(scale), model(model)
{
}

DRAWABLE::DRAWABLE(const esl::LuaTable& table)
{
	scale = table["scale"].SafeGet<const Vector3D&>(vec3_unit);
	position = table["position"];
	angles = table["angles"];
	model = table["model"];
}

void CELL_LIST_DESC::SetPivotMatrix(const Matrix4x4& newPivot)
{
	pivotMatrix = newPivot;
	dirty = true;
}

void CELL_LIST_DESC::SetPosition(const VECTOR_NOPAD& newPos)
{
	position = newPos;
	dirty = true;
}

void CELL_LIST_DESC::SetRotation(const VECTOR_NOPAD& newRot)
{
	rotation = newRot;
	dirty = true;
}

EQSCRIPT_TYPE_BEGIN(DRAWABLE)
	EQSCRIPT_BIND_CONSTRUCTOR()
	EQSCRIPT_BIND_CONSTRUCTOR(const Vector3D&, const Vector3D&, const Vector3D&, int)
	EQSCRIPT_BIND_CONSTRUCTOR(const esl::LuaTable&)
	EQSCRIPT_BIND_VAR(position)
	EQSCRIPT_BIND_VAR(scale)
	EQSCRIPT_BIND_VAR(angles)
	EQSCRIPT_BIND_VAR(model)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(BUILDING_BOX)
	EQSCRIPT_BIND_VAR(pos)
	EQSCRIPT_BIND_VAR(xsize)
	EQSCRIPT_BIND_VAR(zsize)
	EQSCRIPT_BIND_VAR(theta)
	EQSCRIPT_BIND_VAR(height)
	EQSCRIPT_BIND_VAR(modelRef)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(CELL_LIST_DESC)
	EQSCRIPT_BIND_VAR_EX_SET(position, SetPosition)
	EQSCRIPT_BIND_VAR_EX_SET(rotation, SetRotation)
	EQSCRIPT_BIND_VAR_EX_SET(pivotMatrix, SetPivotMatrix)
	EQSCRIPT_BIND_VAR(visible)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(LevelRenderProps)
	EQSCRIPT_BIND_VAR(ambientColor)
	EQSCRIPT_BIND_VAR(lightColor)

	EQSCRIPT_BIND_VAR(fogColor)
	EQSCRIPT_BIND_VAR(fogParams)

	EQSCRIPT_BIND_VAR(nightAmbientScale)
	EQSCRIPT_BIND_VAR(nightLightScale)
	EQSCRIPT_BIND_VAR(ambientScale)
	EQSCRIPT_BIND_VAR(lightScale)
	EQSCRIPT_BIND_VAR(nightMode)
	EQSCRIPT_BIND_VAR(noLod)

	EQSCRIPT_BIND_VAR(displayCollisionBoxes)
	EQSCRIPT_BIND_VAR(displayHeightMap)
	EQSCRIPT_BIND_VAR(displayAllCellLevels)
	EQSCRIPT_BIND_VAR(displayCellObjectList)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(ModelRef_t)
	EQSCRIPT_BIND_VAR(name)
	EQSCRIPT_BIND_VAR(index)
	EQSCRIPT_BIND_VAR(highDetailId)
	EQSCRIPT_BIND_VAR(lowDetailId)
	EQSCRIPT_BIND_VAR(enabled)
	EQSCRIPT_BIND_VAR(lightingLevel)
	EQSCRIPT_BIND_STATIC_FUNC("GetShapeFlags", +[](const ModelRef_t& thisRef) {
		return thisRef.model ? thisRef.model->shape_flags : 0;
	})
	EQSCRIPT_BIND_STATIC_FUNC("GetModelFlags", +[](const ModelRef_t& thisRef) {
		return thisRef.model ? thisRef.model->flags2 : 0;
	})
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(TexDetailInfo_t)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(CELL_OBJECT)
	//sol::call_constructor, sol::factories(
	//[](const VECTOR_NOPAD& position, const ubyte& yang, const ushort& type) {
	//	return CELL_OBJECT{ position, 0, yang, type };
	//},
	//[](const sol::table& table) {
	//	return CELL_OBJECT{ (VECTOR_NOPAD&)table["position"], 0, table["yang"], table["type"] };
	//},
	//[]() { return CELL_OBJECT{ 0 }; }),
	EQSCRIPT_BIND_CONSTRUCTOR()
	EQSCRIPT_BIND_VAR(pos)
	EQSCRIPT_BIND_VAR(yang)
	EQSCRIPT_BIND_VAR(type)
EQSCRIPT_TYPE_END


void CWorld::Lua_Init(const esl::ScriptState& state)
{
	state.RegisterClass<DRAWABLE>();
	state.RegisterClass<BUILDING_BOX>();
	state.RegisterClass<CELL_LIST_DESC>();
	state.RegisterClass<LevelRenderProps>();
	state.RegisterClass<ModelRef_t>();
	state.RegisterClass<TexDetailInfo_t>();
	state.RegisterClass<CELL_OBJECT>();

	esl::LuaTable engineTbl = eslSys::GetOrCreateGlobalTable(state, "engine");

	{
		esl::LuaTable world = engineTbl["World"].CreateTable();

		world["FindTextureDetail"] = EQSCRIPT_CFUNC(FindTextureDetail);
		world["StepTextureDetailPalette"] = EQSCRIPT_CFUNC(StepTextureDetailPalette);
		world["LoadLevel"] = EQSCRIPT_CFUNC(LoadLevel);
		world["UnloadLevel"] = EQSCRIPT_CFUNC(UnloadLevel);

		world["SpoolAllRegions"] = EQSCRIPT_CFUNC(SpoolAllRegions);
		world["SpoolRegions"] = EQSCRIPT_CFUNC(SpoolRegions);
		world["IsLevelLoaded"] = EQSCRIPT_CFUNC(IsLevelLoaded);

		world["GetModelByIndex"] = EQSCRIPT_CFUNC(GetModelByIndex);
		world["GetModelByName"] = EQSCRIPT_CFUNC(GetModelByName);

		world["MapHeight"] = EQSCRIPT_CFUNC(MapHeight);
		world["GetSurfaceIndex"] = EQSCRIPT_CFUNC(GetSurfaceIndex);

		world["QueryCollision"] = EQSCRIPT_CFUNC(+[](const VECTOR_NOPAD& queryPos, int queryDist, esl::LuaFunctionRef& func) {
			QueryCollision(queryPos, queryDist, [&func](const BUILDING_BOX& box, CELL_OBJECT* co) {
				using QueryFuncCall = esl::runtime::FunctionCall<bool, const BUILDING_BOX&, CELL_OBJECT*>;
				auto result = QueryFuncCall::Invoke(func, box, co);
				if (!LUA_CHECK_CALL(result, "QueryCollision func"))
					return false;
				return *result;
				});
			});

		world["PushCellObject"] = EQSCRIPT_CFUNC(PushCellObject);
		world["PurgeCellObjects"] = EQSCRIPT_CFUNC(PurgeCellObjects);
		world["AddDrawable"] = EQSCRIPT_CFUNC(AddDrawable);

		world["CreateCellList"] = EQSCRIPT_CFUNC(CreateCellList);
		world["RemoveCellList"] = EQSCRIPT_CFUNC(RemoveCellList);

		world["EndStep"] = EQSCRIPT_CFUNC(EndStep);
		world["ResetStep"] = EQSCRIPT_CFUNC(ResetStep);
		world["StepCount"] = EQSCRIPT_CFUNC(+[]() {return StepCount; });
	}

	{
		esl::LuaTable modelFlags2Tbl = state.CreateTable();
		state.SetGlobal("ModelFlags2", modelFlags2Tbl);
		modelFlags2Tbl["Median"] = MODEL_FLAG_MEDIAN;
		modelFlags2Tbl["Junction"] = MODEL_FLAG_JUNC;
		modelFlags2Tbl["Alley"] = MODEL_FLAG_ALLEY;
		modelFlags2Tbl["Indoors"] = MODEL_FLAG_INDOORS;
		modelFlags2Tbl["Chair"] = MODEL_FLAG_CHAIR;
		modelFlags2Tbl["Barrier"] = MODEL_FLAG_BARRIER;
		modelFlags2Tbl["Smashable"] = MODEL_FLAG_SMASHABLE;
		modelFlags2Tbl["Lamp"] = MODEL_FLAG_LAMP;
		modelFlags2Tbl["Tree"] = MODEL_FLAG_TREE;
		modelFlags2Tbl["Grass"] = MODEL_FLAG_GRASS;
		modelFlags2Tbl["Path"] = MODEL_FLAG_PATH;
	}

	{
		esl::LuaTable modelShapeFlagsTbl = state.CreateTable();
		state.SetGlobal("ModelShapeFlags", modelShapeFlagsTbl);
		modelShapeFlagsTbl["LitPoly"] = SHAPE_FLAG_LITPOLY;
		modelShapeFlagsTbl["BSPData"] = SHAPE_FLAG_BSPDATA;
		modelShapeFlagsTbl["Trans"] = SHAPE_FLAG_TRANS;
		modelShapeFlagsTbl["NoCollide"] = SHAPE_FLAG_NOCOLLIDE;
		modelShapeFlagsTbl["Water"] = SHAPE_FLAG_WATER;
		modelShapeFlagsTbl["Ambient2"] = SHAPE_FLAG_AMBIENT2;
		modelShapeFlagsTbl["Ambient1"] = SHAPE_FLAG_AMBIENT1;
		modelShapeFlagsTbl["Tile"] = SHAPE_FLAG_TILE;
		modelShapeFlagsTbl["Shadow"] = SHAPE_FLAG_SHADOW;
		modelShapeFlagsTbl["Alpha"] = SHAPE_FLAG_ALPHA;
		modelShapeFlagsTbl["Road"] = SHAPE_FLAG_ROAD;
		modelShapeFlagsTbl["Sprite"] = SHAPE_FLAG_SPRITE;
	}

	engineTbl["LevelRenderProps"] = &CRender_Level::RenderProps;
}

//-----------------------------------------------------------------

void CWorld::InitObjectMatrix()
{
	for (int i = 0; i < 64; i++)
	{
		MATRIX& m = g_objectMatrixFixed[i];

		InitMatrix(m);
		RotMatrixY(i * 64, &m);

		const float cellRotationRad = -i / 64.0f * M_PI_F * 2.0f;
		g_objectMatrix[i] = rotateY4(cellRotationRad);
	}
}


//-----------------------------------------------------------------

static ITexturePtr g_hwTexturePages[128][16];
ushort g_hwTexturePagesDirty[128] = { 0xFFFF };
static ITexturePtr GetLevelDefaultTexture()
{
	// FIXME: use white texture?
	return g_matSystem->GetErrorCheckerboardTexture();
}

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
	uint* color_data = (uint*)PPAlloc(imgSize);

	memset(color_data, 0, imgSize);

	const int numDetails = tpage->GetDetailCount();
	for (int i = 0; i < numDetails; i++)
		tpage->ConvertIndexedTextureToRGBA(color_data, i, &bitmap.clut[i], false, false);

	if (g_hwTexturePagesDirty[tpageId] & 1)
	{
		ITexturePtr& texture = g_hwTexturePages[tpageId][0];
		// create new or update
		//if (texture == GetLevelDefaultTexture())
		//	texture = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
		//else
		//	GR_UpdateRGBATexture(texture, TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
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
			ITexturePtr& texture = g_hwTexturePages[tpageId][pal];
			//if (texture == GetLevelDefaultTexture())
			//	texture = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
			//else
			//	GR_UpdateRGBATexture(texture, TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
		}
	}

	g_hwTexturePagesDirty[tpageId] = 0;
	
	// no longer need in RGBA data
	PPFree(color_data);
}

void CWorld::FreeHWTexturePage(CTexturePage* tpage)
{
	int tpageId = tpage->GetId();

	for (int pal = 0; pal < 16; pal++)
	{
		//if(g_hwTexturePages[tpageId][pal] != GetLevelDefaultTexture())
		//	GR_DestroyTexture(g_hwTexturePages[tpageId][pal]);

		g_hwTexturePages[tpageId][pal] = GetLevelDefaultTexture();
		g_hwTexturePagesDirty[tpageId] = 0xffff;
	}
}

// returns hardware texture
ITexture* CWorld::GetHWTexture(int tpage, int pal)
{
	if (tpage < 0 || tpage >= 128 ||
		pal < 0 || pal >= 16)
		return GetLevelDefaultTexture();

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
			g_hwTexturePages[i][j] = GetLevelDefaultTexture();
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

static IFilePtr g_levFile = nullptr;

//-------------------------------------------------------
// Perorms level loading and renderer data initialization
//-------------------------------------------------------
bool CWorld::LoadLevel(const char* fileName)
{
	g_levFile = g_fileSystem->Open(fileName, FS_OPEN_READ);
	if (!g_levFile)
	{
		MsgError("Cannot open '%s'\n", fileName);
		return false;
	}

	UnloadLevel();

	// seek to begin
	MsgWarning("-----------\nLoading LEV file '%s'\n", fileName);

	ELevelFormat levFormat = CDriverLevelLoader::DetectLevelFormat(g_levFile);

	// create map accordingly
	if (levFormat >= LEV_FORMAT_DRIVER2_ALPHA16 || levFormat == LEV_FORMAT_AUTODETECT)
		g_levMap = PPNew CDriver2LevelMap();
	else
		g_levMap = PPNew CDriver1LevelMap();

	CDriverLevelLoader loader;
	loader.Initialize(g_levInfo, &g_levTextures, &g_levModels, g_levMap);

	bool result = loader.Load(g_levFile);

	CRender_Cars::Init();
	CRender_Level::Init();
	ResetStep();

	return result;
}

//-------------------------------------------------------
// Frees all data
//-------------------------------------------------------
void CWorld::UnloadLevel()
{
	CRender_Cars::Terminate();
	CRender_Level::Terminate();
	CManager_Players::RemoveAllPlayers();
	if (g_levMap)
	{
		CellLists.clear();

		MsgWarning("Freeing level data ...\n");
		g_levMap->FreeAll();

		g_levTextures.FreeAll();
		g_levModels.FreeAll();

		SAFE_DELETE(g_levMap);

		g_levFile = nullptr;
	}
}

//-------------------------------------------------------
// Render level viewer
//-------------------------------------------------------
void CWorld::RenderLevelView(const CViewParams& view)
{
	Volume frustumVolume;

	// setup standard camera
	CRenderModel::SetupModelShader();
	CCamera::SetupViewAndMatrices(view, frustumVolume);

	//GR_SetDepthMode(1, 1);
	//GR_SetCullMode(CULL_FRONT);

	// reset lighting
	CRenderModel::SetupLightingProperties();

	const bool driver2Map = g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;
	
	CRender_Level::DrawMap(view.GetOrigin(), view.GetAngles().y, frustumVolume);
	//GR_SetCullMode(CULL_FRONT);
	{
		const VECTOR_NOPAD cameraPosition = ToFixedVector(view.GetOrigin());
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

			for (CELL_OBJECT& obj : *it)
				CRender_Level::DrawCellObject(obj, view.GetOrigin(), view.GetAngles().y, frustumVolume, driver2Map);
		}
	}

	for (DRAWABLE& drawable : Drawables)
		CRender_Level::DrawObject(drawable, view.GetOrigin(), frustumVolume, driver2Map);

	Drawables.clear();
}

int CWorld::SpoolRegions(const VECTOR_NOPAD& position, int radius)
{
	if (!IsLevelLoaded())
		return 0;

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = g_levFile;
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

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = g_levFile;
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

	static Array<CELL_OBJECT*> collisionObjects{ PP_SL };
	static Array<const ModelRef_t*> collisionObjectModels{ PP_SL };
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
				for (CELL_OBJECT& obj : *cellObjList)
					checkAndAddCellObj(&obj);
			}

			cell.z++;
		}

		cell.x++;
	}

	// check collisions
	for (int i = 0; i < collisionObjects.numElem(); i++)
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
		Array<CELL_OBJECT> objs{ PP_SL };
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
	ASSERT(CellLists.contains(list) == false);

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
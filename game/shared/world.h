#ifndef WORLD_H
#define WORLD_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

#include "math/psx_math_types.h"
#include "camera.h"
#include "routines/d2_types.h"

class CTexturePage;
struct TexDetailInfo_t;
struct ModelRef_t;

struct BUILDING_BOX
{
	VECTOR_NOPAD pos;
	int xsize, zsize;
	int theta, height;
	ModelRef_t* modelRef;
};

struct DRAWABLE
{
	Vector3D position;
	Vector3D angles;
	Vector3D scale;
	int model;
};

struct CELL_LIST_DESC
{
	Matrix4x4 pivotMatrix{ identity4() };

	// TODO: multiple instances?
	Matrix4x4 transform{ identity4() };		// this field is recalculated whenever list is dirty
	VECTOR_NOPAD rotation{ 0 };
	VECTOR_NOPAD position{ 0 };
	bool visible{ true };
	bool dirty{ false };
};

// almost like SURFACE and SINFO
struct EVENT_SURFACE
{
	struct POLY
	{
		VECTOR_NOPAD verts[4]{ VECTOR_NOPAD{ 0 } };
		sdPlane surface;		// computed surface
		bool dirty{ false };
	};

	Array<POLY> polys;
};

// TODO: replace std::function with custom impl with allocator support
using CellObjectIterateFn = std::function<bool(int listType, CELL_OBJECT* co)>;
using BoxCollisionFn = std::function<bool(BUILDING_BOX& box, CELL_OBJECT* co, void* object)>;

extern Matrix4x4 g_objectMatrix[64];
extern MATRIX g_objectMatrixFixed[64];

class CWorld
{
	friend class CRender_Level;
public:

	// shared object and rendering stuff
	static void				InitObjectMatrix();

	//------------------------------------------
	// 
	// texture management
	static void				InitHWTexturePage(CTexturePage* tpage);
	static void				FreeHWTexturePage(CTexturePage* tpage);

	static TextureID		GetHWTexture(int tpage, int pal);
	static CTexturePage*	GetTPage(int tpage);
	static TexDetailInfo_t*	FindTextureDetail(const char* name);

	static void				InitHWTextures();
	static void				InitHWModels();
	// for animated textures
	static void				StepTextureDetailPalette(const TexDetailInfo_t* detail, int start, int stop);

	static void				RenderLevelView(const CameraViewParams& view);

	//------------------------------------------
	// level management

	static bool				LoadLevel(const char* fileName);
	static void				UnloadLevel();

	static bool				IsLevelLoaded();

	static void				SpoolAllRegions();
	static int				SpoolRegions(const VECTOR_NOPAD& position, int radius);

	static ModelRef_t*		GetModelByIndex(int modelIndex);
	static ModelRef_t*		GetModelByName(const char* name);

	static int				MapHeight(const VECTOR_NOPAD& position);
	static void				FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane);

	//------------------------------------------
	// objects and collision
	static void				QueryCollision(const VECTOR_NOPAD& queryPos, int queryDist, const BoxCollisionFn& func, void* object);

	// push event cell object
	// any collision checks afterwards will have an effect with it
	static int				PushCellObject(const CELL_OBJECT& object);

	// purges list of recently added objects by PushCellObject
	static void				PurgeCellObjects();

	// adds a drawable object for one draw frame
	static void				AddDrawable(const DRAWABLE& drawable);

	// cell lists
	static CELL_LIST_DESC&	CreateCellList(int list);
	static void				RemoveCellList(int list);

	// event surfaces
	static EVENT_SURFACE&	CreateEventSurface(int surface);
	static void				RemoveEventSurface(int surface);

	// iterates through all cell objects at specific cell on map
	static void				ForEachCellObjectAt(const XZPAIR& cell, const CellObjectIterateFn& func, struct CELL_ITERATOR_CACHE* iteratorCache = nullptr);

	//------------------------------------------
	// game steps
	static void				EndStep();
	static void				ResetStep();
	static int				StepCount;		// aka CameraCnt


	//------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:

	static Map<int, Array<CELL_OBJECT>>	CellObjects;
	static Array<DRAWABLE>				Drawables;
	static Map<int, CELL_LIST_DESC>		CellLists;
	static Map<int, EVENT_SURFACE>		EventSurfaces;
};


#endif // WORLD_H
#ifndef WORLD_H
#define WORLD_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

#include "math/psx_math_types.h"
#include "camera.h"
#include "routines/d2_types.h"

class CTexturePage;
struct ModelRef_t;

struct BUILDING_BOX
{
	VECTOR_NOPAD pos;
	int xsize, zsize;
	int theta, height;
	struct MODEL* model;
};

typedef bool (*CellObjectIterateFn)(int listType, CELL_OBJECT* co);
typedef void (*BoxCollisionFn)(BUILDING_BOX& box, CELL_OBJECT* co, void* object);

extern Matrix4x4 g_objectMatrix[64];
extern MATRIX g_objectMatrixFixed[64];

class CWorld
{
public:

	// shared object and rendering stuff
	static void				InitObjectMatrix();

	//------------------------------------------
	// 
	// texture management
	static void				InitHWTexturePage(CTexturePage* tpage);
	static void				FreeHWTexturePage(CTexturePage* tpage);

	static TextureID		GetHWTexture(int tpage, int pal);

	static void				InitHWTextures();
	static void				InitHWModels();

	static void				RenderLevelView(const CameraViewParams& view);

	//------------------------------------------
	// level management

	static bool				LoadLevel(const char* fileName);
	static void				UnloadLevel();

	static bool				IsLevelLoaded();

	static void				SpoolAllRegions();
	static void				SpoolRegions(const VECTOR_NOPAD& position, int radius);

	static ModelRef_t*		GetModelByIndex(int modelIndex);
	static ModelRef_t*		GetModelByName(const char* name);

	static int				MapHeight(const VECTOR_NOPAD& position);
	static int				FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outNormal, VECTOR_NOPAD& outPoint, sdPlane& outPlane);

	//------------------------------------------
	// objects and collision
	static void				QueryCollision(const VECTOR_NOPAD& queryPos, int queryDist, BoxCollisionFn func, void* object);

	// push event cell object
	// any collision checks afterwards will have an effect with it
	static int				PushCellObject(const CELL_OBJECT& object);

	// purges list of recently added objects by PushCellObject
	static void				PurgeCellObjects();

	// iterates through all cell objects at specific cell on map
	static void				ForEachCellObjectAt(const XZPAIR& cell, CellObjectIterateFn func);

	//------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:

	static Array<CELL_OBJECT>		m_CellObjects;
};

int ViewerMain();


#endif // WORLD_H
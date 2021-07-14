#ifndef WORLD_H
#define WORLD_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

#include "math/psx_math_types.h"
#include "camera.h"
#include "routines/d2_types.h"

class CTexturePage;
struct ModelRef_t;

class CWorld
{
public:

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

	//------------------------------------------
	// objects and collision

	// push event cell object
	// any collision checks afterwards will have an effect with it
	static int				PushCellObject(const CELL_OBJECT& object);

	// purges list of recently added objects by PushCellObject
	static void				PurgeCellObjects();



	//------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:

	static Array<CELL_OBJECT>		m_CellObjects;
};

int ViewerMain();


#endif // WORLD_H
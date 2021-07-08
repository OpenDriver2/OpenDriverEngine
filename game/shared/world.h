#ifndef WORLD_H
#define WORLD_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

#include "math/psx_math_types.h"
#include "camera.h"

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

	static bool				LoadLevel(const char* fileName);
	static void				UnloadLevel();

	static void				RenderLevelView(const CameraViewParams& view);

	//------------------------------------------

	static bool				IsLevelLoaded();

	static void				SpoolAllAreaDatas();

	static ModelRef_t*		GetModelByIndex(int modelIndex);
	static ModelRef_t*		GetModelByName(const char* name);

	static int				MapHeight(const VECTOR_NOPAD& position);

	//------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:


};

int ViewerMain();


#endif // WORLD_H
#ifndef WORLD_H
#define WORLD_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

class CWorld
{
public:

	// texture management
	static void				InitHWTexturePage(CTexturePage* tpage);
	static void				FreeHWTexturePage(CTexturePage* tpage);

	static TextureID		GetHWTexture(int tpage, int pal);

	static void				InitHWTextures();
	static void				InitHWModels();

	static bool				LoadLevelFile(const char* fileName);
	static void				FreeLevelData();

	static void				RenderLevelView();
	static void				SpoolAllAreaDatas();

	static void				Lua_Init(sol::state& lua);

protected:


};

// extern some vars
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

int ViewerMain();


#endif // WORLD_H
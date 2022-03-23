#ifndef RENDER_SKY_H
#define RENDER_SKY_H

#include "game/shared/camera.h"

class CSky
{
public:
	static void				Init();

	// Initialize sky texture and UVs
	static bool				Load(const char* filename, int skyNumber);

	// Destroy sky and UVs
	static void				Unload();

	// Renders a sky
	static void				Draw(const CameraViewParams& view);

	static void				Lua_Init(sol::state& lua);

	static ColorRGB			Color;
};

#endif // RENDER_SKY_H
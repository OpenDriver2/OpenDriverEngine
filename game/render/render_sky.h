#ifndef RENDER_SKY_H
#define RENDER_SKY_H

class CSky
{
public:
	// Initialize sky texture and UVs
	static bool				Init(const char* filename, int skyNumber);

	// Destroy sky and UVs
	static void				Destroy();

	// Renders a sky
	static void				Draw();

	static void				Lua_Init(sol::state& lua);
};

#endif // RENDER_SKY_H
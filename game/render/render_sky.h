#pragma once
#include "scripting/esl.h"

class CViewParams;

class CSky
{
public:
	static void				Init();

	// Initialize sky texture and UVs
	static bool				Load(const char* filename, int skyNumber);

	// Destroy sky and UVs
	static void				Unload();

	// Renders a sky
	static void				Draw(const CViewParams& view);

	static void				Lua_Init(const esl::ScriptState& state);

	static ColorRGB			Color;
};

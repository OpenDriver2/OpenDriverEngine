#pragma once
#include "scripting/esl.h"

union SDL_Event;

class CInput
{
public:
	static void UpdateEvents(SDL_Event& event, const esl::LuaTable& engineHostTable, bool imguiFocused);

	static void Lua_Init(const esl::ScriptState& state);
};

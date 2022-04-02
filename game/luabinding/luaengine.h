#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include <sol/forward.hpp>

void Engine_Lua_Init(sol::state& lua);
void Engine_Lua_PrintStackTrace(sol::state& lua);

#endif // LUA_ENGINE_H


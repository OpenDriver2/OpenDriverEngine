#include "game/pch.h"
#include "lua_init.h"
#include "luamath.h"
#include "luaengine.h"
#include "game/shared/input.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sol_ImGui/sol_imgui.h>

void LuaInit(sol::state& lua)
{
	lua.open_libraries(sol::lib::base);
	lua.open_libraries(sol::lib::debug);
	lua.open_libraries(sol::lib::math);
	lua.open_libraries(sol::lib::bit32);
	lua.open_libraries(sol::lib::string);
	lua.open_libraries(sol::lib::table);

	sol_ImGui::InitBindings(lua);

	MAKE_PROPERTY_REF(lua, bool);
	MAKE_PROPERTY_REF(lua, int);
	MAKE_PROPERTY_REF(lua, uint);
	MAKE_PROPERTY_REF(lua, short);
	MAKE_PROPERTY_REF(lua, ushort);
	MAKE_PROPERTY_REF(lua, float);
	MAKE_PROPERTY_REF(lua, double);

	// replace default print with Msg
	lua["print"] = lua["Msg"] = [](sol::variadic_args va) {
		for (auto v : va) {
			Msg("%s", luaL_tolstring(v.lua_state(), v.stack_index(), nullptr));
		}
		Msg("\n");
	};

	// as well as expose all dev messages
	lua["MsgWarning"] = [](sol::variadic_args va) {
		for (auto v : va) {
			MsgWarning("%s", luaL_tolstring(v.lua_state(), v.stack_index(), nullptr));
		}
		MsgWarning("\n");
	};

	lua["MsgError"] = [](sol::variadic_args va) {
		for (auto v : va) {
			MsgError("%s", luaL_tolstring(v.lua_state(), v.stack_index(), nullptr));
		}
		MsgError("\n");
	};

	lua["MsgInfo"] = [](sol::variadic_args va) {
		for (auto v : va) {
			MsgInfo("%s", luaL_tolstring(v.lua_state(), v.stack_index(), nullptr));
		}
		MsgInfo("\n");
	};

	lua["MsgAccept"] = [](sol::variadic_args va) {
		for (auto v : va) {
			MsgAccept("%s", luaL_tolstring(v.lua_state(), v.stack_index(), nullptr));
		}
		MsgAccept("\n");
	};

	lua["DevMsg"] = &DevMsg;

	lua["Spew"] = lua.create_table_with(
		"Norm", SPEW_NORM,
		"Info", SPEW_INFO,
		"Warning", SPEW_WARNING,
		"Error", SPEW_ERROR,
		"Success", SPEW_SUCCESS
	);


	//-----------------------------------
	// MODULES
	Engine_Lua_Init(lua);
	CInput::Lua_Init(lua);
	CManager_Players::Lua_Init(lua);
	CManager_Cars::Lua_Init(lua);
	CWorld::Lua_Init(lua);
	CSky::Lua_Init(lua);
	CCamera::Lua_Init(lua);

	// this should come last always
	CLuaDocumentation::Lua_Init(lua);
}
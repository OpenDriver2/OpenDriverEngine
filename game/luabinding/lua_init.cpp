#include "core/core_common.h"

#include "lua_init.h"
#include "luamath.h"
#include "luaengine.h"
#include "luarefvalue.h"
#include "luadocs.h"

#include "game/shared/players.h"
#include "game/shared/manager_cars.h"
#include "game/shared/world.h"
#include "game/render/render_sky.h"
#include "game/shared/camera.h"
#include "game/shared/replay.h"

EQSCRIPT_BIND_PROPERTY_REF(bool);
EQSCRIPT_BIND_PROPERTY_REF(int);
EQSCRIPT_BIND_PROPERTY_REF(uint);
EQSCRIPT_BIND_PROPERTY_REF(short);
EQSCRIPT_BIND_PROPERTY_REF(ushort);
EQSCRIPT_BIND_PROPERTY_REF(float);
EQSCRIPT_BIND_PROPERTY_REF(double);

bool OpenDriverLuaInit(const esl::ScriptState& state)
{
	MAKE_PROPERTY_REF(bool);
	MAKE_PROPERTY_REF(int);
	MAKE_PROPERTY_REF(uint);
	MAKE_PROPERTY_REF(short);
	MAKE_PROPERTY_REF(ushort);
	MAKE_PROPERTY_REF(float);
	MAKE_PROPERTY_REF(double);

	//-----------------------------------
	// MODULES
	Engine_Lua_Init(state);
	CManager_Players::Lua_Init(state);
	CManager_Cars::Lua_Init(state);
	CWorld::Lua_Init(state);
	CSky::Lua_Init(state);
	CCamera::Lua_Init(state);
	CReplayData::Lua_Init(state);

	// this should come last always
	CLuaDocumentation::Lua_Init(state);

	return true;
}
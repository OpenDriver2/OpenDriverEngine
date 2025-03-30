#include "core/core_common.h"
#include "core/IFileSystem.h"

#include "lua_init.h"
#include "luamath.h"
#include "luaengine.h"
#include "luadocs.h"

#include "game/shared/players.h"
#include "game/shared/manager_cars.h"
#include "game/shared/world.h"
#include "game/render/render_sky.h"
#include "game/shared/camera.h"
#include "game/shared/replay.h"

#define MAIN_SCRIPT_FILE "scripts/lua/_init.lua"

static bool eslLoadMainOpenDriverScript(lua_State* L)
{
	esl::runtime::StackGuard g(L);
	esl::ScriptState state(L);
	IFilePtr mainScriptFile = g_fileSystem->Open(MAIN_SCRIPT_FILE);
	if (!mainScriptFile)
	{
		esl::runtime::ResetErrorValue(L);
		lua_pushfstring(L, "Main script file '%s' not found", MAIN_SCRIPT_FILE);
		esl::runtime::SetLuaErrorFromTopOfStack(L);
		return false;
	}

	return state.RunFileBuffer(mainScriptFile, mainScriptFile->GetName());
}

bool eslSysOpenDriverInit(const esl::ScriptState& state)
{
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

	if (!eslLoadMainOpenDriverScript(state))
		return false;

	return true;
}
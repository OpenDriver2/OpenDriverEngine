#include "core/core_common.h"
#include "luamath.h"
#include "luaengine.h"
#include "sys/scripting/sys_esl.h"

bool Engine_Lua_Init(const esl::ScriptState& state)
{
	ESL_SYS_INIT(Math_Lua_Init);
	return true;
}
#pragma once
#include "sys/scripting/sys_esl.h"

#include "math/psx_math_types.h"

EQSCRIPT_BIND_TYPE_NO_PARENT(VECTOR_NOPAD, "VECTOR", BY_VALUE)
EQSCRIPT_BIND_TYPE_NO_PARENT(SVECTOR, "SVECTOR", BY_VALUE)

bool Math_Lua_Init(const esl::ScriptState& state);


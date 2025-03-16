#pragma once
#include "scripting/esl.h"
#include "scripting/esl_luaref.h"
#include "scripting/esl_bind.h"
#include "math/psx_math_types.h"
#include "luarefvalue.h"

EQSCRIPT_BIND_TYPE_NO_PARENT(VECTOR_NOPAD, "VECTOR", BY_VALUE)
EQSCRIPT_BIND_TYPE_NO_PARENT(SVECTOR, "SVECTOR", BY_VALUE)

EQSCRIPT_BIND_PROPERTY_REF(VECTOR_NOPAD)
EQSCRIPT_BIND_PROPERTY_REF(SVECTOR)

bool Math_Lua_Init(const esl::ScriptState& state);


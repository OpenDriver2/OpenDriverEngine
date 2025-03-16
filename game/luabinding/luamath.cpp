#include "core/core_common.h"

#include "scripting/esl_luaref.h"
#include "scripting/esl_bind.h"

#include "luamath.h"
#include "luadocs.h"
#include "math/psx_matrix.h"
#include "math/convert.h"
#include "math/isin.h"
#include "math/ratan2.h"
#include "math/squareroot0.h"
//
// Vector3D
//
EQSCRIPT_TYPE_BEGIN(VECTOR_NOPAD)
	EQSCRIPT_CLONE_FUNC()
	EQSCRIPT_BIND_CONSTRUCTOR(int)
	EQSCRIPT_BIND_CONSTRUCTOR(int, int, int)

	EQSCRIPT_BIND_OP(add)
	EQSCRIPT_BIND_OP(sub)
	EQSCRIPT_BIND_OP(mul)
	EQSCRIPT_BIND_OP(div)
	EQSCRIPT_BIND_OP(unm)
	EQSCRIPT_BIND_OP(shl)
	EQSCRIPT_BIND_OP(shr)
	EQSCRIPT_BIND_OP(band)
	EQSCRIPT_BIND_OP(bor)
	EQSCRIPT_BIND_OP(xor)

	EQSCRIPT_BIND_VAR(vx)
	EQSCRIPT_BIND_VAR(vy)
	EQSCRIPT_BIND_VAR(vz)
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(SVECTOR)
	EQSCRIPT_CLONE_FUNC()
	EQSCRIPT_BIND_CONSTRUCTOR(short)
	EQSCRIPT_BIND_CONSTRUCTOR(short, short, short)
	EQSCRIPT_BIND_CONSTRUCTOR(const VECTOR_NOPAD&)

	EQSCRIPT_BIND_VAR(vx)
	EQSCRIPT_BIND_VAR(vy)
	EQSCRIPT_BIND_VAR(vz)
EQSCRIPT_TYPE_END

static Vector3D L_FromFixedVector(const esl::ScriptState& state)
{
	esl::Object<const VECTOR_NOPAD> a(state, 1);
	esl::Object<const SVECTOR> b(state, 1);
	//esl::Object<const SVECTOR_NOPAD> c(state, 1);

	if (a)
		return FromFixedVector(a.Get());
	if (b)
		return FromFixedVector(b.Get());
	//if (c)
	//	return FromFixedVector(c.Get());

	state.ThrowError("FromFixedVector expects VECTOR_NOPAD or SVECTOR or SVECTOR_NOPAD");
	return vec3_zero;
}

bool Math_Lua_Init(const esl::ScriptState& state)
{
	//
	// FIXED MATH
	//
	{
		LUADOC_NAMESPACE("fix");
		esl::LuaTable fix = state.CreateTable();
		state.SetGlobal("fix", fix);

		{
			LUADOC_TYPE("VECTOR", "Three dimensional vector (32 bit)");
			MAKE_PROPERTY_REF(VECTOR_NOPAD);
			state.RegisterClass<VECTOR_NOPAD>();
		}

		{
			LUADOC_TYPE("SVECTOR", "Three dimensional vector (16 bit)");
			MAKE_PROPERTY_REF(SVECTOR);
			state.RegisterClass<SVECTOR>();
		}

		fix.Set("ONE", ONE);
		fix.Set("ONE_BITS", ONE_BITS);
		fix.Set("toRadian", TO_RADIAN);
		fix.Set("toGTEAngle", TO_GTE_ANGLE);
		fix.Set("ToFixed", EQSCRIPT_CFUNC(+[](const float a) { return int(a * ONE_F); }));
		fix.Set("FromFixed", EQSCRIPT_CFUNC(+[](const int a) { return float(a) * ONE_F_RECIP; }));
		fix.Set("DivHalfRound", EQSCRIPT_CFUNC(+[](const int a, const int bits) { return FixDivHalfRound(a, bits); }));
		fix.Set("DIFF_ANGLES_F", EQSCRIPT_CFUNC(+[](const float x, const float y) { return DIFF_ANGLES_F(x, y); }));
		fix.Set("DIFF_ANGLES", EQSCRIPT_CFUNC(+[](const int x, const int y) { return DIFF_ANGLES(x, y); }));

		fix.Set("ToFixedVector", EQSCRIPT_CFUNC(ToFixedVector));
		fix.Set("FromFixedVector", EQSCRIPT_CFUNC(L_FromFixedVector));
	}

	{
		LUADOC_NAMESPACE("gte");

		// extend Lua math
		esl::LuaTable gte = state.CreateTable();
		state.SetGlobal("gte", gte);
		gte.Set("isin", EQSCRIPT_CFUNC(isin));
		gte.Set("icos", EQSCRIPT_CFUNC(icos));
		gte.Set("ratan2", EQSCRIPT_CFUNC(ratan2));
		gte.Set("SquareRoot0", EQSCRIPT_CFUNC(SquareRoot0));
		// gte.Set("MulMatrix0", EQSCRIPT_CFUNC(MulMatrix0));
	}

	return true;
}
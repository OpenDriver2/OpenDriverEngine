#include "core/core_common.h"
#include "sys/scripting/sys_esl_math.h"
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
	//-----------------------------------
	// 3D MATH
	{
#if 0
		LUADOC_NAMESPACE("vec");

		auto& vec = lua["vec"].get_or_create<sol::table>();

		//
		// Vector 2D
		//
		{
			MAKE_PROPERTY_REF(lua, Vector2D);
			LUADOC_TYPE();
			vec.new_usertype<Vector2D>(
				LUADOC_T("vec2", "Two-dimensional vector"),
				sol::call_constructor, sol::factories(
					[](const sol::table& table) {
						return Vector2D(table["x"], table["y"]);
					}),
				sol::call_constructor, sol::constructors<Vector2D(const float&, const float&), Vector2D(const float&)>(),
				LUADOC_P("set"), sol::overload(
					[](Vector2D& self, const float& v) { self.x = self.y = v; },
					[](Vector2D& self, const float& x, const float& y) { self.x = x; self.y = y; },
					[](Vector2D& self, const Vector2D& other) { self = other; },
					[](Vector2D& self, const sol::table& table) { self = Vector2D(table["x"], table["y"]); }
				),
				VEC_OPERATORS(Vector2D, "vec2")
				LUADOC_P("x"), &Vector2D::x,
				LUADOC_P("y"), &Vector2D::y
			);
		}

		//
		// Vector 3D
		//
		{
			MAKE_PROPERTY_REF(lua, Vector3D);
			LUADOC_TYPE();
			vec.new_usertype<Vector3D>(
				LUADOC_T("vec3", "Three-dimensional vector"),
				sol::call_constructor, sol::factories(
					[](const sol::table& table) {
						return Vector3D(table["x"], table["y"], table["z"]);
					}),
				sol::call_constructor, sol::constructors<Vector3D(const float&, const float&, const float&), Vector3D(const float&)>(),
				LUADOC_P("set"), sol::overload(
					[](Vector3D& self, const float& v) { self.x = self.y = self.z  = v; },
					[](Vector3D& self, const float& x, const float& y, const float& z) { self.x = x; self.y = y; self.z = z; },
					[](Vector3D& self, const Vector3D& other) { self = other; },
					[](Vector3D& self, const sol::table& table) { self = Vector3D(table["x"], table["y"], table["z"]); }
				),
				VEC_OPERATORS(Vector3D, "vec3")
				LUADOC_M("cross", "(a: vec3, b: vec3) : vec3"), sol::resolve<Vector3D(const Vector3D&, const Vector3D&)>(cross),
				// members
				LUADOC_P("x"), &Vector3D::x,
				LUADOC_P("y"), &Vector3D::y,
				LUADOC_P("z"), &Vector3D::z
			);
		}

		//
		// Vector 4D
		//
		{
			MAKE_PROPERTY_REF(lua, Vector4D);
			LUADOC_TYPE();
			vec.new_usertype<Vector4D>(
				LUADOC_T("vec4", "Four-dimensional vector"),
				sol::call_constructor, sol::factories(
					[](const sol::table& table) {
						return Vector4D(table["x"], table["y"], table["z"], table["w"]);
					}),
				sol::call_constructor, sol::constructors<Vector4D(const float&), Vector4D(const float&, const float&, const float&, const float&)>(),
				LUADOC_P("set"), sol::overload(
					[](Vector4D& self, const float& v) { self.x = self.y = self.z = self.w = v; },
					[](Vector4D& self, const float& x, const float& y, const float& z, const float& w) { self.x = x; self.y = y; self.z = z; self.w = w; },
					[](Vector4D& self, const Vector4D& other) { self = other; },
					[](Vector4D& self, const sol::table& table) { self = Vector4D(table["x"], table["y"], table["z"], table["w"]); }
				),
				VEC_OPERATORS(Vector4D, "vec4")
				// members
				"x", &Vector4D::x,
				"y", &Vector4D::y,
				"z", &Vector4D::z,
				"w", &Vector4D::w
			);
		}

		//----------------------------------------------------
		// MATRIX TYPES

		//
		// Matrix3x3
		//
		{
			MAKE_PROPERTY_REF(lua, Matrix3x3);
			LUADOC_TYPE();
			vec.new_usertype<Matrix3x3>(
				LUADOC_T("mat3", "3x3 Matrix"),
				sol::call_constructor, sol::factories(
					[](const sol::table& table) {
						return Matrix3x3((Vector3D&)table[1], (Vector3D&)table[2], (Vector3D&)table[3]);
					},
					[]() {return identity3(); }),
				sol::call_constructor, sol::constructors<
						Matrix3x3(const float&, const float&, const float&, 
								  const float&, const float&, const float&, 
								  const float&, const float&, const float&)>(),
				// matrix - matrix ops
				sol::meta_function::addition, sol::resolve<Matrix3x3(const Matrix3x3&, const Matrix3x3&)>(&operator+),
				sol::meta_function::subtraction, sol::resolve<Matrix3x3(const Matrix3x3&, const Matrix3x3&)>(&operator-),
				sol::meta_function::multiplication, sol::resolve<Matrix3x3(const Matrix3x3&, const Matrix3x3&)>(&operator*),
				// negate 
				sol::meta_function::unary_minus, sol::resolve<Matrix3x3(const Matrix3x3&)>(&operator-),
				// inverse matrix
				sol::meta_function::bitwise_not, sol::resolve<Matrix3x3(const Matrix3x3&)>(&operator!),
				// members
				"r1", sol::property([](Matrix3x3& self) {return self.rows[0]; }, [](Matrix3x3& self, const Vector3D& value) {self.rows[0] = value; }),
				"r2", sol::property([](Matrix3x3& self) {return self.rows[1]; }, [](Matrix3x3& self, const Vector3D& value) {self.rows[1] = value; }),
				"r3", sol::property([](Matrix3x3& self) {return self.rows[2]; }, [](Matrix3x3& self, const Vector3D& value) {self.rows[2] = value; }),

				// members - row access
				"m11", sol::property([](Matrix3x3& self) {return self.rows[0][0]; }, [](Matrix3x3& self, const float& value) {self.rows[0][0] = value; }),
				"m12", sol::property([](Matrix3x3& self) {return self.rows[0][1]; }, [](Matrix3x3& self, const float& value) {self.rows[0][1] = value; }),
				"m13", sol::property([](Matrix3x3& self) {return self.rows[0][2]; }, [](Matrix3x3& self, const float& value) {self.rows[0][2] = value; }),
				"m21", sol::property([](Matrix3x3& self) {return self.rows[1][0]; }, [](Matrix3x3& self, const float& value) {self.rows[1][0] = value; }),
				"m22", sol::property([](Matrix3x3& self) {return self.rows[1][1]; }, [](Matrix3x3& self, const float& value) {self.rows[1][1] = value; }),
				"m23", sol::property([](Matrix3x3& self) {return self.rows[1][2]; }, [](Matrix3x3& self, const float& value) {self.rows[1][2] = value; }),
				"m31", sol::property([](Matrix3x3& self) {return self.rows[2][0]; }, [](Matrix3x3& self, const float& value) {self.rows[2][0] = value; }),
				"m32", sol::property([](Matrix3x3& self) {return self.rows[2][1]; }, [](Matrix3x3& self, const float& value) {self.rows[2][1] = value; }),
				"m33", sol::property([](Matrix3x3& self) {return self.rows[2][2]; }, [](Matrix3x3& self, const float& value) {self.rows[2][2] = value; }),

				// operations
				LUADOC_M("transformVec", "(m: mat3, v: vec3) : vec3 - Transforms input vector by the matrix"), 
				[](Matrix3x3& self, const Vector3D& vec) {return transform3(vec, self); },

				LUADOC_M("transformVecInv", "(m: mat3, v: vec3) : vec3 - Transforms input vector by the matrix"),
				[](Matrix3x3& self, const Vector3D& vec) {return transform3Inv(vec, self); },

				LUADOC_M("transposed", "(m: mat3) : mat3 - Returns transposed matrix"), 
				[](Matrix3x3& self) { return transpose(self); },

				LUADOC_M("eulersXYZ", "(m: mat3) : vec3 - Returns euler angles in specified rotation order"), 
				[](Matrix3x3& self) { return EulerMatrixXYZ(self); },

				LUADOC_M("eulersZXY", "(m: mat3) : vec3 - Returns euler angles in specified rotation order"),
				[](Matrix3x3& self) { return EulerMatrixZXY(self); },

				// common matrix generators
				LUADOC_M("identity", "(void) : mat3 - Returns new identity matrix"), []() {return identity3(); },

				LUADOC_M("rotationX", "(x: float) : mat3 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateX3(val); },

				LUADOC_M("rotationY", "(y: float) : mat3 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateY3(val); },

				LUADOC_M("rotationZ", "(z: float) : mat3 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateZ3(val); },

				LUADOC_M("rotationXYZ", "(x: float, y: float, z: float) : mat3 - Makes rotation matrix around specified axes"),
				[](const Vector3D& val) {return rotateXYZ3(val.x, val.y, val.z); },

				LUADOC_M("rotationZXY", "(x: float, y: float, z: float) : mat3 - Makes rotation matrix around specified axes"),
				[](const Vector3D& val) {return rotateZXY3(val.x, val.y, val.z); }
			);
			LUADOC_P("r<n>", "access matrix row by number as vec3");
			LUADOC_P("m<row><column>", "access matrix value by row - column");
		}

		//
		// Matrix4x4
		//
		{
			MAKE_PROPERTY_REF(lua, Matrix4x4);
			LUADOC_TYPE();
			vec.new_usertype<Matrix4x4>(
				LUADOC_T("mat4", "4x4 Matrix"),
				sol::call_constructor, sol::factories(
					[](const sol::table& table) {
						return Matrix4x4((Vector4D&)table[1], (Vector4D&)table[2], (Vector4D&)table[3], (Vector4D&)table[4]);
					},
					[]() {return identity4(); }),
				sol::call_constructor, sol::constructors<
						Matrix4x4(const float&, const float&, const float&, const float&,
								  const float&, const float&, const float&, const float&, 
								  const float&, const float&, const float&, const float&,
								  const float&, const float&, const float&, const float&)>(),
				// matrix - matrix ops
				sol::meta_function::addition, sol::resolve<Matrix4x4(const Matrix4x4&, const Matrix4x4&)>(&operator+),
				sol::meta_function::subtraction, sol::resolve<Matrix4x4(const Matrix4x4&, const Matrix4x4&)>(&operator-),
				sol::meta_function::multiplication, sol::resolve<Matrix4x4(const Matrix4x4&, const Matrix4x4&)>(&operator*),
				// negate 
				sol::meta_function::unary_minus, sol::resolve<Matrix4x4(const Matrix4x4&)>(&operator-),
				// inverse matrix
				sol::meta_function::bitwise_not, sol::resolve<Matrix4x4(const Matrix4x4&)>(&operator!),
				// members
				"r1", sol::property([](Matrix4x4& self) {return self.rows[0]; }, [](Matrix4x4& self, const Vector4D& value) {self.rows[0] = value; }),
				"r2", sol::property([](Matrix4x4& self) {return self.rows[1]; }, [](Matrix4x4& self, const Vector4D& value) {self.rows[1] = value; }),
				"r3", sol::property([](Matrix4x4& self) {return self.rows[2]; }, [](Matrix4x4& self, const Vector4D& value) {self.rows[2] = value; }),
				"r4", sol::property([](Matrix4x4& self) {return self.rows[3]; }, [](Matrix4x4& self, const Vector4D& value) {self.rows[3] = value; }),

				// members - row access
				"m11", sol::property([](Matrix4x4& self) {return self.rows[0][0]; }, [](Matrix4x4& self, const float& value) {self.rows[0][0] = value; }),
				"m12", sol::property([](Matrix4x4& self) {return self.rows[0][1]; }, [](Matrix4x4& self, const float& value) {self.rows[0][1] = value; }),
				"m13", sol::property([](Matrix4x4& self) {return self.rows[0][2]; }, [](Matrix4x4& self, const float& value) {self.rows[0][2] = value; }),
				"m14", sol::property([](Matrix4x4& self) {return self.rows[0][3]; }, [](Matrix4x4& self, const float& value) {self.rows[0][3] = value; }),
				"m21", sol::property([](Matrix4x4& self) {return self.rows[1][0]; }, [](Matrix4x4& self, const float& value) {self.rows[1][0] = value; }),
				"m22", sol::property([](Matrix4x4& self) {return self.rows[1][1]; }, [](Matrix4x4& self, const float& value) {self.rows[1][1] = value; }),
				"m23", sol::property([](Matrix4x4& self) {return self.rows[1][2]; }, [](Matrix4x4& self, const float& value) {self.rows[1][2] = value; }),
				"m24", sol::property([](Matrix4x4& self) {return self.rows[1][3]; }, [](Matrix4x4& self, const float& value) {self.rows[1][3] = value; }),
				"m31", sol::property([](Matrix4x4& self) {return self.rows[2][0]; }, [](Matrix4x4& self, const float& value) {self.rows[2][0] = value; }),
				"m32", sol::property([](Matrix4x4& self) {return self.rows[2][1]; }, [](Matrix4x4& self, const float& value) {self.rows[2][1] = value; }),
				"m33", sol::property([](Matrix4x4& self) {return self.rows[2][2]; }, [](Matrix4x4& self, const float& value) {self.rows[2][2] = value; }),
				"m34", sol::property([](Matrix4x4& self) {return self.rows[2][3]; }, [](Matrix4x4& self, const float& value) {self.rows[2][3] = value; }),
				"m41", sol::property([](Matrix4x4& self) {return self.rows[3][0]; }, [](Matrix4x4& self, const float& value) {self.rows[3][0] = value; }),
				"m42", sol::property([](Matrix4x4& self) {return self.rows[3][1]; }, [](Matrix4x4& self, const float& value) {self.rows[3][1] = value; }),
				"m43", sol::property([](Matrix4x4& self) {return self.rows[3][2]; }, [](Matrix4x4& self, const float& value) {self.rows[3][2] = value; }),
				"m44", sol::property([](Matrix4x4& self) {return self.rows[3][3]; }, [](Matrix4x4& self, const float& value) {self.rows[3][3] = value; }),

				// getters
				LUADOC_M("getTranslationComponent", "(void) : vec3"),
				&Matrix4x4::getTranslationComponent,

				LUADOC_M("getRotationComponent", "(void) : mat3"),
				&Matrix4x4::getRotationComponent,

				LUADOC_M("getTranslationComponentTransposed", "(void) : vec3"),
				&Matrix4x4::getTranslationComponentTransposed,

				LUADOC_M("getRotationComponentTransposed", "(void) : mat3"),
				&Matrix4x4::getRotationComponentTransposed,

				LUADOC_M("setTranslation", "(translate: vec3)"),
				&Matrix4x4::setTranslation,

				LUADOC_M("setTranslationTransposed", "(translate: vec3)"),
				&Matrix4x4::setTranslationTransposed,

				// operations
				LUADOC_M("transformVec", "(m: mat4, v: vec3) : vec3 - Transforms input vector by the matrix"), 
				[](Matrix4x4& self, const Vector3D& vec) {return transform4(vec, self); },

				LUADOC_M("transformVecInv", "(m: mat4, v: vec3) : vec3 - Transforms input vector by the matrix"),
				[](Matrix4x4& self, const Vector3D& vec) {return transform4Inv(vec, self); },

				LUADOC_M("transposed", "(m: mat4) : mat4 - Returns transposed matrix"), 
				[](Matrix4x4& self) { return transpose(self); },

				LUADOC_M("eulersXYZ", "(m: mat4) : vec3 - Returns euler angles in specified rotation order"), 
				[](Matrix4x4& self) { return EulerMatrixXYZ(self.getRotationComponent()); },

				LUADOC_M("eulersZXY", "(m: mat4) : vec3 - Returns euler angles in specified rotation order"),
				[](Matrix4x4& self) { return EulerMatrixZXY(self.getRotationComponent()); },

				// common matrix generators
				LUADOC_M("identity", "(void) : mat4 - Returns new identity matrix"), []() {return identity4(); },

				LUADOC_M("rotationX", "(x: float) : mat4 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateX4(val); },

				LUADOC_M("rotationY", "(y: float) : mat4 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateY4(val); },

				LUADOC_M("rotationZ", "(z: float) : mat4 - Makes rotation matrix around specified axis"),
				[](float val) {return rotateZ4(val); },

				LUADOC_M("translate", "(translation: vec3) : mat4 - Makes translation matrix"),
				[](const Vector3D& val) {return translate(val); },

				LUADOC_M("rotationXYZ", "(x: float, y: float, z: float) : mat4 - Makes rotation matrix around specified axes"),
				[](const Vector3D& val) {return rotateXYZ4(val.x, val.y, val.z); },

				LUADOC_M("rotationZXY", "(x: float, y: float, z: float) : mat4 - Makes rotation matrix around specified axes"),
				[](const Vector3D& val) {return rotateZXY4(val.x, val.y, val.z); }
			);
			LUADOC_P("r<n>", "access matrix row by number as vec4");
			LUADOC_P("m<row><column>", "access matrix value by row - column");
		}

		//----------------------------------------------------
		vec["AngleVectors"] = [](const Vector3D& v) {
			Vector3D forward, right, up;
			AngleVectors(v, &forward, &right, &up);
			return std::make_tuple(forward, right, up);
		};
#endif
	}

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
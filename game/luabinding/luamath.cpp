#include "game/pch.h"
#include "luamath.h"


#define VEC_OPERATORS_ONLY(vec_type, name) \
	/* vec - vec */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator/),\
	/* negate */\
	sol::meta_function::unary_minus, sol::resolve<vec_type(const vec_type&)>(&operator-)


#define VEC_OPERATORS(vec_type, name) \
	VEC_OPERATORS_ONLY(vec_type, name),\
	/* common functions */ \
	LUADOC_M("dot", "(a: " name ", b: " name "): float"), sol::resolve<float(const vec_type&, const vec_type&)>(dot),\
	LUADOC_M("normalize", "(v: " name "): " name), sol::resolve<vec_type(const vec_type&)>(normalize),\
	LUADOC_M("length", "(v: " name "): float"), sol::resolve<float(const vec_type&)>(length),\
	LUADOC_M("lengthSqr", "(v: " name "): float"), sol::resolve<float(const vec_type&)>(lengthSqr),\
	LUADOC_M("distance", "(a: " name ", b: " name "): float"), sol::resolve<float(const vec_type&, const vec_type&)>(distance),\
	LUADOC_M("lerp", "(a: " name ", b: " name ", v: float): " name), sol::resolve<vec_type(const vec_type&, const vec_type&, float)>(lerp),\
	LUADOC_M("cerp", "(a: " name ", b: " name ", c: " name ", v: float): " name), sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&, const vec_type&, float)>(cerp),\
	LUADOC_M("sign", "(v: " name "): " name), sol::resolve<vec_type(const vec_type&)>(sign),\
	LUADOC_M("clamp", "(v: " name ", min: " name ", max: " name "): " name), sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&)>(clamp),

// NOT USED - conflicting (SAD)
#define VEC_FLOAT_OPERATORS(vec_type) \
	/* vec - float */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const float&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const float&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const float&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const float&)>(&operator/),\

void Math_Lua_Init(sol::state& lua)
{
	//-----------------------------------
	// 3D MATH
	{
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
				sol::meta_function::construct, sol::constructors<Vector2D(float), Vector2D(float, float)>(),
				// init from table
				sol::meta_function::construct, sol::factories(
					[](const sol::table& table) {
						return Vector2D(table["x"], table["y"]);
					}),
				VEC_OPERATORS(Vector2D, "vec2")
				LUADOC_P("x"), &Vector2D::x,
				LUADOC_P("y"), &Vector2D::y);
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
				VEC_OPERATORS(Vector3D, "vec3")
				LUADOC_M("cross", "(a: vec3, b: vec3) : vec3"), sol::resolve<Vector3D(const Vector3D&, const Vector3D&)>(cross),
				// members
				LUADOC_P("x"), &Vector3D::x,
				LUADOC_P("y"), &Vector3D::y,
				LUADOC_P("z"), &Vector3D::z);
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
				sol::call_constructor, sol::constructors<Vector3D(const float&), Vector4D(const float&, const float&, const float&, const float&)>(),
				VEC_OPERATORS(Vector4D, "vec4")
				// members
				"x", &Vector4D::x,
				"y", &Vector4D::y,
				"z", &Vector4D::z,
				"w", &Vector4D::w);
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
	}

	//
	// FIXED MATH
	//
	{
		LUADOC_NAMESPACE("fix");
		auto& fix = lua["fix"].get_or_create<sol::table>();

		VECTOR_NOPAD test = (VECTOR_NOPAD{ 0, 10, 0 }) + (VECTOR_NOPAD{ 15, 5, 0 });
		test += VECTOR_NOPAD{ 1, 4, 1 };

		//
		// Fixed Vector 3D
		//
		{
			{
				MAKE_PROPERTY_REF(lua, VECTOR_NOPAD);
				LUADOC_TYPE();
				fix.new_usertype<VECTOR_NOPAD>(
					LUADOC_T("VECTOR", "Three dimensional vector (32 bit)"),
					sol::call_constructor, sol::factories(
						[](const int& x, const int& y, const int& z) {
							return VECTOR_NOPAD{ x, y, z };
						},
						[](const sol::table& table) {
							return VECTOR_NOPAD{ table["x"], table["y"], table["z"] };
						},
						[](const SVECTOR& vec) {
							return VECTOR_NOPAD{ vec.vx, vec.vy, vec.vz };
						},
						[]() { return VECTOR_NOPAD{ 0 }; }),
					LUADOC_P("vx"), &VECTOR_NOPAD::vx,
					LUADOC_P("vy"), &VECTOR_NOPAD::vy,
					LUADOC_P("vz"), &VECTOR_NOPAD::vz,
					/* vec - vec */
					sol::meta_function::addition, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator+),
					sol::meta_function::subtraction, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator-),
					sol::meta_function::multiplication, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator*),
					sol::meta_function::division, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator/),
					/* negate */
					sol::meta_function::unary_minus, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&)>(&operator-),
					/* bit shifts*/
					sol::meta_function::bitwise_left_shift, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator<<),
					sol::meta_function::bitwise_right_shift, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&operator>>),
					sol::meta_function::bitwise_and, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const int&)>(&(operator&)),
					sol::meta_function::bitwise_or, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const int&)>(&(operator|)),
					sol::meta_function::bitwise_xor, sol::resolve<VECTOR_NOPAD(const VECTOR_NOPAD&, const VECTOR_NOPAD&)>(&(operator^))
				);
			}

			// Fixed Short vector 3D (for car cosmetics and shit)
			{
				MAKE_PROPERTY_REF(lua, SVECTOR);
				LUADOC_TYPE();
				fix.new_usertype<SVECTOR>(
					LUADOC_T("SVECTOR", "Three dimensional vector (16 bit)"),
					sol::call_constructor, sol::factories(
						[](const short& x, const short& y, const short& z) {
							return SVECTOR{ x, y, z, 0 };
						},
						[](const sol::table& table) {
							return SVECTOR{ (short)table["x"], (short)table["y"], (short)table["z"], 0 };
						},
						[]() { return SVECTOR{ 0 }; }),
					LUADOC_P("vx"), &SVECTOR::vx,
					LUADOC_P("vy"), &SVECTOR::vy,
					LUADOC_P("vz"), &SVECTOR::vz
				);
			}

			fix["ONE"] = ONE;
			fix["ONE_BITS"] = ONE_BITS;
			fix["toRadian"] = TO_RADIAN;
			fix["toGTEAngle"] = TO_GTE_ANGLE;

			fix["ToFixed"]		= [](const float& a)					{ return int(a * ONE_F); };
			fix["FromFixed"]	= [](const int& a)						{ return float(a) / ONE_F; };
			fix["DivHalfRound"]	= [](const int& a, const int& bits)		{ return FixDivHalfRound(a, bits); };
			fix["DIFF_ANGLES"] = sol::overload(
				[](const int& x, const int& y)		{ return DIFF_ANGLES(x, y); },
				[](const float& x, const float& y)	{ return DIFF_ANGLES_F(x, y); }
			);

			fix["ToFixedVector"] = &ToFixedVector;
			fix["FromFixedVector"] = sol::overload(
				sol::resolve<Vector3D(const VECTOR_NOPAD&)>(&FromFixedVector), 
				sol::resolve<Vector3D(const SVECTOR&)>(&FromFixedVector),
				sol::resolve<Vector3D(const SVECTOR_NOPAD&)>(&FromFixedVector)
			);
		}
	}

	{
		LUADOC_NAMESPACE("gte");

		// extend Lua math
		auto& gte = lua["gte"].get_or_create<sol::table>();

		gte["isin"] = &isin;
		gte["icos"] = &icos;
		gte["ratan2"] = &ratan2;
		gte["SquareRoot0"] = &SquareRoot0;
		// gte["MulMatrix0"] = &MulMatrix0;
	}
}
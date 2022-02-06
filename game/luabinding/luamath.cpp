#include "game/pch.h"
#include "luamath.h"

#define VEC_OPERATORS(vec_type) \
	/* vec - vec */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator/),\
	/* negate */\
	sol::meta_function::unary_minus, sol::resolve<vec_type(const vec_type&)>(&operator-),\
	/* common functions */ \
	LUADOC_M("dot"), sol::resolve<float(const vec_type&, const vec_type&)>(dot),\
	LUADOC_M("normalize"), sol::resolve<vec_type(const vec_type&)>(normalize),\
	LUADOC_M("length"), sol::resolve<float(const vec_type&)>(length),\
	LUADOC_M("lengthSqr"), sol::resolve<float(const vec_type&)>(lengthSqr),\
	LUADOC_M("distance"), sol::resolve<float(const vec_type&, const vec_type&)>(distance),\
	LUADOC_M("lerp"), sol::resolve<vec_type(const vec_type&, const vec_type&, float)>(lerp),\
	LUADOC_M("cerp"), sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&, const vec_type&, float)>(cerp),\
	LUADOC_M("sign"), sol::resolve<vec_type(const vec_type&)>(sign),\
	LUADOC_M("clamp"), sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&)>(clamp),

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
				VEC_OPERATORS(Vector2D)
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
				VEC_OPERATORS(Vector3D)
				LUADOC_M("cross", "Cross product with other vector"), sol::resolve<Vector3D(const Vector3D&, const Vector3D&)>(cross),
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
				VEC_OPERATORS(Vector4D)
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
				LUADOC_M("transformVec", "Transforms input vector by the matrix"), 
				[](Matrix3x3& self, const Vector3D& vec) {return transform3(vec, self); },

				LUADOC_M("transformVecInv", "Transforms input vector by the matrix"),
				[](Matrix3x3& self, const Vector3D& vec) {return transform3Inv(vec, self); },

				LUADOC_M("transposed", "Returns transposed matrix"), 
				[](Matrix3x3& self) { return transpose(self); },

				LUADOC_M("eulersXYZ", "Returns euler angles in specified rotation order"), 
				[](Matrix3x3& self) { return EulerMatrixXYZ(self); },

				LUADOC_M("eulersZXY", "Returns euler angles in specified rotation order"),
				[](Matrix3x3& self) { return EulerMatrixZXY(self); },

				// common matrix generators
				LUADOC_M("identity", "Returns new identity matrix"), []() {return identity3(); },

				LUADOC_M("rotationX", "Makes rotation matrix around specified axis"),
				[](float val) {return rotateX3(val); },

				LUADOC_M("rotationY", "Makes rotation matrix around specified axis"),
				[](float val) {return rotateY3(val); },

				LUADOC_M("rotationZ", "Makes rotation matrix around specified axis"),
				[](float val) {return rotateZ3(val); },

				LUADOC_M("rotationXYZ", "Makes rotation matrix around specified axis"),
				[](const Vector3D& val) {return rotateXYZ3(val.x, val.y, val.z); },

				LUADOC_M("rotationZXY", "Makes rotation matrix around specified axis"),
				[](const Vector3D& val) {return rotateZXY3(val.x, val.y, val.z); }
			);
			LUADOC_P("r<n>", "access matrix row by number as vec3");
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
						[]() { return VECTOR_NOPAD{ 0 }; }),
					LUADOC_P("vx"), &VECTOR_NOPAD::vx,
					LUADOC_P("vy"), &VECTOR_NOPAD::vy,
					LUADOC_P("vz"), &VECTOR_NOPAD::vz
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
							return SVECTOR{ x, y, z };
						},
						[](const sol::table& table) {
							return SVECTOR{ table["x"], table["y"], table["z"] };
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
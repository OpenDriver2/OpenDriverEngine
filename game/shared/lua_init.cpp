#include "game/pch.h"
#include "lua_init.h"
#include "input.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sol_ImGui/sol_imgui.h>

enum EDocPropType
{
	DocProp_MemberFunc,
	DocProp_Property
};

struct LuaDocProp
{
	EDocPropType type;
	String declName;
	String description;
};

struct LuaDocItem
{
	String namespaceName;
	String declName;
	String description;
	Array<LuaDocProp> members;
};

Array<LuaDocItem*> g_luaDoc_typeList;

CLuaDocumentation::NamespaceGuard::NamespaceGuard(const char* name /*= nullptr*/)
{
	m_name = name ? String::fromCString(name) : "_G";
}

CLuaDocumentation::NamespaceGuard::~NamespaceGuard()
{

}

CLuaDocumentation::TypeGuard::TypeGuard(NamespaceGuard& ns, const char* name /*= nullptr*/, const char* docText /*= nullptr*/)
	: m_ns(ns)
{
	m_item = new LuaDocItem();
	m_item->namespaceName = m_ns.m_name;

	if (name)
		Init(name, docText);

	g_luaDoc_typeList.append(m_item);
}

CLuaDocumentation::TypeGuard::~TypeGuard()
{
}

const char* CLuaDocumentation::TypeGuard::Init(const char* name, const char* docText /*= nullptr*/)
{
	m_item->declName = String::fromCString(name);
	m_item->description = docText ? String::fromCString(docText) : "";
	return name;
}

const char* CLuaDocumentation::TypeGuard::Property(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_Property,
		String::fromCString(name),
		docText ? String::fromCString(docText) : ""
		});

	return name;
}

const char* CLuaDocumentation::TypeGuard::MemberFunc(const char* name, const char* docText /*= nullptr*/)
{
	m_item->members.append(LuaDocProp{
		DocProp_MemberFunc,
		String::fromCString(name),
		docText ? String::fromCString(docText) : ""
		});

	return name;
}

//--------------------------------------

void CLuaDocumentation::Initialize(sol::state& lua)
{
	auto& docsTable = lua["docs"].get_or_create<sol::table>();

	for (usize i = 0; i < g_luaDoc_typeList.size(); i++)
	{
		LuaDocItem* doc = g_luaDoc_typeList[i];

		auto& namespaceTable = docsTable[(char*)doc->namespaceName].get_or_create<sol::table>();

		auto& declTable = namespaceTable[(char*)doc->declName].get_or_create<sol::table>();
		declTable["description"] = (char*)doc->description;

		auto& membersTable = declTable["members"].get_or_create<sol::table>();
		for (usize j = 0; j < doc->members.size(); j++)
		{
			LuaDocProp& prop = doc->members[j];
			membersTable[(char*)prop.declName] = (char*)prop.description;
		}

		delete doc;
	}
}

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

void CDebugOverlay_Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();
	auto engine = lua["engine"].get_or_create<sol::table>();

	{
		auto debugOverlay = engine["DebugOverlay"].get_or_create<sol::table>();

		LUADOC_TYPE("DebugOverlay");

		debugOverlay[LUADOC_M("SetTransform", "mat4")] = &CDebugOverlay::SetTransform;
		debugOverlay[LUADOC_M("Line", "pointA, pointB, color")] = &CDebugOverlay::Line;
		debugOverlay[LUADOC_M("Box", "minPoint, maxPoint, color")] = &CDebugOverlay::Box;
		debugOverlay[LUADOC_P("Enabled")] = sol::property(&CDebugOverlay::IsEnabled, &CDebugOverlay::Enable);
	}
	
}

void IAudioSystem_Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();

	// wave source data
	{
		LUADOC_TYPE();
		lua.new_usertype<ISoundSource>(
			LUADOC_T("ISoundSource", "Audio sound source"),

			LUADOC_M("GetFilename"), 
			&ISoundSource::GetFilename,

			LUADOC_M("IsStreaming", "true if sample is streaming"), 
			&ISoundSource::IsStreaming
		);
	}

	// audio system itself
	{
		LUADOC_TYPE();
		lua.new_usertype<IAudioSystem>(
			LUADOC_T("IAudioSystem"),

			LUADOC_M("CreateSource", "Creates new audible source"),
			&IAudioSystem::CreateSource,

			LUADOC_M("DestroySource"),
			&IAudioSystem::DestroySource,

			LUADOC_M("LoadSample", "Loads new sample (.wav, .ogg)"),
			&IAudioSystem::LoadSample,

			LUADOC_M("FreeSample"),
			&IAudioSystem::FreeSample
		);
	}

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Audio"] = IAudioSystem::Instance;
}

void LuaInit(sol::state& lua)
{
	lua.open_libraries(sol::lib::base);
	lua.open_libraries(sol::lib::debug);
	lua.open_libraries(sol::lib::math);
	lua.open_libraries(sol::lib::bit32);
	lua.open_libraries(sol::lib::string);
	lua.open_libraries(sol::lib::table);

	sol_ImGui::InitBindings(lua);

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
	// 3D MATH
	{
		LUADOC_NAMESPACE("vec");

		auto& vec = lua["vec"].get_or_create<sol::table>();

		//
		// Vector 2D
		//
		{
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
				LUADOC_M("identity"), []() {return identity3(); },

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
			fix["DIFF_ANGLES"]  = [](const int& x, const int& y)		{ return DIFF_ANGLES(x, y); };

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

	//-----------------------------------
	// MODULES
	IAudioSystem_Lua_Init(lua);
	CDebugOverlay_Lua_Init(lua);

	CInput::Lua_Init(lua);
	CManager_Players::Lua_Init(lua);
	CManager_Cars::Lua_Init(lua);
	CWorld::Lua_Init(lua);
	CSky::Lua_Init(lua);
	CCamera::Lua_Init(lua);

	CLuaDocumentation::Initialize(lua);
}
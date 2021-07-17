
#include "core/cmdlib.h"

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include <nstd/Array.hpp>
#include <nstd/File.hpp>
#include <nstd/Directory.hpp>
#include <nstd/String.hpp>
#include <nstd/Time.hpp>
#include <nstd/Math.hpp>

#include "lua_init.h"

#include "renderer/gl_renderer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sol_ImGui/sol_imgui.h>

#include "camera.h"
#include "world.h"
#include "input.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/psx_math_types.h"
#include "math/isin.h"
#include "math/squareroot0.h"
#include "math/ratan2.h"
#include "math/convert.h"

#include "game/shared/manager_cars.h"
#include "game/render/render_sky.h"

#include "renderer/debug_overlay.h"

#define VEC_OPERATORS(vec_type) \
	/* vec - vec */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator/),\
	/* negate */\
	sol::meta_function::unary_minus, sol::resolve<vec_type(const vec_type&)>(&operator-),\
	/* common functions */ \
	"dot", sol::resolve<float(const vec_type&, const vec_type&)>(dot),\
	"normalize", sol::resolve<vec_type(const vec_type&)>(normalize),\
	"length", sol::resolve<float(const vec_type&)>(length),\
	"lengthSqr", sol::resolve<float(const vec_type&)>(lengthSqr),\
	"distance", sol::resolve<float(const vec_type&, const vec_type&)>(distance),\
	"lerp", sol::resolve<vec_type(const vec_type&, const vec_type&, float)>(lerp),\
	"cerp", sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&, const vec_type&, float)>(cerp),\
	"sign", sol::resolve<vec_type(const vec_type&)>(sign),\
	"clamp", sol::resolve<vec_type(const vec_type&, const vec_type&, const vec_type&)>(clamp),

// NOT USED - conflicting (SAD)
#define VEC_FLOAT_OPERATORS(vec_type) \
	/* vec - float */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const float&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const float&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const float&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const float&)>(&operator/),\

void LuaInit(sol::state& lua)
{
	lua.open_libraries(sol::lib::base);
	lua.open_libraries(sol::lib::debug);
	lua.open_libraries(sol::lib::math);
	lua.open_libraries(sol::lib::bit32);
	lua.open_libraries(sol::lib::string);

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
	auto& vec = lua["vec"].get_or_create<sol::table>();

	//
	// Vector 2D
	//
	vec.new_usertype<Vector2D>("vec2",
		sol::meta_function::construct, sol::constructors<Vector2D(float), Vector2D(float, float)>(),
		// init from table
		sol::meta_function::construct, sol::factories(
			[](const sol::table& table) {
				return Vector2D(table["x"], table["y"]);
			}),
		VEC_OPERATORS(Vector2D)
		"x", &Vector2D::x,
		"y", &Vector2D::y);

	//
	// Vector 3D
	//
	vec.new_usertype<Vector3D>("vec3",
		sol::call_constructor, sol::factories(
			[](const sol::table& table) {
				return Vector3D(table["x"], table["y"], table["z"]);
			}),
		sol::call_constructor, sol::constructors<Vector3D(const float&, const float&, const float&), Vector3D(const float&)>(),
		VEC_OPERATORS(Vector3D)
		"cross", sol::resolve<Vector3D(const Vector3D&, const Vector3D&)>(cross),
		// members
		"x", &Vector3D::x,
		"y", &Vector3D::y,
		"z", &Vector3D::z);

	//
	// Vector 4D
	//
	vec.new_usertype<Vector4D>("vec4",
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

	//----------------------------------------------------

	vec["AngleVectors"] = [](const Vector3D& v) {
		Vector3D forward, right, up;
		AngleVectors(v, &forward, &right, &up);
		return std::make_tuple(forward, right, up);
	};

	//
	// FIXED MATH
	//
	auto& fix = lua["fix"].get_or_create<sol::table>();

	//
	// Fixed Vector 3D
	//
	fix.new_usertype<VECTOR_NOPAD>("VECTOR",
		sol::call_constructor, sol::factories(
			[](const int& x, const int& y, const int& z) {
				return VECTOR_NOPAD{ x, y, z };
			},
			[](const sol::table& table) {
				return VECTOR_NOPAD{ table["x"], table["y"], table["z"] };
			},
			[]() { return VECTOR_NOPAD{ 0 }; }),
			"vx", &VECTOR_NOPAD::vx,
			"vy", &VECTOR_NOPAD::vy,
			"vz", &VECTOR_NOPAD::vz);

	// Fixed Short vector 3D (for car cosmetics and shit)
	fix.new_usertype<SVECTOR>("SVECTOR",
		sol::call_constructor, sol::factories(
			[](const short& x, const short& y, const short& z) {
				return SVECTOR{ x, y, z };
			},
			[](const sol::table& table) {
				return SVECTOR{ table["x"], table["y"], table["z"] };
			},
				[]() { return SVECTOR{ 0 }; }),
		"vx", &SVECTOR::vx,
		"vy", &SVECTOR::vy,
		"vz", &SVECTOR::vz);

	fix["ONE"] = ONE;

	fix["ToFixed"]		= [](const float& a)	{ return int(a * ONE_F); };
	fix["FromFixed"]	= [](const int& a)		{ return float(a) / ONE_F; };

	fix["ToFixedVector"] = &ToFixedVector;
	fix["FromFixedVector"] = &FromFixedVector;

	// extend Lua math
	auto& gte = lua["gte"].get_or_create<sol::table>();

	gte["isin"] = &isin;
	gte["icos"] = &icos;
	gte["SquareRoot0"] = &SquareRoot0;
	gte["ratan2"] = &ratan2;
	// gte["MulMatrix0"] = &MulMatrix0;

	//-----------------------------------
	// MODULES
	CInput::Lua_Init(lua);
	CManager_Cars::Lua_Init(lua);
	CDebugOverlay::Lua_Init(lua);
	CWorld::Lua_Init(lua);
	CSky::Lua_Init(lua);
	CCamera::Lua_Init(lua);
}
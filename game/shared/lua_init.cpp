
#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include <nstd/Array.hpp>
#include <nstd/File.hpp>
#include <nstd/Directory.hpp>
#include <nstd/String.hpp>
#include <nstd/Time.hpp>

#include "lua_init.h"

#include "renderer/gl_renderer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sol_ImGui/sol_imgui.h>

#include "game/shared/manager_cars.h"
#include "game/render/render_sky.h"

#include "renderer/debug_overlay.h"

#include "camera.h"
#include "world.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/psx_math_types.h"
#include "math/isin.h"
#include "math/squareroot0.h"
#include "math/ratan2.h"
#include "math/convert.h"

#define VEC_OPERATORS(vec_type) \
	/* vec - vec */\
	sol::meta_function::addition, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator+),\
	sol::meta_function::subtraction, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator-),\
	sol::meta_function::multiplication, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator*),\
	sol::meta_function::division, sol::resolve<vec_type(const vec_type&, const vec_type&)>(&operator/),\
	/* negate */\
	sol::meta_function::unary_minus, sol::resolve<vec_type(const vec_type&)>(&operator-),

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

	//
	// Vector 2D
	//
	vec.new_usertype<VECTOR_NOPAD>("FVECTOR",
		sol::call_constructor, sol::factories(
			[](const sol::table& table) {
				return VECTOR_NOPAD{ table["x"], table["y"], table["z"] };
			},
			[]() { return VECTOR_NOPAD{ 0 }; }),
		"vx", &VECTOR_NOPAD::vx,
		"vy", &VECTOR_NOPAD::vy,
		"vz", &VECTOR_NOPAD::vz);

	auto& fix = lua["fix"].get_or_create<sol::table>();

	vec["ONE"] = ONE;

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
	CManager_Cars::Lua_Init(lua);
	CDebugOverlay::Lua_Init(lua);
	CWorld::Lua_Init(lua);
	CSky::Lua_Init(lua);
	CCamera::Lua_Init(lua);
}
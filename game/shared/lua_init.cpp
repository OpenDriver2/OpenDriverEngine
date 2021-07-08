
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

//typedef normalize<Vector2D> vec2_normalize;

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

	vec.new_usertype<Vector2D>("vec2",
		"x", &Vector2D::x,
		"y", &Vector2D::y);

	vec.new_usertype<Vector3D>("vec3",
		"x", &Vector3D::x,
		"y", &Vector3D::y,
		"z", &Vector3D::z);

	vec.new_usertype<Vector4D>("vec4",
		"x", &Vector4D::x,
		"y", &Vector4D::y,
		"z", &Vector4D::z,
		"w", &Vector4D::w);

	vec.new_usertype<VECTOR_NOPAD>("FIXED_VECTOR",
		"vx", &VECTOR_NOPAD::vx,
		"vy", &VECTOR_NOPAD::vy,
		"vz", &VECTOR_NOPAD::vz);

	// extend Lua math
	auto& gte = lua["gte"].get_or_create<sol::table>();

	gte["isin"] = &isin;
	gte["icos"] = &icos;
	// gte["SquareRoot0"] = &SquareRoot0;
	// gte["MulMatrix0"] = &MulMatrix0;

	//-----------------------------------
	// MODULES
	CManager_Cars::Lua_Init(lua);
	CDebugOverlay::Lua_Init(lua);
	CWorld::Lua_Init(lua);
	CSky::Lua_Init(lua);
	CCamera::Lua_Init(lua);
}
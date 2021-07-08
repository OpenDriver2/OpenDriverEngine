#include "core/ignore_vc_new.h"

#include <sol/sol.hpp>
#include <nstd/Array.hpp>

#include "core/cmdlib.h"
#include "manager_cars.h"

static CManager_Cars s_carManagerInstance;
CManager_Cars* g_cars = &s_carManagerInstance;

/*static*/ void	CManager_Cars::Lua_Init(sol::state& lua)
{
	lua.new_usertype<CManager_Cars>(
		"CManager_Cars",
		"UpdateControl", &CManager_Cars::UpdateControl,
		"GlobalTimeStep", &CManager_Cars::GlobalTimeStep,
		"DoScenaryCollisions", &CManager_Cars::DoScenaryCollisions);

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Cars"] = g_cars;
}

void CManager_Cars::UpdateControl()
{
	MsgWarning("Unimplemented CManager_Cars::UpdateControl()\n");
}

void CManager_Cars::GlobalTimeStep()
{
	MsgWarning("Unimplemented CManager_Cars::GlobalTimeStep()\n");
}

void CManager_Cars::DoScenaryCollisions()
{
	MsgWarning("Unimplemented CManager_Cars::DoScenaryCollisions()\n");
}
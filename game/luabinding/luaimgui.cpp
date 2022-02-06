#include "game/pch.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <sol_ImGui/sol_imgui.h>

void LuaImGuiInit(sol::state& lua)
{
	sol_ImGui::InitBindings(lua);
}

#ifndef INPUT_H
#define INPUT_H

#include <sol/forward.hpp>

union SDL_Event;

class CInput
{
public:
	static void UpdateEvents(SDL_Event& event, sol::table& engineHostTable, bool imguiFocused);

	static void Lua_Init(sol::state& lua);
};

#endif // INPUT_H
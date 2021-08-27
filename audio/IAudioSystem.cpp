#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <nstd/Array.hpp>
#include "audioSystemAL.h"

IAudioSystem* IAudioSystem::Instance = nullptr;

void IAudioSystem::Lua_Init(sol::state& lua)
{

}

IAudioSystem* IAudioSystem::Create()
{
	if (!IAudioSystem::Instance)
	{
		Instance = new CAudioSystemAL();
		Instance->Init();
	}

	return Instance;
}

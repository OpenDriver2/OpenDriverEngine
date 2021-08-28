#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <nstd/Array.hpp>
#include "audioSystemAL.h"

IAudioSystem* IAudioSystem::Instance = new CAudioSystemAL();
static bool s_audioInitialized = false;

void IAudioSystem::Lua_Init(sol::state& lua)
{
	// wave source data
	lua.new_usertype<ISoundSource>(
		"ISoundSource",
		"GetFilename", &ISoundSource::GetFilename,
		"IsStreaming", &ISoundSource::IsStreaming
	);

	// audio system itself
	lua.new_usertype<IAudioSystem>(
		"IAudioSystem",

		"CreateSource", &IAudioSystem::CreateSource,
		"DestroySource", &IAudioSystem::DestroySource,

		"LoadSample", &IAudioSystem::LoadSample,
		"FreeSample", &IAudioSystem::FreeSample
	);

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Audio"] = IAudioSystem::Instance;
}

IAudioSystem* IAudioSystem::Create()
{
	if (!s_audioInitialized)
		Instance->Init();

	return Instance;
}

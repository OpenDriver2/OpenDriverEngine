#include <AL/al.h>
#include <AL/alc.h>
#include <nstd/Array.hpp>
#include "audioSystemAL.h"

static CAudioSystemAL s_audioSystemAL;
IAudioSystem* IAudioSystem::Instance = &s_audioSystemAL;
static bool s_audioInitialized = false;

IAudioSystem* IAudioSystem::Create()
{
	if (!s_audioInitialized)
		Instance->Init();

	return Instance;
}

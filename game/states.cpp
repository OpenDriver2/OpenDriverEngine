#include "core/core_common.h"
#include "core/IDkCore.h"
#include "core/IEqParallelJobs.h"
#include "core/IFileSystem.h"
#include "core/ConCommand.h"
#include "core/ConVar.h"
#include "core/IConsoleCommands.h"

#include "sys/sys_host.h"
#include "sys/scripting/sys_esl.h"
#include "states.h"
#include "imgui_backend/imgui_host.h"
#include "audio/eqSoundEmitterSystem.h"
#include "studio/StudioCache.h"
#include "materialsystem1/IMaterialSystem.h"

#ifdef IMGUI_ENABLED
#include <imgui.h>
#include "audio/SoundScriptEditorUI.h"
//#include "instancer.h"
#endif

#include "state_game.h"
#include "physics/IStudioShapeCache.h"

#define GAME_WINDOW_TITLE	"Driver"

enum ESoundChannelType
{
	CHAN_STATIC = 0,
	CHAN_BODY,
	CHAN_SKID,
	CHAN_ITEM,
	CHAN_SIGNAL,
	CHAN_ENGINE,
	CHAN_VOICE,

	CHAN_STREAM,
	CHAN_MENU,

	CHAN_COUNT
};

static ChannelDef s_soundChannels[] = {
	DEFINE_SOUND_CHANNEL(CHAN_STATIC, 16),	// anything that dont fit categories below
	DEFINE_SOUND_CHANNEL(CHAN_BODY, 16),	// hit sounds
	DEFINE_SOUND_CHANNEL(CHAN_SKID, 36),	// car skid sounds
	DEFINE_SOUND_CHANNEL(CHAN_ITEM, 8),
	DEFINE_SOUND_CHANNEL(CHAN_SIGNAL, 4),	// horn, siren and rear gear beep
	DEFINE_SOUND_CHANNEL(CHAN_ENGINE, 6),	// rev, non-rev and idle sounds
	DEFINE_SOUND_CHANNEL(CHAN_VOICE, 2),
	DEFINE_SOUND_CHANNEL(CHAN_STREAM, 2),
	DEFINE_SOUND_CHANNEL(CHAN_MENU, 16),	// hit sounds
};
static_assert(elementsOf(s_soundChannels) == CHAN_COUNT, "ESoundChannelType needs to be in sync with s_soundChannels");

namespace OpenDriverUnits
{
	static constexpr float DefaultSoundDistance = 100.0f;
}

static CEmptyStudioShapeCache s_shapeCache;

bool InitScriptState()
{
	const esl::ScriptState state = eslSys::GetScriptState();
	ESL_SYS_INIT(eslSysInit);
	//ESL_SYS_INIT(eslSysOpenDriverInit);
	return true;
}

namespace eqAppStateMng
{
static CAppStateBase* s_appStates[APP_STATE_COUNT] = { nullptr };
static StatePostUpdateEvent::Sub s_statePostUpdateSub;

const char* GetAppNameTitle()
{
	return GAME_WINDOW_TITLE;
}

CAppStateBase* GetAppStateByType(int stateType)
{
	return s_appStates[stateType];
}

void PostUpdateState(float fDt)
{
	PROF_EVENT("SoundSystem update");
	g_sounds->Update();
}

bool InitAppStates()
{
	s_appStates[APP_STATE_MAIN_GAMELOOP] = g_State_Game;

	g_pHost->SetWindowTitle(GAME_WINDOW_TITLE);
	g_eqCore->RegisterInterface(&s_shapeCache);

	g_sounds->Init(OpenDriverUnits::DefaultSoundDistance, s_soundChannels);
	g_studioCache->Init(g_parallelJobs->GetJobMng());

	s_statePostUpdateSub = g_onPostUpdateState.Subscribe(PostUpdateState);

#ifdef ENABLE_MULTIPLAYER
	Networking::InitNetworking();
#endif

	g_matSystem->RegisterShaderOverride("BaseUnlit", [](int instanceFormatId) -> const char* {
		if (instanceFormatId == SHADER_VERTEX_ID(EGFVertexGameObj))
			return "BaseUnlitGame";
		return nullptr;
	});

	g_matSystem->RegisterShaderOverride("DrvSynVehicle", [](int instanceFormatId) -> const char* {
		return "BaseUnlitGame";
	});

	g_matSystem->RegisterShaderOverride("Skinned", [](int instanceFormatId) -> const char* {
		return "BaseUnlitGame";
	});

	g_matSystem->RegisterShaderOverride("BaseParticle", [](int instanceFormatId) -> const char* {
		return "BaseUnlit";
	});

#ifdef IMGUI_ENABLED
	//g_imGuiHost->AddDebugMenu("ENGINE/RENDER/INSTANCE MANAGER DEBUG UI", DemoInstManagerDebugDrawUI);
#endif

	// Lua binding is initialized from here
	if (!InitScriptState())
	{
		ErrorMsg("Lua base initialization error:\n\n%s\n", esl::runtime::GetLastError(eslSys::GetScriptState()));
		return false;
	}

	eqAppStateMng::SetCurrentStateType(APP_STATE_MAIN_GAMELOOP);

	return true;
}

void ShutdownAppStates()
{
	s_statePostUpdateSub.Unsubscribe();

	for (int i = 0; i < APP_STATE_COUNT; ++i)
		s_appStates[i] = nullptr;

	eslSysTerm();
	eslSys::DestroyScriptState();

#ifdef ENABLE_MULTIPLAYER
	Networking::ShutdownNetworking();
#endif

	g_studioCache->Shutdown();
	g_eqCore->UnregisterInterface<CEmptyStudioShapeCache>();
}

bool IsPauseAllowed()
{
	return false;
}

void SignalPause()
{
}

}

#ifdef PLAT_ANDROID

#include "SDL_main.h"
#undef main

extern int main(int argc, char* argv[]);

extern "C"
{
	// GCC is a piece of shit when linking static with export
	DECLSPEC int SDL_main(int argc, char* argv[])
	{
		return main(argc, argv);
	}
}


#endif




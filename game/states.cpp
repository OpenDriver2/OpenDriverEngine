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

const char* GetAppNameTitle()
{
	return GAME_WINDOW_TITLE;
}

CAppStateBase* GetAppStateByType(int stateType)
{
	return s_appStates[stateType];
}

bool InitAppStates()
{
	s_appStates[APP_STATE_MAIN_GAMELOOP] = g_State_Game;

	g_pHost->SetWindowTitle(GAME_WINDOW_TITLE);
	g_eqCore->RegisterInterface(&s_shapeCache);

	g_sounds->Init(OpenDriverUnits::DefaultSoundDistance, s_soundChannels);
	g_studioCache->Init(g_parallelJobs->GetJobMng());


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
	for (int i = 0; i < APP_STATE_COUNT; ++i)
		s_appStates[i] = nullptr;

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




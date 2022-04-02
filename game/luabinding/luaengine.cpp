#include "game/pch.h"
#include "luamath.h"


void CDebugOverlay_Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();
	auto engine = lua["engine"].get_or_create<sol::table>();

	{
		LUADOC_TYPE();
		auto debugOverlay = engine[LUADOC_T("DebugOverlay")].get_or_create<sol::table>();
		
		debugOverlay[LUADOC_M("SetTransform", "(transform: vec.mat4)")] 
			= &CDebugOverlay::SetTransform;

		debugOverlay[LUADOC_M("Line", "(pointA: vec.vec3, pointB: vec.vec3, color: vec.vec4)")] 
			= &CDebugOverlay::Line;

		debugOverlay[LUADOC_M("Box", "(minPoint: vec.vec3, maxPoint: vec.vec3, color: vec.vec4)")] 
			= &CDebugOverlay::Box;

		debugOverlay[LUADOC_P("Enabled")] 
			= sol::property(&CDebugOverlay::IsEnabled, &CDebugOverlay::Enable);
	}
	
}

void IAudioSystem_Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();

	// wave source data
	{
		LUADOC_TYPE();
		lua.new_usertype<ISoundSource>(
			LUADOC_T("ISoundSource", "Audio sample data"),

			LUADOC_M("GetFilename", "(void) : string"),
			&ISoundSource::GetFilename,

			LUADOC_M("IsStreaming", "(void) : boolean - true if sample is streaming"), 
			&ISoundSource::IsStreaming
		);
	}

	// audible source parameters
	{
		LUADOC_TYPE();
		lua.new_usertype<IAudioSource::Params>(
			LUADOC_T("SOUND_PARAMS", "Audio source parameters"),

			sol::call_constructor, sol::default_constructor,

			LUADOC_P("position", "<vec.vec3>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_position<Vector3D>), &IAudioSource::Params::position),

			LUADOC_P("velocity", "<vec.vec3>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_velocity<Vector3D>), &IAudioSource::Params::velocity),

			LUADOC_P("volume", "<float>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_volume<float>), &IAudioSource::Params::volume),

			LUADOC_P("pitch", "<float>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_pitch<float>), &IAudioSource::Params::pitch),

			LUADOC_P("referenceDistance", "<float>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_referenceDistance<float>)	, &IAudioSource::Params::referenceDistance),

			LUADOC_P("rolloff", "<float>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_rolloff<float>), &IAudioSource::Params::rolloff),

			LUADOC_P("airAbsorption", "<float>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_airAbsorption<float>), &IAudioSource::Params::airAbsorption),

			LUADOC_P("state", "<SoundState>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_state<IAudioSource::State>), &IAudioSource::Params::state),

			LUADOC_P("effectSlot", "<int>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_effectSlot<int>), &IAudioSource::Params::effectSlot),

			LUADOC_P("relative", "<boolean>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_relative<bool>), &IAudioSource::Params::relative),

			LUADOC_P("looping", "<boolean>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_looping<bool>), &IAudioSource::Params::looping),

			LUADOC_P("releaseOnStop", "<boolean>"),
			sol::property(sol::resolve(&IAudioSource::Params::set_releaseOnStop<bool>), &IAudioSource::Params::releaseOnStop)
		);
	}

	{
		LUADOC_TYPE();
		LUA_BEGIN_ENUM(IAudioSource::State);
		lua.new_enum<IAudioSource::State>(LUADOC_T("SoundState"), {
			LUA_ENUM(STOPPED, "Stopped"),
			LUA_ENUM(PLAYING, "Playing"),
			LUA_ENUM(PAUSED, "Paused")
		});
	}

	// audible source
	{
		LUADOC_TYPE();
		lua.new_usertype<IAudioSource>(
			LUADOC_T("IAudioSource", "Audible sound source"),

			LUADOC_M("Setup", "(chanId: int, sample: ISoundSource) - sets up type id and sound sample for playback"), 
			[](IAudioSource* self, int chanId, ISoundSource* sample) { 
				// no callbacks are allowed
				self->Setup(chanId, sample, nullptr);
			},

			LUADOC_M("Release", "(void) - stops the sound and drops the reference"),
			&IAudioSource::Release,

			LUADOC_M("GetState", "(void) : SoundState - returns sound source playback state"),
			&IAudioSource::GetState,

			LUADOC_M("IsLooping", "(void) : bool - returns loop state"),
			&IAudioSource::IsLooping,

			LUADOC_P("params", "<SOUND_PARAMS> - get/set sound parameters"),
			sol::property([](IAudioSource* self) {
				IAudioSource::Params params;
				self->GetParams(params);
				return params;
			}, & IAudioSource::UpdateParams)
			
		);
	}

	// audio system itself
	{
		LUADOC_TYPE();
		lua.new_usertype<IAudioSystem>(
			LUADOC_T("IAudioSystem"),

			LUADOC_M("CreateSource", "(void) : IAudioSource - creates new audible source"),
			&IAudioSystem::CreateSource,

			LUADOC_M("DestroySource", "(source: IAudioSource) - force stop and destroy audible source"),
			&IAudioSystem::DestroySource,

			LUADOC_M("LoadSample", "(filename: string) : ISoundSource - Loads new sample (.wav, .ogg)"),
			&IAudioSystem::LoadSample,

			LUADOC_M("FreeSample", "(sample: ISoundSource)"),
			&IAudioSystem::FreeSample,

			LUADOC_M("StopAllSounds", "(chanId: int = -1)"),
			&IAudioSystem::StopAllSounds,

			LUADOC_M("PauseAllSounds", "(chanId: int = -1)"),
			&IAudioSystem::PauseAllSounds,

			LUADOC_M("ResumeAllSounds", "(chanId: int = -1)"),
			&IAudioSystem::ResumeAllSounds,

			LUADOC_M("SetChannelVolume", "(chanId: int, value: float)"),
			&IAudioSystem::SetChannelVolume,

			LUADOC_M("SetChannelPitch", "(chanId: int, value: float)"),
			&IAudioSystem::SetChannelPitch,

			LUADOC_M("SetMasterVolume", "(value: float)"),
			&IAudioSystem::SetMasterVolume
		);
	}

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Audio"] = IAudioSystem::Instance;
}

void Engine_Lua_PrintStackTrace(sol::state& lua)
{
	lua_State* L = lua;
	lua_Debug ar;
	int depth = 0;

	Msg("\nLua stack trace:\n");

	while (lua_getstack(L, depth, &ar))
	{
		int status = lua_getinfo(L, "Sln", &ar);
		assert(status);

		Msg("\t %s:", ar.short_src);
		if (ar.currentline > 0)
			Msg("%d:", ar.currentline);
		if (*ar.namewhat != '\0')  /* is there a name? */
			Msg(" in function '%s'", ar.name);
		else
		{
			if (*ar.what == 'm')  /* main? */
				Msg(" in main chunk");
			else if (*ar.what == 'C' || *ar.what == 't')
				Msg(" ?");  /* C function or tail call */
			else
				Msg(" in function <%s:%d>",
					ar.short_src, ar.linedefined);
		}
		Msg("\n");
		depth++;
	}
	Msg("\n");
}

void Engine_Lua_Init(sol::state& lua)
{
	Math_Lua_Init(lua);
	IAudioSystem_Lua_Init(lua);
	CDebugOverlay_Lua_Init(lua);
}
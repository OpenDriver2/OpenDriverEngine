#include "game/pch.h"
#include "luamath.h"


void CDebugOverlay_Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();
	auto engine = lua["engine"].get_or_create<sol::table>();

	{
		auto debugOverlay = engine["DebugOverlay"].get_or_create<sol::table>();

		LUADOC_TYPE("DebugOverlay");

		debugOverlay[LUADOC_M("SetTransform", "mat4")] = &CDebugOverlay::SetTransform;
		debugOverlay[LUADOC_M("Line", "pointA, pointB, color")] = &CDebugOverlay::Line;
		debugOverlay[LUADOC_M("Box", "minPoint, maxPoint, color")] = &CDebugOverlay::Box;
		debugOverlay[LUADOC_P("Enabled")] = sol::property(&CDebugOverlay::IsEnabled, &CDebugOverlay::Enable);
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

			LUADOC_M("GetFilename"), 
			&ISoundSource::GetFilename,

			LUADOC_M("IsStreaming", "true if sample is streaming"), 
			&ISoundSource::IsStreaming
		);
	}

	// audible source parameters
	{
		lua["SoundState"] = lua.create_table_with(
			"Stopped", IAudioSource::STOPPED,
			"Playing", IAudioSource::PLAYING,
			"Paused", IAudioSource::PAUSED
		);

		LUADOC_TYPE();
		lua.new_usertype<IAudioSource::Params>(
			LUADOC_T("SOUND_PARAMS", "Audio source parameters"),

			sol::call_constructor, sol::default_constructor,
			

			LUADOC_P("position"),
			sol::property(sol::resolve(&IAudioSource::Params::set_position<Vector3D>), &IAudioSource::Params::position),

			LUADOC_P("velocity"),
			sol::property(sol::resolve(&IAudioSource::Params::set_velocity<Vector3D>), &IAudioSource::Params::velocity),

			LUADOC_P("volume"),
			sol::property(sol::resolve(&IAudioSource::Params::set_volume<float>), &IAudioSource::Params::volume),

			LUADOC_P("pitch"),
			sol::property(sol::resolve(&IAudioSource::Params::set_pitch<float>), &IAudioSource::Params::pitch),

			LUADOC_P("referenceDistance"),
			sol::property(sol::resolve(&IAudioSource::Params::set_referenceDistance<float>)	, &IAudioSource::Params::referenceDistance),

			LUADOC_P("rolloff"),
			sol::property(sol::resolve(&IAudioSource::Params::set_rolloff<float>), &IAudioSource::Params::rolloff),

			LUADOC_P("airAbsorption"),
			sol::property(sol::resolve(&IAudioSource::Params::set_airAbsorption<float>), &IAudioSource::Params::airAbsorption),

			LUADOC_P("state"),
			sol::property(sol::resolve(&IAudioSource::Params::set_state<IAudioSource::State>), &IAudioSource::Params::state),

			LUADOC_P("effectSlot"),
			sol::property(sol::resolve(&IAudioSource::Params::set_effectSlot<int>), &IAudioSource::Params::effectSlot),

			LUADOC_P("relative"),
			sol::property(sol::resolve(&IAudioSource::Params::set_relative<bool>), &IAudioSource::Params::relative),

			LUADOC_P("looping"),
			sol::property(sol::resolve(&IAudioSource::Params::set_looping<bool>), &IAudioSource::Params::looping),

			LUADOC_P("releaseOnStop"),
			sol::property(sol::resolve(&IAudioSource::Params::set_releaseOnStop<bool>), &IAudioSource::Params::releaseOnStop)
		);
	}

	// audible source
	{
		LUADOC_TYPE();
		lua.new_usertype<IAudioSource>(
			LUADOC_T("IAudioSource", "Audible sound source"),

			LUADOC_M("Setup", "sets up type id and sound sample for playback"), 
			[](IAudioSource* self, int typeId, ISoundSource* sample) { 
				// no callbacks are allowed
				self->Setup(typeId, sample, nullptr);
			},

			LUADOC_M("UpdateParams", "updates sound parameters while this it is playing"), 
			&IAudioSource::UpdateParams,

			LUADOC_M("Release", "stops the sound and drops the reference"),
			&IAudioSource::Release,

			LUADOC_M("GetState", "returns sound source playback state"),
			&IAudioSource::GetState,

			LUADOC_M("IsLooping", "returns loop state"),
			&IAudioSource::IsLooping,

			LUADOC_M("params", "get/set sound parameters"),
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

			LUADOC_M("CreateSource", "Creates new audible source"),
			&IAudioSystem::CreateSource,

			LUADOC_M("DestroySource"),
			&IAudioSystem::DestroySource,

			LUADOC_M("LoadSample", "Loads new sample (.wav, .ogg)"),
			&IAudioSystem::LoadSample,

			LUADOC_M("FreeSample"),
			&IAudioSystem::FreeSample
		);
	}

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Audio"] = IAudioSystem::Instance;
}

void Engine_Lua_Init(sol::state& lua)
{
	Math_Lua_Init(lua);
	IAudioSystem_Lua_Init(lua);
	CDebugOverlay_Lua_Init(lua);
}
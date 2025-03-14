#include "core/core_common.h"

#include "players.h"
#include "replay.h"
#include "cars.h"

const int REPLAY_STEAM_MAX_LENGTH = 8000;

void CPlayer::Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();

	{
		LUADOC_TYPE();
		lua.new_usertype<InputData>(
			LUADOC_T("PlayerInputData"),
			sol::call_constructor, sol::factories(
				[](const sol::table& table) {
					InputData in;

					in.accel = table["accel"];
					in.brake = table["brake"];
					in.wheelspin = table["wheelspin"];
					in.handbrake = table["handbrake"];
					in.fastSteer = table["fastSteer"];
					in.steering = table["steering"];
					in.useAnalogue = table["useAnalogue"];
					in.horn = table["horn"];

					return in;
				},
				[]() { return InputData(); }),
			LUADOC_P("accel"), &InputData::accel,
			LUADOC_P("brake"), &InputData::brake,
			LUADOC_P("wheelspin"),  &InputData::wheelspin,
			LUADOC_P("handbrake"),  &InputData::handbrake,
			LUADOC_P("fastSteer"),  &InputData::fastSteer,
			LUADOC_P("steering"), &InputData::steering,
			LUADOC_P("useAnalogue"), &InputData::useAnalogue,
					
			LUADOC_P("horn"), &InputData::horn
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<CPlayer>(
			LUADOC_T("Player"),

			LUADOC_P("InitReplay", "(inputStream?: ReplayStream) - initializes replay stream. If inputStream is null, only recording is initiated"),
			&CPlayer::InitReplay,

			LUADOC_P("currentCar", "<CCar> - get/set player car. If replay is initialized empty, it will be initialized"), 
			sol::property(&CPlayer::GetCurrentCar, &CPlayer::SetCurrentCar),

			LUADOC_P("controlType", "<CarControlType>"),
			&CPlayer::m_controlType,

			LUADOC_P("input", "<PlayerInputData> - player control buttons data"), 
			sol::property(&CPlayer::m_currentInputs, &CPlayer::UpdateControls),

			LUADOC_P("rubberbandPowerRatio", "<int> - chase power ratio of rubber band (usually target car value)"),
			sol::property(&CPlayer::GetRubberbandPowerRatio, &CPlayer::SetRubberbandPowerRatio),

			LUADOC_P("rubberbandPoint", "<fix.VECTOR> - world position for computing rubberbanding scale"),
			sol::property(&CPlayer::GetRubberbandPoint, &CPlayer::SetRubberbandPoint),

			LUADOC_P("rubberbandMode", "<RubberbandMode> - rubberbanding mode preset"),
			sol::property(&CPlayer::GetRubberbandMode, &CPlayer::SetRubberbandMode),

			LUADOC_P("playbackStream", "<ReplayStream> (mayBeNull readonly) - replay playback stream"),
			sol::property([](const CPlayer& ply) { 
				CReplayStream& ptr = *ply.m_playbackStream;
				return &ptr;
			}),

			LUADOC_P("recordStream", "<ReplayStream> (mayBeNull readonly) - replay recording stream (if null - no recording is done)"),
			sol::property([](const CPlayer& ply) {
				CReplayStream& ptr = *ply.m_recordStream;
				return &ptr;
			})

			
		);
	}

	{
		LUADOC_TYPE();
		LUA_BEGIN_ENUM(ERubberBandMode);
		lua.new_enum<ERubberBandMode>(LUADOC_T("RubberbandMode"), {
			LUA_ENUM(Rubberband_Off, "Off"),
			LUA_ENUM(Rubberband_Chaser, "Chaser"),
			LUA_ENUM(Rubberband_Escape, "Escape"),
		});
	}
}

CPlayer::~CPlayer()
{
}

void CPlayer::Net_Init()
{
}

void CPlayer::Net_Finalize()
{
}

void CPlayer::InitReplay(CReplayStream* sourceStream /*= nullptr*/)
{
	if (sourceStream)
	{
		sourceStream->Reset();
	}
	else 
	{
		CReplayStream* ptr = &*m_recordStream;

		if (!m_recordStream)
		{
			m_recordStream = CRefPtr_new(CReplayStream, REPLAY_STEAM_MAX_LENGTH);
		}
		m_recordStream->Purge();
	}

	m_playbackStream.Assign(sourceStream);
}

EPlayerControlType CPlayer::GetControlType() const
{
	return m_controlType;
}

void CPlayer::SetCurrentCar(CCar* newCar)
{
	if (m_recordStream && m_recordStream->IsEmpty())
	{
		STREAM_SOURCE& streamSrc = m_recordStream->GetSourceParams();
		streamSrc.type = newCar ? 1 : 0;
		if (newCar) 
		{
			streamSrc.model = newCar->m_ap.model;
			streamSrc.palette = newCar->m_ap.palette;
			streamSrc.controlType = newCar->m_controlType;
			streamSrc.flags = 0;
			streamSrc.rotation = newCar->m_hd.direction;
			streamSrc.position = newCar->GetPosition();
			streamSrc.totaldamage = newCar->m_totalDamage;
			for (int i = 0; i < 6; i++) {
				streamSrc.damage[i] = newCar->m_ap.damage[i];
			}
		}
	}

	m_currentCar = newCar;
}

CCar* CPlayer::GetCurrentCar() const
{
	return m_currentCar;
}

void CPlayer::UpdateControls(const InputData& input)
{
	m_currentInputs = input;
}

void CPlayer::ProcessCarPad()
{
	//PED_MODEL_TYPES whoExit;
	//whoExit = TANNER_MODEL;

	// first in priority is playback stream, next is record
	if (m_playbackStream)
		m_playbackStream->Play(m_currentInputs);
	else if (m_recordStream)
		m_recordStream->Record(m_currentInputs);

	bool use_analogue = m_currentInputs.useAnalogue;

#if 0
	// Handle player car controls...
	if (m_currentCar->m_controlType == CONTROL_TYPE_PLAYER)
	{
		// handle car leaving
		if ((pad & CAR_PAD_LEAVECAR) == CAR_PAD_LEAVECAR && player_id > -1)
		{
			if (!TannerStuckInCar(1, player_id))
			{
				if (player[player_id].dying == 0)
				{
					if (ActiveCheats.cheat12 && (GameLevel == 1 || GameLevel == 2))		// [A] play as Jericho cheat
						whoExit = OTHER_MODEL;

					ActivatePlayerPedestrian(cp, NULL, 0, NULL, whoExit);
				}
			}
			else if (lockAllTheDoors != 0)
			{
				// this is to show message
				gLockPickingAttempted = 1;
			}
		}

		// Lock car if it has mission lock or fully damaged
		if (gStopPadReads != 0 || MaxPlayerDamage[*cp->ai.padid] <= cp->totalDamage || gCantDrive != 0)
		{
			pad = CAR_PAD_HANDBRAKE;

			// apply brakes
			if (cp->hd.wheel_speed > 36864)
				pad = CAR_PAD_BRAKE;

			int_steer = 0;
			use_analogue = 1;
		}

		// turn of horning
		if (player_id > -1)
		{
			if (CarHasSiren(cp->ap.model) == 0)
				player[player_id].horn.on = (pad >> 3) & 1;
			else if ((cp->lastPad & 8U) == 0 && (pad & 8) != 0)
				player[player_id].horn.on ^= 8;
		}
	}
#endif

	CCar* cp = m_currentCar;

	if (cp->m_hd.autoBrake > 90)
		cp->m_hd.autoBrake = 90;

	// handle burnouts or handbrake
	if (m_currentInputs.handbrake)
	{
		cp->m_handbrake = 1;
	}
	else
	{
		cp->m_handbrake = 0;

		if (m_currentInputs.wheelspin)
			cp->m_wheelspin = 1;
		else
			cp->m_wheelspin = 0;

		// continue without burnout
		if (cp->m_wheelspin != 0 && cp->m_hd.wheel_speed > cp->m_cosmetics.wheelspinMaxSpeed)
		{
			cp->m_wheelspin = 0;
			m_currentInputs.accel = true;
		}
	}

	// handle steering
	if (!use_analogue)
	{
		if (m_currentInputs.fastSteer)
		{
			// fast steer
			if (m_currentInputs.steering > 0)
			{
				cp->m_wheel_angle += 64;

				if (cp->m_wheel_angle > 511)
					cp->m_wheel_angle = 511;
			}

			if (m_currentInputs.steering < 0)
			{
				cp->m_wheel_angle -= 64;

				if (cp->m_wheel_angle < -511)
					cp->m_wheel_angle = -511;
			}
		}
		else
		{
			// regular steer
			if (m_currentInputs.steering > 0)
			{
				cp->m_wheel_angle += 32;

				if (cp->m_wheel_angle > 352)
					cp->m_wheel_angle = 352;
			}

			if (m_currentInputs.steering < 0)
			{
				cp->m_wheel_angle -= 32;

				if (cp->m_wheel_angle < -352)
					cp->m_wheel_angle = -352;
			}
		}

		if (m_currentInputs.steering != 0)
			cp->m_hd.autoBrake++;
		else
			cp->m_hd.autoBrake = 0;
	}
	else
	{
		int int_steer = m_currentInputs.steering;
		int analog_angle;

		if (m_currentInputs.fastSteer)
		{
			int_steer *= (int_steer * int_steer) / 60;
			analog_angle = ((long long)int_steer * 0x88888889) >> 32;		// int_steer * 0.6
		}
		else
		{
			int_steer *= (int_steer * int_steer) / 80;
			analog_angle = ((long long)int_steer * 0x66666667) >> 32;		// int_steer * 0.4
		}

		analog_angle = (analog_angle >> 5) - (int_steer >> 0x1f);

		cp->m_wheel_angle = analog_angle & 0xfffc;

		if (analog_angle + 270U < 541)
			cp->m_hd.autoBrake = 0;
		else
			cp->m_hd.autoBrake++;
	}

	// center steering
	if (m_currentInputs.steering == 0)
	{
		if (cp->m_wheel_angle < -64)
			cp->m_wheel_angle += 64;
		else if (cp->m_wheel_angle < 65)
			cp->m_wheel_angle = 0;
		else
			cp->m_wheel_angle -= 64;
	}

	cp->m_thrust = 0;

	if (m_currentInputs.brake)
	{
		int rws;

		// brakes
		rws = FIXEDH(cp->m_hd.wheel_speed * 1500 / 1024);

		if (-rws < 23)
			rws = -5000;
		else
			rws = ((rws + 278) * -4778) >> 8;

		cp->m_thrust = FIXEDH(cp->m_cosmetics.powerRatio * rws);
	}
	else if (m_currentInputs.accel)
	{
		// TODO: D1 scales support

		// accelerate faster if closer to point
		if (m_rubberbandMode == Rubberband_Escape)
		{
			const int dx = m_rubberbandPoint.vx - cp->m_hd.where.t[0] >> 10;
			const int dz = m_rubberbandPoint.vz - cp->m_hd.where.t[2] >> 10;

			const int dist = dx * dx + dz * dz;

			if (dist > 40)
				cp->m_thrust = 3000;
			else if (dist > 20)
				cp->m_thrust = 4000;
			else if (dist > 9)
				cp->m_thrust = 4900;
			else
				cp->m_thrust = 6000;
		}
		else
		{
			cp->m_thrust = FIXEDH(cp->m_cosmetics.powerRatio * 4915);
		}

		// accelerate faster if further to point
		if (m_rubberbandMode == Rubberband_Chaser)
		{
			if (cp->m_cosmetics.powerRatio > 3050)
				cp->m_thrust = FIXEDH(m_rubberbandPowerRatio * 4915);

			const int cx = cp->m_hd.where.t[0] - m_rubberbandPoint.vx >> 10;
			const int cz = cp->m_hd.where.t[2] - m_rubberbandPoint.vz >> 10;

			const int chase_square_dist = cx * cx + cz * cz;

			if (chase_square_dist > 20)
				cp->m_thrust = (cp->m_thrust * 8000) / 7000;
			else if (chase_square_dist > 6)
				cp->m_thrust = (cp->m_thrust * 7400) / 7000;
			else
				cp->m_thrust = (cp->m_thrust * 6700) / 7000;
		}

		if (/*cp->m_hndType == 0 && */cp->m_hd.changingGear != 0)
			cp->m_thrust = 1;
	}
}

//-------------------------------------------------------------

CPlayer	CManager_Players::LocalPlayer;
Array<CPlayer*>	CManager_Players::Players;

void CManager_Players::Lua_Init(sol::state& lua)
{
	CPlayer::Lua_Init(lua);

	auto engine = lua["engine"].get_or_create<sol::table>();
	LUADOC_GLOBAL();
	{
		LUADOC_TYPE("Players");
		auto players = engine["Players"].get_or_create<sol::table>();

		players[LUADOC_P("localPlayer", "<Player> - current player")]
			= &LocalPlayer;

		players[LUADOC_M("GetPlayerByCar", "(car: CCar) : Player")]
			= &GetPlayerByCar;

		players[LUADOC_M("CreatePlayer", "() : Player")]
			= &CreatePlayer;

		players[LUADOC_M("RemovePlayer", "(Player)")]
			= &RemovePlayer;

		players[LUADOC_M("RemoveAllPlayers", "(void)")]
			= &RemoveAllPlayers;

		players[LUADOC_M("Update", "()")]
			= &Update;
	}

	// make local player default
	Players.append(&LocalPlayer);
}

void CManager_Players::Update()
{
	// TODO: network update
}

CPlayer* CManager_Players::GetLocalPlayer()
{
	return &LocalPlayer;
}

CPlayer* CManager_Players::GetPlayerByCar(CCar* car)
{
	for (int i = 0; i < Players.numElem(); ++i)
	{
		if (Players[i]->GetCurrentCar() == car)
			return Players[i];
	}

	return nullptr;
}

CPlayer* CManager_Players::CreatePlayer()
{
	CPlayer* newPlayer = new CPlayer();
	Players.append(newPlayer);

	return newPlayer;
}

void CManager_Players::RemovePlayer(CPlayer* player)
{
	const int foundIdx = arrayFindIndex(Players, player);
	if (foundIdx != -1)
	{
		Players.fastRemoveIndex(foundIdx);
		delete player;
	}
}

void CManager_Players::RemoveAllPlayers()
{
	for (int i = 0; i < Players.numElem(); ++i)
	{
		if(Players[i] != &LocalPlayer)
			delete Players[i];
	}
	Players.clear();
	Players.append(&LocalPlayer);
}

void CManager_Players::Net_Init()
{
	LocalPlayer.Net_Init();
}

void CManager_Players::Net_Finalize()
{
	LocalPlayer.Net_Finalize();

	// TODO: disconnect all other peers
}

void CManager_Players::Net_Update()
{

}
#include "game/pch.h"
#include "players.h"

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
			LUADOC_P("horn"), &InputData::horn
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<CPlayer>(
			LUADOC_T("CPlayer"),

			LUADOC_P("currentCar", "get/set player control buttons"), 
			sol::property(&CPlayer::GetCurrentCar, &CPlayer::SetCurrentCar),

			LUADOC_P("controlType"), 
			sol::property(&CPlayer::m_controlType),

			LUADOC_P("input", "get/set player control buttons"), 
			sol::property(&CPlayer::m_currentInputs, &CPlayer::UpdateControls)
		);
	}
}

void CPlayer::Net_Init()
{

}

void CPlayer::Net_Finalize()
{

}

EPlayerControlType CPlayer::GetControlType() const
{
	return m_controlType;
}

void CPlayer::SetCurrentCar(CCar* newCar)
{
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
	bool use_analogue = false; // TODO: from InputData
	int analog_angle;
	//PED_MODEL_TYPES whoExit;
	//whoExit = TANNER_MODEL;

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
#if 0 // TODO: player handler for rubberbanding
		if (cp->m_hndType == 5)
		{
			// rubber band freeroamer.
			// accelerate faster if closer to player
			int dx, dz, dist;

			dx = car_data[player[0].playerCarId].hd.where.t[0] - cp->hd.where.t[0] >> 10;
			dz = car_data[player[0].playerCarId].hd.where.t[2] - cp->hd.where.t[2] >> 10;

			dist = dx * dx + dz * dz;

			if (dist > 40)
				cp->thrust = 3000;
			else if (dist > 20)
				cp->thrust = 4000;
			else if (dist > 9)
				cp->thrust = 4900;
			else
				cp->thrust = 6000;
		}
		else
#endif
		{
			cp->m_thrust = FIXEDH(cp->m_cosmetics.powerRatio * 4915);
		}

#if 0
		if (cp->m_controlType == CONTROL_TYPE_PLAYER)
		{
			CAR_DATA* tp;
			int targetCarId, cx, cz, chase_square_dist;

			if (player[0].playerCarId == cp->id)
				targetCarId = player[0].targetCarId;
			else if (player[1].playerCarId == cp->id)
				targetCarId = player[1].targetCarId;
			else
				targetCarId = -1;

			// apply rubber banding to player car depending on distance from target car
			if (targetCarId != -1)
			{
				tp = &car_data[targetCarId];

				if (3050 < cp->ap.carCos->powerRatio)
					cp->thrust = FIXEDH(tp->ap.carCos->powerRatio * 4915);

				cx = cp->hd.where.t[0] - tp->hd.where.t[0] >> 10;
				cz = cp->hd.where.t[2] - tp->hd.where.t[2] >> 10;

				chase_square_dist = cx * cx + cz * cz;

				if (chase_square_dist > 20)
					cp->thrust = (cp->thrust * 8000) / 7000;
				else if (chase_square_dist > 6)
					cp->thrust = (cp->thrust * 7400) / 7000;
				else
					cp->thrust = (cp->thrust * 6700) / 7000;
			}
		}
#endif

		if (/*cp->m_hndType == 0 && */cp->m_hd.changingGear != 0)
			cp->m_thrust = 1;
	}
}

//-------------------------------------------------------------

CPlayer	CManager_Players::LocalPlayer;

void CManager_Players::Lua_Init(sol::state& lua)
{
	CPlayer::Lua_Init(lua);

	auto engine = lua["engine"].get_or_create<sol::table>();

	{
		LUADOC_NAMESPACE("Players");
		auto world = engine["Players"].get_or_create<sol::table>();
		world["localPlayer"] = &LocalPlayer;
	}
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
	if (LocalPlayer.GetCurrentCar() == car)
		return GetLocalPlayer();

	return nullptr;
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
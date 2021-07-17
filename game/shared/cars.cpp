#include <nstd/String.hpp>
#include <nstd/Array.hpp>

#include "math/psx_math_types.h"
#include "math/isin.h"
#include "math/ratan2.h"

#include "routines/d2_types.h"
#include "renderer/gl_renderer.h"

#include "core/cmdlib.h"

#include <stdlib.h>

#include "world.h"
#include "cars.h"

struct GEAR_DESC
{
	int lowidl_ws, low_ws, hi_ws;
	int ratio_ac, ratio_id;
};

struct HANDLING_TYPE
{
	int8 frictionScaleRatio, aggressiveBraking, fourWheelDrive, autoBrakeOn;
};

const HANDLING_TYPE handlingType[7] =
{
	// frictionScaleRatio, aggressiveBraking, fourWheelDrive, autoBrakeOn
	{ 32, 1, 0, 1 },
	{ 29, 0, 0, 0 },
	{ 45, 1, 1, 0 },
	{ 55, 1, 1, 0 },
	{ 68, 1, 1, 0 },
	{ 32, 1, 0, 1 },
	{ 29, 0, 0, 0 }
};

// gear ratios for different handling types
static const GEAR_DESC s_gearDesc[2][4] =
{
	{
		{ 0, 0, 163, 144, 135 },
		{ 86, 133, 260, 90, 85 },
		{ 186, 233, 360, 60, 57 },
		{ 286, 326, 9999, 48, 45 }
	},
	{
		{ 0, 0, 50, 144, 135 },
		{ 43, 66, 100, 90, 85 },
		{ 93, 116, 150, 60, 57 },
		{ 143, 163, 9999, 48, 45 }
	}
};

int gCopDifficultyLevel = 0;		// TODO: Lua parameter on difficulty?
int wetness = 0;					// TODO: CWorld::GetWetness()

// FIXME: dummy! Replace with GTE replacements!
#define gte_ldv0(x)
#define gte_rtv0tr()
#define gte_stlvnl(x)
#define gte_ldlvl(x)
#define gte_rtir()
#define gte_stsv(x)
#define gte_SetRotMatrix(m)
#define gte_SetTransMatrix(m)


void CCar::AddWheelForcesDriver1(CAR_LOCALS& cl)
{
	int oldCompression, newCompression;
	int dir;
	int forcefac;
	int angle;
	int lfx, lfz;
	int sidevel, slidevel;
	int susForce;
	int chan;
	WHEEL* wheel;
	int friction_coef;
	int oldSpeed;
	int wheelspd;
	LONGVECTOR4 wheelPos;
	LONGVECTOR4 surfacePoint;
	LONGVECTOR4 surfaceNormal;
	VECTOR_NOPAD force;
	LONGVECTOR4 pointVel;
	int frontFS;
	int rearFS;
	sdPlane* SurfacePtr;
	int i;
	int cdx, cdz;
	int sdx, sdz;
	CAR_COSMETICS* car_cos;
	int player_id;

	oldSpeed = m_hd.speed * 3 >> 1;

	if (oldSpeed < 32)
		oldSpeed = oldSpeed * -72 + 3696;
	else
		oldSpeed = 1424 - oldSpeed;

	dir = m_hd.direction;
	cdx = isin(dir);
	cdz = icos(dir);

	dir += m_wheel_angle;
	sdx = isin(dir);
	sdz = icos(dir);

	//player_id = GetPlayerId(cp);
	car_cos = m_ap.carCos;

	GetFrictionScalesDriver1(cl, frontFS, rearFS);

	m_hd.front_vel = 0;
	m_hd.rear_vel = 0;

	if (oldSpeed > 3300)
		oldSpeed = 3300;

	i = 3;
	wheel = m_hd.wheel + 3;
	do {
		gte_ldv0(&car_cos->wheelDisp[i]);

		gte_rtv0tr();
		gte_stlvnl(wheelPos);

		newCompression = CWorld::FindSurface(*(VECTOR_NOPAD*)&wheelPos, *(VECTOR_NOPAD*)&surfaceNormal, *(VECTOR_NOPAD*)&surfacePoint, &SurfacePtr);

		friction_coef = (newCompression * (32400 - wetness) >> 15) + 500;

		if (SurfacePtr != nullptr)
			wheel->onGrass = SurfacePtr->surfaceType == 4;
		else
			wheel->onGrass = 0;

		if (SurfacePtr)
		{
			switch (SurfacePtr->surfaceType)
			{
				case 4:
				case 6:
				case 9:
				case 11:
					wheel->surface = 0x80;
					break;
				default:
					wheel->surface = 0;
			}

			// [A] indication of Event surface which means we can't add tyre tracks for that wheel
			if (SurfacePtr->surfaceType - 16U < 16)
				wheel->surface |= 0x8;

			switch (SurfacePtr->surfaceType)
			{
				case 8:
					wheel->surface |= 0x2;
					break;
				case 6:
				case 9:
					wheel->surface |= 0x1;
					break;
				case 11:
					wheel->surface |= 0x3;
					break;
			}
		}
		else
		{
			wheel->surface = 0;
		}

		oldCompression = wheel->susCompression;
		newCompression = FIXEDH((surfacePoint[1] - wheelPos[1]) * surfaceNormal[1]) + 14;

		if (newCompression < 0)
			newCompression = 0;

		if (newCompression > 800)
			newCompression = 12;
#if 0
		// play wheel collision sound
		// and apply vibration to player controller
		if (controlType == CONTROL_TYPE_PLAYER)
		{
			if (ABS(newCompression - oldCompression) > 12 && (i & 1U) != 0)
			{
				chan = GetFreeChannel(0);
				if (chan > -1)
				{
					if (NumPlayers > 1 && NoPlayerControl == 0)
						SetPlayerOwnsChannel(chan, player_id);

					Start3DSoundVolPitch(chan, SOUND_BANK_SFX, 1, hd.where.t[0], hd.where.t[1], hd.where.t[2], -2500, 400);
					SetChannelPosition3(chan, (VECTOR*)hd.where.t, NULL, -2500, 400, 0);
				}
			}

			if (newCompression >= 65)
				SetPadVibration(*ai.padid, 1);
			else if (newCompression >= 35)
				SetPadVibration(*ai.padid, 2);
			else if (newCompression > 25)
				SetPadVibration(*ai.padid, 3);
		}
#endif

		if (newCompression > 42)
			newCompression = 42;

		if (newCompression == 0 && oldCompression == 0)
		{
			wheel->susCompression = 0;
		}
		else
		{
			wheelPos[2] = wheelPos[2] - m_hd.where.t[2];
			wheelPos[1] = wheelPos[1] - m_hd.where.t[1];
			wheelPos[0] = wheelPos[0] - m_hd.where.t[0];

			force.vz = 0;
			force.vx = 0;

			pointVel[0] = FIXEDH(cl.avel[1] * wheelPos[2] - cl.avel[2] * wheelPos[1]) + cl.vel[0];
			pointVel[2] = FIXEDH(cl.avel[0] * wheelPos[1] - cl.avel[1] * wheelPos[0]) + cl.vel[2];

			// that's our spring
			susForce = newCompression * 230 - oldCompression * 100;

			if (wheel->locked)
			{
				dir = ratan2(pointVel[0] >> 6, pointVel[2] >> 6);

				lfx = isin(dir);
				lfz = icos(dir);

				if (abs(pointVel[0]) + abs(pointVel[2]) < 8000)
				{
					surfaceNormal[0] = 0;
					surfaceNormal[1] = ONE;
					surfaceNormal[2] = 0;
				}
			}
			else
			{
				if (i & 1)
				{
					lfz = -cdx;
					lfx = cdz;
				}
				else
				{
					lfz = -sdx;
					lfx = sdz;
				}
			}

			slidevel = (pointVel[0] / 64) * (lfx / 64) + (pointVel[2] / 64) * (lfz / 64);
			wheelspd = abs((oldSpeed / 64) * (slidevel / 64));

			if (slidevel > 50000)
			{
				slidevel = 12500;
			}
			else if (slidevel < -50000)
			{
				slidevel = -12500;
			}
			else
			{
				slidevel = FIXEDH(oldSpeed * slidevel);

				if (slidevel > 12500)
					slidevel = 12500;

				if (slidevel < -12500)
					slidevel = -12500;
			}

			if ((i & 1U) != 0)
			{
				// rear wheels
				if (wheel->locked == 0)
				{
					sidevel = FIXEDH(rearFS * slidevel);

					if (handlingType[m_hndType].autoBrakeOn != 0 && 0 < sidevel * m_wheel_angle)
						m_hd.autoBrake = -1;

					force.vx = -lfz * m_thrust;
					force.vz = lfx * m_thrust;
				}
				else
				{
					sidevel = FixHalfRound(frontFS * slidevel, 14);
				}

				if (m_hd.rear_vel < wheelspd)
					m_hd.rear_vel = wheelspd;
			}
			else
			{
				// front wheels
				sidevel = frontFS * slidevel + 2048 >> 12;

				if (wheel->locked)
				{
					sidevel = (frontFS * slidevel + 2048 >> 13) + sidevel >> 1;

					forcefac = FixHalfRound(FIXEDH(-sidevel * lfx) * sdz - FIXEDH(-sidevel * lfz) * sdx, 11);

					force.vx = forcefac * sdz;
					force.vz = -forcefac * sdx;
				}
				else
				{
					if (m_controlType == CONTROL_TYPE_PURSUER_AI)
					{
						force.vx = sdx * m_thrust;
						force.vz = sdz * m_thrust;
					}
				}

				if (m_hd.front_vel < wheelspd)
					m_hd.front_vel = wheelspd;

			}

			force.vx += (susForce * surfaceNormal[0] - sidevel * lfx) - cl.vel[0] * 12;
			force.vz += (susForce * surfaceNormal[2] - sidevel * lfz) - cl.vel[2] * 12;

			// apply speed reduction by water
			if ((wheel->surface & 7) == 1)
			{
				force.vx -= cl.vel[0] * 75;
				force.vz -= cl.vel[2] * 75;
			}

			angle = m_hd.where.m[1][1];

			if (angle < 2048)
			{
				angle = 4096 - angle;

				if (angle <= 4096)
					angle = 4096 - FIXEDH(angle * angle);
				else
					angle = 0;

				friction_coef = FIXEDH(friction_coef * angle);
			}

			if (surfaceNormal[1] < 3276)
				friction_coef = friction_coef * surfaceNormal[1] * 5 >> 0xe;

			force.vy = FIXEDH(susForce * surfaceNormal[1] - cl.vel[1] * 12);
			force.vx = FIXEDH(force.vx) * friction_coef >> 0xc;
			force.vz = FIXEDH(force.vz) * friction_coef >> 0xc;

			// pursuer cars have more stability
			if (m_controlType == CONTROL_TYPE_PURSUER_AI)
			{
				if (gCopDifficultyLevel == 2)
					wheelPos[1] = (wheelPos[1] * 12) / 32;
				else
					wheelPos[1] = (wheelPos[1] * 19) / 32;
			}

			m_hd.acc[0] += force.vx;
			m_hd.acc[1] += force.vy;
			m_hd.acc[2] += force.vz;

			m_hd.aacc[0] += FIXEDH(wheelPos[1] * force.vz - wheelPos[2] * force.vy);
			m_hd.aacc[1] += FIXEDH(wheelPos[2] * force.vx - wheelPos[0] * force.vz);
			m_hd.aacc[2] += FIXEDH(wheelPos[0] * force.vy - wheelPos[1] * force.vx);

			wheel->susCompression = newCompression;
		}
		wheel--;
		i--;
	} while (i >= 0);

	if (m_hd.wheel[1].susCompression == 0 && m_hd.wheel[3].susCompression == 0)
	{
		if (m_thrust >= 1)
			m_hd.wheel_speed = 1703936 + 0x4000;
		else if (m_thrust <= -1)
			m_hd.wheel_speed = -1245184 + 0x4000;
		else
			m_hd.wheel_speed = 0;
	}
	else
	{
		m_hd.wheel_speed = cdz / 64 * (cl.vel[2] / 64) + cdx / 64 * (cl.vel[0] / 64);
	}
}

void CCar::GetFrictionScalesDriver1(CAR_LOCALS& cl, int& frontFS, int& rearFS)
{
	int autoBrake;
	int q;
	const HANDLING_TYPE* hp;

	hp = &handlingType[m_hndType];

	if (m_thrust < 0)
		frontFS = 1453;
	else if (m_thrust < 1)
		frontFS = 937;
	else
		frontFS = 820;

	autoBrake = m_hd.autoBrake;

	if (m_wheelspin == 0 && hp->autoBrakeOn != 0 && autoBrake > 0 && m_hd.wheel_speed > 0)
	{
		q = autoBrake << 1;

		if (autoBrake > 13)
		{
			autoBrake = 13;
			q = 26;
		}

		frontFS += (q + autoBrake) * 15;

		if(hp->autoBrakeOn == 2)
			MsgError("invalid autoBrakeOn");
	}

	if ((m_thrust < 0 && m_hd.wheel_speed > 41943 && m_hndType == 0) ||
		(m_controlType == CONTROL_TYPE_CIV_AI && m_ai.c.thrustState == 3 && m_ai.c.ctrlState != 9))
	{
		m_hd.wheel[3].locked = 1;
		m_hd.wheel[2].locked = 1;
		m_hd.wheel[1].locked = 1;
		m_hd.wheel[0].locked = 1;
	}
	else
	{
		m_hd.wheel[3].locked = 0;
		m_hd.wheel[2].locked = 0;
		m_hd.wheel[1].locked = 0;
		m_hd.wheel[0].locked = 0;
	}

	if (m_handbrake == 0)
	{
		if (m_wheelspin != 0)
			frontFS += 600;
	}
	else
	{
		if (m_thrust > -1)
			m_thrust = 0;

		if (m_hd.wheel_speed < 1)
			frontFS -= 375;
		else
			frontFS += 656;

		m_hd.wheel[1].locked = 1;
		m_hd.wheel[3].locked = 1;
		m_wheelspin = 0;
	}

	if (m_hd.wheel_speed < 0 && m_thrust > -1 && m_handbrake == 0)
	{
		frontFS -= 400;
	}

	rearFS = 1920 - frontFS;

	if (m_wheelspin != 0)
	{
		m_thrust = FIXEDH(m_ap.carCos->powerRatio * 5000);
	}

	if (m_thrust < 0 && m_hd.wheel_speed > 41943 && cl.aggressive != 0)
	{
		frontFS = (frontFS * 10) / 8;
		rearFS = (rearFS * 10) / 8;
	}
	else
	{
		if (m_hd.wheel[0].onGrass == 0)
			frontFS = (frontFS * 36 - frontFS) / 32;
		else
			frontFS = (frontFS * 40 - frontFS) / 32;
	}

	frontFS = (frontFS * hp->frictionScaleRatio) / 32;
	rearFS = (rearFS * hp->frictionScaleRatio) / 32;

	if ((m_hndType == 5) && (m_ai.l.dstate == 5))
	{
		frontFS = (frontFS * 3) / 2;
		rearFS = (rearFS * 3) / 2;
	}

	int traction = m_ap.carCos->traction;

	if (traction != 4096)
	{
		frontFS = FIXEDH(frontFS * traction);
		rearFS = FIXEDH(rearFS * traction);
	}
}

void CCar::ConvertTorqueToAngularAcceleration(CAR_LOCALS& cl)
{
	int twistY, twistZ;
	int zd;
	int i;

	CAR_COSMETICS* car_cos = m_ap.carCos;

	twistY = car_cos->twistRateY;
	twistZ = car_cos->twistRateZ;

	zd = FIXEDH(m_hd.where.m[0][2] * m_hd.aacc[0] + m_hd.where.m[1][2] * m_hd.aacc[1] + m_hd.where.m[2][2] * m_hd.aacc[2]);

	for (i = 0; i < 3; i++)
	{
		m_hd.aacc[i] = m_hd.aacc[i] * twistY + FIXEDH(m_hd.where.m[i][2] * (twistZ - twistY) * zd - cl.avel[i] * 128);

		if (cl.extraangulardamping == 1)
			m_hd.aacc[i] -= cl.avel[i] / 8;
	}
}

void CCar::StepOneCar()
{
	static int frictionLimit[6] = {
		1005 * ONE, 6030 * ONE,
		1005 * ONE, 5025 * ONE,
		1884 * ONE, 5025 * ONE
	};

	volatile int impulse;
	CAR_COSMETICS* car_cos;
	int friToUse;
	int lift;
	int a, b, speed;
	int count, i;
	CAR_LOCALS _cl;
	LONGVECTOR4 deepestNormal, deepestLever, deepestPoint;
	LONGVECTOR4 pointPos, surfacePoint, surfaceNormal;
	LONGVECTOR4 lever, reaction;
	VECTOR_NOPAD direction;
	sdPlane* SurfacePtr;

	// FIXME: redundant?
	// if (m_controlType == CONTROL_TYPE_NONE)
	// 	return;

	SurfacePtr = NULL;
	_cl.aggressive = handlingType[m_hndType].aggressiveBraking;
	_cl.extraangulardamping = 0;

	for (i = 0; i < 3; i++)
	{
		_cl.vel[i] = m_st.n.linearVelocity[i];
		_cl.avel[i] = m_st.n.angularVelocity[i];

		m_st.n.fposition[i] = (m_st.n.fposition[i] & 0xF) + m_hd.where.t[i] * 16;
	}

	m_hd.acc[0] = 0;
	m_hd.acc[1] = -7456; // apply gravity
	m_hd.acc[2] = 0;

	// calculate car speed
	a = abs(FIXEDH(_cl.vel[0]));
	b = abs(FIXEDH(_cl.vel[2]));

	if (a < b)
		speed = b + a / 2;
	else
		speed = a + b / 2;

	m_hd.speed = speed;

	car_cos = m_ap.carCos;
	lift = 0;

	gte_SetRotMatrix(&m_hd.where);
	gte_SetTransMatrix(&m_hd.where);

	count = 12;

	if (m_hd.where.m[1][1] > 2048)
	{
		if (m_controlType == CONTROL_TYPE_CIV_AI)
			count = (m_totalDamage != 0) * 4;
		else
			count = 4;
	}

	count--;

	// calculate lifting factor
	while (count >= 0)
	{
		gte_ldv0(&car_cos->cPoints[count]);

		gte_rtv0tr();

		gte_stlvnl(pointPos);

		lever[0] = pointPos[0] - m_hd.where.t[0];
		lever[1] = pointPos[1] - m_hd.where.t[1];
		lever[2] = pointPos[2] - m_hd.where.t[2];

		CWorld::FindSurface(*(VECTOR_NOPAD*)&pointPos, *(VECTOR_NOPAD*)&surfaceNormal, *(VECTOR_NOPAD*)&surfacePoint, &SurfacePtr);

		if ((surfacePoint[1] - pointPos[1]) - 1U < 799)
		{
			int newLift;

			newLift = FIXEDH((surfacePoint[1] - pointPos[1]) * surfaceNormal[1]);

			if (lift < newLift)
			{
				friToUse = 0;

				deepestNormal[0] = surfaceNormal[0];
				deepestNormal[1] = surfaceNormal[1];
				deepestNormal[2] = surfaceNormal[2];

				deepestLever[0] = lever[0];
				deepestLever[1] = lever[1];
				deepestLever[2] = lever[2];

				deepestPoint[0] = surfacePoint[0];
				deepestPoint[1] = surfacePoint[1];
				deepestPoint[2] = surfacePoint[2];

				lift = newLift;

				if (count > 3)
					friToUse = 3;
			}
		}

		count--;
	}

	// do lifting
	if (lift != 0)
	{
		int strikeVel;
		int componant;

		int lever_dot_n;
		int twistY;
		int displacementsquared;

		lever[0] = FIXEDH(_cl.avel[1] * deepestLever[2] - _cl.avel[2] * deepestLever[1]) + _cl.vel[0];
		lever[1] = FIXEDH(_cl.avel[2] * deepestLever[0] - _cl.avel[0] * deepestLever[2]) + _cl.vel[1];
		lever[2] = FIXEDH(_cl.avel[0] * deepestLever[1] - _cl.avel[1] * deepestLever[0]) + _cl.vel[2];

		twistY = car_cos->twistRateY;

		lever_dot_n = FIXEDH(deepestLever[0] * deepestNormal[0] + deepestLever[1] * deepestNormal[1] + deepestLever[2] * deepestNormal[2]);
		displacementsquared = FIXEDH(((deepestLever[0] * deepestLever[0] + deepestLever[1] * deepestLever[1] + deepestLever[2] * deepestLever[2]) - lever_dot_n * lever_dot_n) * twistY) + 4096;

		strikeVel = (lever[0] >> 6) * (deepestNormal[0] >> 6) + (lever[1] >> 6) * (deepestNormal[1] >> 6) + (lever[2] >> 6) * (deepestNormal[2] >> 6);
		impulse = (strikeVel / displacementsquared) * -2048;

		// apply friction
		componant = 2;
		do {
			int loss;
			int limit;

			limit = frictionLimit[friToUse + 2 - componant];
			loss = lever[componant] * 67;

			if (loss <= limit)
			{
				if (loss < -limit)
					limit = -limit;
				else
					limit = loss;
			}

			reaction[componant] = FIXEDH(impulse * deepestNormal[componant] - limit);
			componant--;
		} while (componant >= 0);

#if 0
		if (impulse > 20000)
		{
			if (gNight == 1)
			{
				direction.vx = 0;
				direction.vy = 50;
				direction.vz = 0;

				Setup_Sparks((VECTOR*)&deepestPoint, &direction, 15, 1);
			}
			else
			{
				direction.vx = 0;
				direction.vy = 40;
				direction.vz = 0;

				Setup_Debris((VECTOR*)&deepestPoint, &direction, 10, 0);
			}

			if (SurfacePtr && (SurfacePtr->surface != 9) && (SurfacePtr->surface != 6))
			{
				CollisionSound(GetPlayerId(cp), cp, (impulse / 6 + (impulse >> 0x1f) >> 3) - (impulse >> 0x1f), 0);
			}
		}
#endif

		m_hd.acc[0] += reaction[0];
		m_hd.acc[1] += reaction[1];
		m_hd.acc[2] += reaction[2];

		m_hd.aacc[0] += FIXEDH(deepestLever[1] * reaction[2] - deepestLever[2] * reaction[1]);
		m_hd.aacc[1] += FIXEDH(deepestLever[2] * reaction[0] - deepestLever[0] * reaction[2]);
		m_hd.aacc[2] += FIXEDH(deepestLever[0] * reaction[1] - deepestLever[1] * reaction[0]);

		//if (lift != 0)
		{
			lever[0] = FIXEDH(lift * deepestNormal[0]);
			lever[1] = FIXEDH(lift * deepestNormal[1]);
			lever[2] = FIXEDH(lift * deepestNormal[2]);

			m_hd.where.t[0] += lever[0];
			m_hd.where.t[1] += lever[1];
			m_hd.where.t[2] += lever[2];

			m_st.n.fposition[0] += lever[0] * 16;
			m_st.n.fposition[1] += lever[1] * 16;
			m_st.n.fposition[2] += lever[2] * 16;

			gte_SetTransMatrix(&m_hd.where);

			_cl.extraangulardamping = 1;

			if (lift > 120)
				m_st.n.linearVelocity[1] = 0;
		}
	}

	AddWheelForcesDriver1(_cl);
	ConvertTorqueToAngularAcceleration(_cl);

	m_hd.mayBeColliding = 0;
}

//-----------------------------------------------------

uint16 CCar::GetEngineRevs()
{
	int acc;
	const GEAR_DESC* gd;
	int gear;
	int lastgear;
	int ws, lws;
	int type;

	gear = m_hd.gear;
	ws = m_hd.wheel_speed;
	acc = m_thrust;
	type = (m_controlType == CONTROL_TYPE_CIV_AI);

	if (ws > 0)
	{
		ws >>= 11;

		if (gear > 3)
			gear = 3;

		gd = &s_gearDesc[type][gear];

		do {
			if (acc < 1)
				lws = gd->lowidl_ws;
			else
				lws = gd->low_ws;

			lastgear = gear;

			if (ws < lws)
			{
				gd--;
				lastgear = gear - 1;
			}

			if (gd->hi_ws < ws)
			{
				gd++;
				lastgear++;
			}

			if (gear == lastgear)
				break;

			gear = lastgear;

		} while (true);

		m_hd.gear = lastgear;
	}
	else
	{
		ws = -ws / 2048;
		lastgear = 0;

		m_hd.gear = 0;
	}

	if (acc != 0)
		return ws * s_gearDesc[type][lastgear].ratio_ac;

	return ws * s_gearDesc[type][lastgear].ratio_id;
}

void CCar::ControlCarRevs()
{
	const int MaxRevDrop = 1440;
	const int MaxRevRise = 1600;

	char spin;
	int player_id, acc, oldvol;
	short oldRevs, newRevs, desiredRevs;

	acc = m_thrust;
	spin = m_wheelspin;
	oldRevs = m_hd.revs;
	player_id = 0;// GetPlayerId(cp);

	m_hd.changingGear = 0;

	if (spin == 0 && (m_hd.wheel[1].susCompression || m_hd.wheel[3].susCompression || acc == 0))
	{
		desiredRevs = GetEngineRevs();
	}
	else
	{
		desiredRevs = 20160;

		if (m_hd.wheel[1].susCompression == 0 && m_hd.wheel[3].susCompression == 0)
		{
			desiredRevs = 30719;
			spin = 1;
		}

		if (oldRevs < 8000)
			oldRevs = 8000;

		m_hd.gear = 0;
	}

	newRevs = desiredRevs;
	desiredRevs = (oldRevs - newRevs);

	if (MaxRevDrop < desiredRevs)
	{
		acc = 0;
		m_hd.changingGear = 1;
		newRevs = oldRevs - MaxRevDrop;
	}

	desiredRevs = newRevs - oldRevs;

	if (MaxRevRise < desiredRevs)
		newRevs = oldRevs + MaxRevRise;

	m_hd.revs = newRevs;

#if 0
	// TODO: CPlayerManager:GetPlayer()
	if (player_id != -1)
	{
		if (acc == 0 && newRevs < 7001)
		{
			acc = player[player_id].revsvol;

			player[player_id].idlevol += 200;
			player[player_id].revsvol = acc - 200;

			if (player[player_id].idlevol > -6000)
				player[player_id].idlevol = -6000;

			if (player[player_id].revsvol < -10000)
				player[player_id].revsvol = -10000;
		}
		else
		{
			int revsmax;

			if (acc != 0)
				revsmax = -5500;
			else
				revsmax = -6750;

			if (spin == 0)
				acc = -64;
			else
				acc = -256;

			player[player_id].idlevol += acc;

			if (spin == 0)
				acc = 175;
			else
				acc = 700;

			player[player_id].revsvol = player[player_id].revsvol + acc;

			if (player[player_id].idlevol < -10000)
				player[player_id].idlevol = -10000;

			if (player[player_id].revsvol > revsmax)
				player[player_id].revsvol = revsmax;
		}
	}
#endif
}

//---------------------------------------------------

void CCar::InitOrientedBox()
{
	SVECTOR boxDisp;
	CAR_COSMETICS* car_cos;

	short length;

	gte_SetRotMatrix(&m_hd.where);
	gte_SetTransMatrix(&m_hd.where);

	car_cos = m_ap.carCos;

	boxDisp.vx = -car_cos->cog.vx;
	boxDisp.vy = -car_cos->cog.vy;
	boxDisp.vz = -car_cos->cog.vz;

	gte_ldv0(&boxDisp);
	gte_rtv0tr();

	if (m_controlType == CONTROL_TYPE_PURSUER_AI)
	{
		length = (car_cos->colBox.vx * 14) / 16;
		m_hd.oBox.length[0] = length;
	}
	else
	{
		length = car_cos->colBox.vx;
		m_hd.oBox.length[0] = length;
	}

	gte_stlvnl(&m_hd.oBox.location);

	VECTOR_NOPAD svx = { length, 0 ,0 };
	VECTOR_NOPAD svy = { 0, car_cos->colBox.vy ,0 };
	VECTOR_NOPAD svz = { 0, 0 ,car_cos->colBox.vz };

	gte_ldlvl(&svx);

	gte_rtir();
	m_hd.oBox.length[1] = car_cos->colBox.vy;
	gte_stsv(&m_hd.oBox.radii[0]);

	gte_ldlvl(&svy);
	gte_rtir();
	m_hd.oBox.length[2] = car_cos->colBox.vz;
	gte_stsv(&m_hd.oBox.radii[1]);

	gte_ldlvl(&svz);
	gte_rtir();
	gte_stsv(&m_hd.oBox.radii[2]);
}

void LongQuaternion2Matrix(LONGQUATERNION* qua, MATRIX* m)
{
	int qx = (*qua)[0];
	int qy = (*qua)[1];
	int qz = (*qua)[2];
	int qw = (*qua)[3];

	int yy = FixHalfRound(qy * qy, 11);
	int zz = FixHalfRound(qz * qz, 11);
	int xx = FixHalfRound(qx * qx, 11);
	int zw = FixHalfRound(qz * qw, 11);
	int xy = FixHalfRound(qx * qy, 11);
	int xz = FixHalfRound(qx * qz, 11);
	int yw = FixHalfRound(qy * qw, 11);
	int xw = FixHalfRound(qx * qw, 11);
	int yz = FixHalfRound(qy * qz, 11);

	m->m[0][0] = ONE - (yy + zz);
	m->m[1][1] = ONE - (xx + zz);
	m->m[2][2] = ONE - (xx + yy);
	m->m[0][1] = xy - zw;
	m->m[0][2] = xz + yw;
	m->m[1][0] = xy + zw;
	m->m[2][0] = xz - yw;
	m->m[1][2] = yz - xw;
	m->m[2][1] = yz + xw;
}

void CCar::RebuildCarMatrix(RigidBodyState& st)
{
	int sm, osm;
	int qw, qz, qy, qx;

	m_hd.where.t[0] = st.n.fposition[0] >> 4;
	m_hd.where.t[1] = st.n.fposition[1] >> 4;
	m_hd.where.t[2] = st.n.fposition[2] >> 4;

	qx = st.n.orientation[0];
	qy = st.n.orientation[1];
	qz = st.n.orientation[2];
	qw = st.n.orientation[3];

	osm = qx * qx + qy * qy + qz * qz + qw * qw;
	sm = 4096;

	if (osm < 1024)
	{
		st.n.orientation[2] = 0;
		st.n.orientation[1] = 0;
		st.n.orientation[0] = 0;
	}
	else
	{
		sm = 6144 - (osm >> 13);

		st.n.orientation[0] = FIXEDH(sm * qx);
		st.n.orientation[1] = FIXEDH(sm * qy);
		st.n.orientation[2] = FIXEDH(sm * qz);

		sm = FIXEDH(sm * qw);
	}
	st.n.orientation[3] = sm;

	LongQuaternion2Matrix(&st.n.orientation, &m_hd.where);

	InitOrientedBox();
}


void CCar::CheckCarEffects()
{
	// Dummy for now...
	JumpDebris();

	// update skidding sound

	// update tyre tracks

	// update hubcaps release
}

void CCar::JumpDebris()
{
	WHEEL* wheel;
	int count;
	VECTOR_NOPAD position;
	VECTOR_NOPAD velocity;

	wheel = m_hd.wheel;

	for (count = 0; count < 4; count++)
	{
		if (wheel->susCompression != 0)
		{
			//DebrisTimer = 0;
			m_wasOnGround = 1;
			return;
		}

		wheel++;
	}

	if (m_wasOnGround == 1)
	{
		m_wasOnGround = 0;
		//DebrisTimer = 80;

		NoseDown();
	}

#if 0
	if (DebrisTimer != 0 && --DebrisTimer < 75)
	{
		memset((u_char*)&velocity, 0, sizeof(velocity));

		velocity.vx = m_hd.where.t[0] + ((rand() & 0x1ff) - 256);
		velocity.vy = 200 - m_hd.where.t[1];

		position.vz = m_hd.where.t[2] + ((rand() & 0x1ff) - 256);
		position.vx = velocity.vx;
		position.vy = velocity.vy;
		position.pad = velocity.pad;

		velocity.vz = position.vz;

		memset((u_char*)&velocity, 0, sizeof(velocity));
		Setup_Debris(&position, &velocity, 5, 0xb);
	}
#endif
}

void CCar::NoseDown()
{
	m_st.n.angularVelocity[0] += m_hd.where.m[0][0] * 50;
	m_st.n.angularVelocity[1] += m_hd.where.m[1][0] * 50;
	m_st.n.angularVelocity[2] += m_hd.where.m[2][0] * 50;
}

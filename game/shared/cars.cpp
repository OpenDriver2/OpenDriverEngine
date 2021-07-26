#include <nstd/String.hpp>
#include <nstd/Array.hpp>

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include "math/psx_math_types.h"
#include "math/psx_matrix.h"
#include "math/isin.h"
#include "math/ratan2.h"
#include "math/convert.h"
#include "math/Matrix.h"

#include "routines/d2_types.h"
#include "routines/models.h"

#include "renderer/gl_renderer.h"
#include "renderer/debug_overlay.h"

#include "game/render/render_model.h"
#include "manager_cars.h"

#include "core/cmdlib.h"

#include <stdlib.h>

#include "world.h"
#include "cars.h"
#include "bcollide.h"

extern CDriverLevelModels g_levModels;

const short DEFAULT_GRAVITY_FORCE = -7456; // D1 has -10922
const double Car_Fixed_Timestep = 1.0 / 30.0;

HANDLING_TYPE g_handlingType[7] =
{
	// frictionScaleRatio, aggressiveBraking, fourWheelDrive, autoBrakeOn
	{ 32,	true,		false,	1 },
	{ 29,	false,		false,	0 },
	{ 45,	false,		true,	0 },
	{ 55,	false,		true,	0 },
	{ 68,	false,		true,	0 },
	{ 32,	false,		false,	1 },
	{ 29,	false,		false,	0 }
};

// default gear ratios
static const GEAR_DESC s_gearDesc[][4] =
{
	// Driver 2 player car
	{
		{ 0, 0, 163, 144, 135 },
		{ 86, 133, 260, 90, 85 },
		{ 186, 233, 360, 60, 57 },
		{ 286, 326, 9999, 48, 45 }
	},
	// Driver 2 civ car
	{
		{ 0, 0, 50, 144, 135 },
		{ 43, 66, 100, 90, 85 },
		{ 93, 116, 150, 60, 57 },
		{ 143, 163, 9999, 48, 45 }
	},
	// Driver 1 cars
	{
		{ 0, 0, 245, 96, 90 },
		{ 130, 200, 390, 60, 57 },
		{ 130, 350, 540, 40, 38 },
		{ 130, 490, 9999, 32, 30}
	}
};

CarCosmetics::CarCosmetics()
{
	handlingType = g_handlingType[0];
	gears.append(s_gearDesc[0], 4);
	gravity = DEFAULT_GRAVITY_FORCE;
}

void CarCosmetics::InitFrom(const CAR_COSMETICS_D2& srcCos)
{
	headLight = srcCos.headLight;
	frontInd = srcCos.frontInd;
	backInd = srcCos.backInd;
	brakeLight = srcCos.brakeLight;
	revLight = srcCos.revLight;
	policeLight = srcCos.policeLight;
	exhaust = srcCos.exhaust;
	smoke = srcCos.smoke;
	fire = srcCos.fire;

	extraInfo = srcCos.extraInfo;
	powerRatio = srcCos.powerRatio;
	cbYoffset = srcCos.cbYoffset;
	susCoeff = srcCos.susCoeff;
	susCompressionLimit = 42;
	susTopLimit = 800;
	traction = srcCos.traction;
	wheelSize = srcCos.wheelSize;

	colBox = srcCos.colBox;
	cog = srcCos.cog;
	twistRateX = srcCos.twistRateX;
	twistRateY = srcCos.twistRateY;
	twistRateZ = srcCos.twistRateZ;
	mass = srcCos.mass;

	// FIXME: this might change if this become array!
	memcpy(cPoints, srcCos.cPoints, sizeof(cPoints));
	memcpy(wheelDisp, srcCos.wheelDisp, sizeof(wheelDisp));
}

int gCopDifficultyLevel = 0;		// TODO: Lua parameter on difficulty?
int wetness = 0;					// TODO: CWorld::GetWetness()

//--------------------------------------------------------

void CCar::Lua_Init(sol::state& lua)
{
	lua.new_usertype<GEAR_DESC>(
		"GEAR_DESC",
		sol::call_constructor, sol::factories(
			[](const sol::table& table) {
				return GEAR_DESC { 
					table["lowidl_ws"],
					table["low_ws"],
					table["hi_ws"],
					table["ratio_ac"],
					table["ratio_id"]
				};
			}),
			"lowidl_ws", &GEAR_DESC::lowidl_ws,
			"low_ws", &GEAR_DESC::low_ws,
			"hi_ws", &GEAR_DESC::hi_ws,
			"ratio_ac", &GEAR_DESC::ratio_ac,
			"ratio_id", &GEAR_DESC::ratio_id
		);

	lua.new_usertype<HANDLING_TYPE>(
		"HANDLING_TYPE",
		sol::call_constructor, sol::factories(
			[](const sol::table& table) {
				return HANDLING_TYPE{
					table["frictionScaleRatio"],
					table["aggressiveBraking"],
					table["fourWheelDrive"],
					table["autoBrakeOn"],
				};
			}),
			"frictionScaleRatio", &HANDLING_TYPE::frictionScaleRatio,
			"aggressiveBraking", &HANDLING_TYPE::aggressiveBraking,
			"fourWheelDrive", &HANDLING_TYPE::fourWheelDrive,
			"autoBrakeOn", &HANDLING_TYPE::autoBrakeOn
		);

	lua.new_usertype<CarCosmetics>(
		"CarCosmetics",
		sol::call_constructor, sol::factories(
			[](sol::table& table) {

				auto wheelDispTable = table["wheelDisp"].get_or_create<sol::table>();
				auto cPointsTable = table["cPoints"].get_or_create<sol::table>();
				auto gearsTable = table["gears"].get_or_create<sol::table>();

				CarCosmetics newCosmetics;

				if (!table.valid())
					return newCosmetics;

				if (!wheelDispTable.valid())
				{
					throw new sol::error("wheelDisp is null for CarCosmetics");
				}

				if (!cPointsTable.valid())
				{
					throw new sol::error("cPoints is null for CarCosmetics");
				}

				if (wheelDispTable.size() != 4)
				{
					throw new sol::error("wheelDisp count must be 4!");
				}

				if (cPointsTable.size() != 12)
				{
					throw new sol::error("cPoints count is not 12!");
				}

				if (gearsTable.valid() && gearsTable.size())
				{
					newCosmetics.gears.clear();
					for (int i = 0; i < gearsTable.size(); i++)
					{
						GEAR_DESC newGear = gearsTable[i + 1];
						newCosmetics.gears.append(newGear);
					}
				}

				if(table["handlingType"].valid())
					newCosmetics.handlingType = table["handlingType"];

				newCosmetics.headLight = table["headLight"];
				newCosmetics.frontInd = table["frontInd"];
				newCosmetics.backInd = table["backInd"];
				newCosmetics.brakeLight = table["brakeLight"];
				newCosmetics.revLight = table["revLight"];
				newCosmetics.policeLight = table["policeLight"];
				newCosmetics.exhaust = table["exhaust"];
				newCosmetics.smoke = table["smoke"];
				newCosmetics.fire = table["fire"];
				newCosmetics.gravity = table["gravity"];

				for(int i = 0; i < 4; i++)
					newCosmetics.wheelDisp[i] = wheelDispTable[i + 1];

				for (int i = 0; i < 12; i++)
					newCosmetics.cPoints[i] = cPointsTable[i + 1];

				newCosmetics.extraInfo = table["extraInfo"];
				newCosmetics.powerRatio = table["powerRatio"];
				newCosmetics.cbYoffset = table["cbYoffset"];
				newCosmetics.susCoeff = table["susCoeff"];
				newCosmetics.susCompressionLimit = table["susCompressionLimit"];
				newCosmetics.susTopLimit = table["susTopLimit"];
				newCosmetics.traction = table["traction"];
				newCosmetics.wheelSize = table["wheelSize"];
				newCosmetics.colBox = table["colBox"];
				newCosmetics.cog = table["cog"];
				newCosmetics.twistRateX = table["twistRateX"];
				newCosmetics.twistRateY = table["twistRateY"];
				newCosmetics.twistRateZ = table["twistRateZ"];
				newCosmetics.mass = table["mass"];

				return newCosmetics;
			}),
		"ToTable", [](CarCosmetics& self, sol::this_state s) {
			sol::state_view lua(s);
			auto& table = lua.create_table();
			
			table["headLight"] = self.headLight;
			table["frontInd"] = self.frontInd;
			table["backInd"] = self.backInd;
			table["brakeLight"] = self.brakeLight;
			table["revLight"] = self.revLight;
			table["policeLight"] = self.policeLight;
			table["exhaust"] = self.exhaust;
			table["smoke"] = self.smoke;
			table["fire"] = self.fire;
			table["gravity"] = self.gravity;

			table["handlingType"] = self.handlingType;

			auto& wheelDispTable = table["wheelDisp"].get_or_create<sol::table>();
			auto& cPointsTable = table["cPoints"].get_or_create<sol::table>();

			for (int i = 0; i < 4; i++)
				wheelDispTable[i + 1] = self.wheelDisp[i];

			for (int i = 0; i < 12; i++)
				cPointsTable[i + 1] = self.cPoints[i];

			table["extraInfo"] = self.extraInfo;
			table["powerRatio"] = self.powerRatio;
			table["cbYoffset"] = self.cbYoffset;
			table["susCoeff"] = self.susCoeff;
			table["susCompressionLimit"] = self.susCompressionLimit;
			table["susTopLimit"] = self.susTopLimit;
			table["traction"] = self.traction;
			table["wheelSize"] = self.wheelSize;
			table["colBox"] = self.colBox;
			table["cog"] = self.cog;
			table["twistRateX"] = self.twistRateX;
			table["twistRateY"] = self.twistRateY;
			table["twistRateZ"] = self.twistRateZ;
			table["mass"] = self.mass;

			return table;
		},
		"headLight", &CarCosmetics::headLight,
		"frontInd", &CarCosmetics::frontInd,
		"backInd", &CarCosmetics::backInd,
		"brakeLight", &CarCosmetics::brakeLight,
		"revLight", &CarCosmetics::revLight,
		"policeLight", &CarCosmetics::policeLight,
		"exhaust", &CarCosmetics::exhaust,
		"smoke", &CarCosmetics::smoke,
		"fire", &CarCosmetics::fire,
		"wheelDisp", 
			[](CarCosmetics& self, int i) {
				return self.wheelDisp[i - 1];
			},
		"setWheelDisp",
			[](CarCosmetics& self, int i, SVECTOR& v) {
				self.wheelDisp[i - 1] = v;
			},
		"extraInfo", &CarCosmetics::extraInfo,
		"powerRatio", &CarCosmetics::powerRatio,
		"cbYoffset", &CarCosmetics::cbYoffset,
		"susCoeff", &CarCosmetics::susCoeff,
		"traction", &CarCosmetics::traction,
		"wheelSize", &CarCosmetics::wheelSize,
		"cPoints", [](CarCosmetics& self, int i) {
			return self.cPoints[i];
		},
		"setcPoints",
			[](CarCosmetics& self, int i, SVECTOR& v) {
			self.cPoints[i - 1] = v;
		},
		"colBox", &CarCosmetics::colBox,
		"cog", &CarCosmetics::cog,
		"twistRateX", &CarCosmetics::twistRateX,
		"twistRateY", &CarCosmetics::twistRateY,
		"twistRateZ", &CarCosmetics::twistRateZ,
		"mass", &CarCosmetics::mass);

	lua.new_usertype<CCar>(
		"CCar",
		"Destroy", &CCar::Destroy,
		"thrust", &CCar::m_thrust,
		"wheel_angle", &CCar::m_wheel_angle,
		"handbrake", &CCar::m_handbrake,
		"wheelspin", &CCar::m_wheelspin,
		"changingGear", sol::property(&CCar::get_changingGear),
		"wheel_speed", sol::property(&CCar::get_wheel_speed),
		"speed", sol::property(&CCar::get_speed),
		"autobrake", sol::property(&CCar::get_autobrake, &CCar::set_autobrake),
		"cog_position", sol::property(&CCar::GetCogPosition),
		"position", sol::property(&CCar::GetPosition, &CCar::SetPosition),
		"direction", sol::property(&CCar::GetDirection, &CCar::SetDirection),
		"i_cog_position", sol::property(&CCar::GetInterpolatedCogPosition),
		"i_position", sol::property(&CCar::GetInterpolatedPosition),
		"i_direction", sol::property(&CCar::GetInterpolatedDirection),
		"cosmetics", &CCar::m_cosmetics);
}

//--------------------------------------------------------

CCar::CCar()
{
	memset(&m_ap, 0, sizeof(m_ap));
	memset(&m_hd, 0, sizeof(m_hd));
	memset(&m_st, 0, sizeof(m_st));
}

CCar::~CCar()
{
}

void CCar::Destroy()
{
	// too simple
	m_controlType = CONTROL_TYPE_NONE;
}

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
	int oldSpeed, wheelspd;
	LONGVECTOR4 wheelPos, surfacePoint, surfaceNormal;
	VECTOR_NOPAD force;
	LONGVECTOR4 pointVel;
	int frontFS;
	int rearFS;
	sdPlane Surface;
	int i;
	int cdx, cdz;
	int sdx, sdz;
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

	GetFrictionScalesDriver1(cl, frontFS, rearFS);

	m_hd.front_vel = 0;
	m_hd.rear_vel = 0;

	if (oldSpeed > 3300)
		oldSpeed = 3300;

	i = 3;
	wheel = m_hd.wheel + 3;
	do {
		gte_ldv0(&m_cosmetics.wheelDisp[i]);

		gte_rtv0tr();
		gte_stlvnl(wheelPos);

		newCompression = CWorld::FindSurface(*(VECTOR_NOPAD*)&wheelPos, *(VECTOR_NOPAD*)&surfaceNormal, *(VECTOR_NOPAD*)&surfacePoint, Surface);

		//Vector3D lineA = FromFixedVector(*(VECTOR_NOPAD*)&wheelPos) * Vector3D(1, -1, 1);
		//Vector3D lineB = FromFixedVector(*(VECTOR_NOPAD*)&surfacePoint) * Vector3D(1,-1,1);
		//CDebugOverlay::Line(lineA, lineB, ColorRGBA(1, 0, 0, 1));

		friction_coef = (newCompression * (32400 - wetness) >> 15) + 500;

		wheel->onGrass = Surface.surfaceType == 4;
		wheel->surface = 0;

		{
			switch (Surface.surfaceType)
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
			if (Surface.surfaceType - 16U < 16)
				wheel->surface |= 0x8;

			switch (Surface.surfaceType)
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

		oldCompression = wheel->susCompression;
		newCompression = FIXEDH((surfacePoint[1] - wheelPos[1]) * surfaceNormal[1]) + 14;

		if (newCompression < 0)
			newCompression = 0;

		if (newCompression > m_cosmetics.susTopLimit)
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

		if (newCompression > m_cosmetics.susCompressionLimit)
			newCompression = m_cosmetics.susCompressionLimit;

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

					if (m_cosmetics.handlingType.autoBrakeOn != 0 && 0 < sidevel * m_wheel_angle)
						m_hd.autoBrake = -1;

					force.vx = -lfz * m_thrust;
					force.vz = lfx * m_thrust;
				}
				else
				{
					sidevel = FixDivHalfRound(frontFS * slidevel, 14);
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

					forcefac = FixDivHalfRound(FIXEDH(-sidevel * lfx) * sdz - FIXEDH(-sidevel * lfz) * sdx, 11);

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
	HANDLING_TYPE& hp = m_cosmetics.handlingType;

	if (m_thrust < 0)
		frontFS = 1453;
	else if (m_thrust < 1)
		frontFS = 937;
	else
		frontFS = 820;

	autoBrake = m_hd.autoBrake;

	if (m_wheelspin == 0 && hp.autoBrakeOn != 0 && autoBrake > 0 && m_hd.wheel_speed > 0)
	{
		q = autoBrake << 1;

		if (autoBrake > 13)
		{
			autoBrake = 13;
			q = 26;
		}

		frontFS += (q + autoBrake) * 15;

		if(hp.autoBrakeOn == 2)
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
		m_thrust = FIXEDH(m_cosmetics.powerRatio * 5000);
	}

	// [A] 41943 in D2, 61440 in D1
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

	frontFS = (frontFS * hp.frictionScaleRatio) / 32;
	rearFS = (rearFS * hp.frictionScaleRatio) / 32;

	if ((m_hndType == 5) && (m_ai.l.dstate == 5))
	{
		frontFS = (frontFS * 3) / 2;
		rearFS = (rearFS * 3) / 2;
	}

	int traction = m_cosmetics.traction;

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

	twistY = m_cosmetics.twistRateY;
	twistZ = m_cosmetics.twistRateZ;

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

	int friToUse;
	int lift;
	int a, b, speed;
	int count, i;
	CAR_LOCALS _cl;
	LONGVECTOR4 deepestNormal, deepestLever, deepestPoint;
	LONGVECTOR4 pointPos, surfacePoint, surfaceNormal;
	LONGVECTOR4 lever, reaction;
	VECTOR_NOPAD direction;
	sdPlane Surface;

	// FIXME: redundant?
	// if (m_controlType == CONTROL_TYPE_NONE)
	// 	return;

	m_prevPosition = GetPosition();
	m_prevCogPosition = GetCogPosition();
	m_prevDirection = m_hd.direction;

	_cl.aggressive = m_cosmetics.handlingType.aggressiveBraking;
	_cl.extraangulardamping = 0;

	for (i = 0; i < 3; i++)
	{
		_cl.vel[i] = m_st.n.linearVelocity[i];
		_cl.avel[i] = m_st.n.angularVelocity[i];

		m_st.n.fposition[i] = (m_st.n.fposition[i] & 0xF) + m_hd.where.t[i] * 16;
	}

	m_hd.acc[0] = 0;
	m_hd.acc[1] = m_cosmetics.gravity;
	m_hd.acc[2] = 0;

	// calculate car speed
	a = abs(FIXEDH(_cl.vel[0]));
	b = abs(FIXEDH(_cl.vel[2]));

	if (a < b)
		speed = b + a / 2;
	else
		speed = a + b / 2;

	m_hd.speed = speed;

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
		gte_ldv0(&m_cosmetics.cPoints[count]);

		gte_rtv0tr();

		gte_stlvnl(pointPos);

		lever[0] = pointPos[0] - m_hd.where.t[0];
		lever[1] = pointPos[1] - m_hd.where.t[1];
		lever[2] = pointPos[2] - m_hd.where.t[2];

		CWorld::FindSurface(*(VECTOR_NOPAD*)&pointPos, *(VECTOR_NOPAD*)&surfaceNormal, *(VECTOR_NOPAD*)&surfacePoint, Surface);

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

		twistY = m_cosmetics.twistRateY;

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
	
	const int maxGear = m_cosmetics.gears.size() - 1;

	gear = m_hd.gear;
	ws = m_hd.wheel_speed;
	acc = m_thrust;

	if (ws > 0)
	{
		ws >>= 11;

		if (gear > maxGear)
			gear = maxGear;

		gd = &m_cosmetics.gears[gear];

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
		return ws * m_cosmetics.gears[lastgear].ratio_ac;

	return ws * m_cosmetics.gears[lastgear].ratio_id;
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

	short length;

	gte_SetRotMatrix(&m_hd.where);
	gte_SetTransMatrix(&m_hd.where);

	boxDisp.vx = -m_cosmetics.cog.vx;
	boxDisp.vy = -m_cosmetics.cog.vy;
	boxDisp.vz = -m_cosmetics.cog.vz;

	gte_ldv0(&boxDisp);
	gte_rtv0tr();

	if (m_controlType == CONTROL_TYPE_PURSUER_AI)
	{
		length = (m_cosmetics.colBox.vx * 14) / 16;
		m_hd.oBox.length[0] = length;
	}
	else
	{
		length = m_cosmetics.colBox.vx;
		m_hd.oBox.length[0] = length;
	}

	m_hd.oBox.length[1] = m_cosmetics.colBox.vy;
	m_hd.oBox.length[2] = m_cosmetics.colBox.vz;

	gte_stlvnl(&m_hd.oBox.location);

	VECTOR_NOPAD svx = { length, 0 ,0 };
	VECTOR_NOPAD svy = { 0, m_cosmetics.colBox.vy ,0 };
	VECTOR_NOPAD svz = { 0, 0, m_cosmetics.colBox.vz };

	gte_ldlvl(&svx);

	gte_rtir();
	gte_stsv(&m_hd.oBox.radii[0]);

	gte_ldlvl(&svy);
	gte_rtir();
	
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

	int yy = FixDivHalfRound(qy * qy, 11);
	int zz = FixDivHalfRound(qz * qz, 11);
	int xx = FixDivHalfRound(qx * qx, 11);
	int zw = FixDivHalfRound(qz * qw, 11);
	int xy = FixDivHalfRound(qx * qy, 11);
	int xz = FixDivHalfRound(qx * qz, 11);
	int yw = FixDivHalfRound(qy * qw, 11);
	int xw = FixDivHalfRound(qx * qw, 11);
	int yz = FixDivHalfRound(qy * qz, 11);

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

void CCar::DamageCar(CDATA2D* cd, CRET2D* collisionResult, int strikeVel)
{
	// UNIMPLEMENTED
}

void CCar::InitCarPhysics(LONGVECTOR4* startpos, int direction)
{
	int ty;
	int odz;
	int dz;
	int sn, cs;

	dz = m_cosmetics.wheelDisp[0].vz + m_cosmetics.wheelDisp[1].vz;
	ty = dz / 5;
	odz = dz / 32;

	m_hd.direction = direction;

	m_hd.autoBrake = 0;

	sn = isin(direction / 2);
	cs = icos(direction / 2);

	m_st.n.orientation[0] = FIXEDH(-cs * ty);
	m_st.n.orientation[1] = sn;
	m_st.n.orientation[2] = FIXEDH(sn * ty);
	m_st.n.orientation[3] = cs;

	m_st.n.fposition[0] = (*startpos)[0] << 4;
	m_st.n.fposition[1] = (*startpos)[1] << 4;
	m_st.n.fposition[2] = (*startpos)[2] << 4;

	m_st.n.linearVelocity[0] = 0;
	m_st.n.linearVelocity[1] = 0;
	m_st.n.linearVelocity[2] = 0;
	m_st.n.angularVelocity[0] = 0;
	m_st.n.angularVelocity[1] = 0;
	m_st.n.angularVelocity[2] = 0;

	m_hd.aacc[0] = 0;
	m_hd.aacc[1] = 0;
	m_hd.aacc[2] = 0;
	m_hd.acc[0] = 0;
	m_hd.acc[1] = 0;
	m_hd.acc[2] = 0;

	RebuildCarMatrix(m_st);

	UpdateCarDrawMatrix();

	m_hd.wheel[0].susCompression = 14 - odz;
	m_hd.wheel[1].susCompression = odz + 14;
	m_hd.wheel[2].susCompression = 14 - odz;
	m_hd.wheel[3].susCompression = odz + 14;

	m_thrust = 0;
	m_wheel_angle = 0;
	m_hd.wheel_speed = 0;
}

void CCar::TempBuildHandlingMatrix(int init)
{
	int dz;
	int sn, cs;

	dz = (m_cosmetics.wheelDisp[0].vz + m_cosmetics.wheelDisp[1].vz) / 5;

	if (init == 1)
	{
		m_st.n.fposition[0] = m_hd.where.t[0] << 4;
		m_st.n.fposition[1] = m_hd.where.t[1] << 4;
		m_st.n.fposition[2] = m_hd.where.t[2] << 4;
	}

	sn = isin(m_hd.direction / 2);
	cs = icos(m_hd.direction / 2);

	m_st.n.orientation[0] = FIXEDH(-cs * dz);
	m_st.n.orientation[1] = sn;
	m_st.n.orientation[2] = FIXEDH(sn * dz);
	m_st.n.orientation[3] = cs;

	RebuildCarMatrix(m_st);
}

void CCar::StepCarPhysics()
{
	int car_id;

	int frontWheelSpeed;
	int backWheelSpeed;

	HANDLING_TYPE& hp = m_cosmetics.handlingType;

	// [A] update wheel rotation - fix for multiplayer outside cameras
	frontWheelSpeed = m_hd.wheel_speed / 256;

	if (m_hd.wheel[0].locked == 0)
	{
		m_frontWheelRotation += frontWheelSpeed;
		m_frontWheelRotation &= 0xfff;
	}

	if (m_hd.wheel[3].locked == 0)
	{
		backWheelSpeed = frontWheelSpeed;

		if (m_wheelspin != 0)
			backWheelSpeed = 700;

		m_backWheelRotation += backWheelSpeed;
		m_backWheelRotation &= 0xfff;
	}
}

//----------------------------------------------------------

void CCar::UpdateCarDrawMatrix()
{
	m_prevDrawCarMatrix = m_drawCarMatrix;
	m_drawCarMatrix = FromFixedMatrix(m_hd.where);
}

void CCar::InitWheelModels()
{
	// clean
	if (!m_wheelModels[0])
	{
		int modelIdx = g_levModels.FindModelIndexByName("CLEANWHEEL");
		m_wheelModels[0] = g_levModels.GetModelByIndex(modelIdx);
	}

	// fast
	if (!m_wheelModels[1])
	{
		int modelIdx = g_levModels.FindModelIndexByName("FASTWHEEL");
		m_wheelModels[1] = g_levModels.GetModelByIndex(modelIdx);
	}

	// damaged
	if (!m_wheelModels[2])
	{
		int modelIdx = g_levModels.FindModelIndexByName("DAMWHEEL");
		m_wheelModels[2] = g_levModels.GetModelByIndex(modelIdx);
	}
}

void CCar::CreateDentableCar()
{
	// UNIMPEMENTED!!!
}

void CCar::DentCar()
{
	// UNIMPEMENTED!!!
}

VECTOR_NOPAD CCar::GetInterpolatedCogPosition() const
{
	float factor = m_owner->GetInterpTime() / Car_Fixed_Timestep;

	return ToFixedVector(lerp(FromFixedVector(m_prevCogPosition), FromFixedVector(GetCogPosition()), factor));
}

const VECTOR_NOPAD& CCar::GetInterpolatedPosition() const
{
	float factor = m_owner->GetInterpTime() / Car_Fixed_Timestep;

	return ToFixedVector(lerp(FromFixedVector(m_prevPosition), FromFixedVector(GetPosition()), factor));
}

int CCar::GetInterpolatedDirection() const
{
	float factor = m_owner->GetInterpTime() / Car_Fixed_Timestep;

	int shortest_angle = DIFF_ANGLES(m_hd.direction, m_prevDirection);

	return m_prevDirection + float(shortest_angle) * factor;
}

VECTOR_NOPAD CCar::GetCogPosition() const
{
	VECTOR_NOPAD result;
	SVECTOR cog;
	gte_SetRotMatrix(&m_hd.where);
	gte_SetTransMatrix(&m_hd.where);

	cog = m_cosmetics.cog;
	cog.vx = -cog.vx;
	cog.vy = -cog.vy;
	cog.vz = -cog.vz;

	gte_ldv0(&cog);

	gte_rtv0tr();
	gte_stlvnl(&result);

	return result;
}

const VECTOR_NOPAD& CCar::GetPosition() const
{
	return *(VECTOR_NOPAD*)m_hd.where.t;
}

void CCar::SetPosition(const VECTOR_NOPAD& value)
{
	m_hd.where.t[0] = value.vx;
	m_hd.where.t[1] = value.vy;
	m_hd.where.t[2] = value.vz;
}

int	CCar::GetDirection() const
{
	return m_hd.direction;
}

void CCar::SetDirection(const int& newDir)
{
	m_hd.direction = newDir;
	TempBuildHandlingMatrix(0);
}

bool CCar::get_changingGear() const
{
	return m_hd.changingGear;
}

int	CCar::get_wheel_speed() const
{
	return m_hd.wheel_speed;
}

int	CCar::get_speed() const
{
	return m_hd.speed;
}

int8 CCar::get_autobrake() const
{
	return m_hd.autoBrake;
}

void CCar::set_autobrake(const int8& value)
{
	m_hd.autoBrake = value;
}



void CCar::DrawCar()
{
	float factor = m_owner->GetInterpTime() / Car_Fixed_Timestep;

	// this potentially could warp matrix. PLEASE consider using quaternions in future
	Matrix4x4 drawCarMat(
		lerp(m_prevDrawCarMatrix.rows[0], m_drawCarMatrix.rows[0], factor),
		lerp(m_prevDrawCarMatrix.rows[1], m_drawCarMatrix.rows[1], factor),
		lerp(m_prevDrawCarMatrix.rows[2], m_drawCarMatrix.rows[2], factor),
		lerp(m_prevDrawCarMatrix.rows[3], m_drawCarMatrix.rows[3], factor)
	);

	// UNIMPEMENTED!!!
	CRenderModel::SetupModelShader();

	Vector3D cog = FromFixedVector(m_cosmetics.cog);

	Matrix4x4 objectMatrix = drawCarMat * (translate(-cog) * rotateY4(DEG2RAD(180)));

	GR_SetMatrix(MATRIX_WORLD, objectMatrix);
	GR_UpdateMatrixUniforms();

	CRenderModel* renderModel = (CRenderModel*)m_model->userData;

	if (renderModel)
	{
		renderModel->Draw(true, m_ap.palette);
	}

	// draw wheels
	objectMatrix = drawCarMat * rotateY4(DEG2RAD(180));

	{
		const int wheelSize = m_cosmetics.wheelSize;
		int BackWheelIncrement, FrontWheelIncrement;

		BackWheelIncrement = FrontWheelIncrement = m_hd.wheel_speed >> 8;

		if (m_wheelspin != 0)
			BackWheelIncrement = 700;

		if (m_hd.wheel[0].locked != 0)
			FrontWheelIncrement = 0;

		if (m_hd.wheel[3].locked != 0)
			BackWheelIncrement = 0;

		ModelRef_t* wheelModelFront = m_wheelModels[0];
		ModelRef_t* wheelModelBack = m_wheelModels[0];

		if (FrontWheelIncrement + 400U < 801)
			wheelModelFront = m_wheelModels[0];
		else
			wheelModelFront = m_wheelModels[1];

		if (BackWheelIncrement + 400U < 801)
			wheelModelBack = m_wheelModels[0];
		else
			wheelModelBack = m_wheelModels[1];

		for (int i = 0; i < 4; i++)
		{
			const WHEEL& wheel = m_hd.wheel[i];
			const SVECTOR& wheelDisp = m_cosmetics.wheelDisp[i];

			SVECTOR sWheelPos;
			if ((i & 2) == 0)
				sWheelPos.vx = 17 - wheelDisp.vx;
			else
				sWheelPos.vx = -17 - wheelDisp.vx;

			sWheelPos.vz = -wheelDisp.vz;
			sWheelPos.vy = -((-wheelSize - wheelDisp.vy) - wheel.susCompression + 14);

			Vector3D wheelPos = FromFixedVector(sWheelPos);

			Matrix4x4 wheelMat = objectMatrix * translate(wheelPos);

			if ((i & 1) != 0)
			{
				renderModel = (CRenderModel*)wheelModelBack->userData;
				wheelMat = wheelMat * rotateX4(-float(m_backWheelRotation) / 4096.0f * PI_F * 2.0f);
			}
			else
			{
				renderModel = (CRenderModel*)wheelModelFront->userData;
				
				wheelMat = wheelMat * rotateY4(-float(m_wheel_angle) / 4096.0f * PI_F * 2.0f);

				wheelMat = wheelMat * rotateX4(-float(m_frontWheelRotation) / 4096.0f * PI_F * 2.0f);
			}

			if (renderModel)
			{
				GR_SetMatrix(MATRIX_WORLD, wheelMat);
				GR_UpdateMatrixUniforms();

				renderModel->Draw();
			}
		}
	}
}

int CCar::CarBuildingCollision(BUILDING_BOX& building, CELL_OBJECT* cop, int flags)
{
	int temp;
	int strikeVel;
	int boxDiffY;
	int collided;
	short scale;
	int chan;
	MODEL* model;
	VECTOR_NOPAD tempwhere;
	SVECTOR boxDisp;
	VECTOR_NOPAD velocity;
	LONGVECTOR4 pointVel;
	LONGVECTOR4 reaction;
	LONGVECTOR4 lever;
	VECTOR_NOPAD LeafPosition;
	VECTOR_NOPAD lamp_velocity;
	int debris_colour;
	int displacement;
	int denom;
	int buildingHeightY;

	CDATA2D cd[2] = { 0 }; // offset 0x0
	CRET2D collisionResult = { 0 }; // offset 0xd0

	model = building.model;

	cd[0].isCameraOrTanner = (m_controlType == CONTROL_TYPE_TANNERCOLLIDER || m_controlType == CONTROL_TYPE_CAMERACOLLIDER);

	if (m_controlType == CONTROL_TYPE_TANNERCOLLIDER)
		cd[0].isCameraOrTanner += 2;

	cd[1].isCameraOrTanner = (model->flags2 & MODEL_FLAG_BARRIER) == 0;

	boxDiffY = m_hd.oBox.location.vy + building.pos.vy;
	boxDiffY = abs(boxDiffY);

	collided = 0;

	buildingHeightY = building.height >> 1;

	if (boxDiffY <= buildingHeightY + (m_hd.oBox.length[1] >> 1) && (model->shape_flags & SHAPE_FLAG_NOCOLLIDE) == 0 /*&& (cop->pos.vx != OBJECT_SMASHED_MARK)*/)
	{
		tempwhere.vx = m_hd.where.t[0];
		tempwhere.vz = m_hd.where.t[2];

		debris_colour = 0;// GetDebrisColour(cp);

		cd[0].theta = m_hd.direction;
#if 0
		if (m_controlType == CONTROL_TYPE_TANNERCOLLIDER)
		{
			cd[0].x.vx = m_hd.where.t[0];
			cd[0].x.vy = m_hd.where.t[1];
			cd[0].x.vz = m_hd.where.t[2];

			cd[0].vel.vx = FIXEDH(m_st.n.linearVelocity[0]);
			cd[0].vel.vz = FIXEDH(m_st.n.linearVelocity[2]);

			m_hd.where.t[0] += cd[0].vel.vx;
			m_hd.where.t[2] += cd[0].vel.vz;

			cd[0].length[0] = 90;
			cd[0].length[1] = 90;
		}
		else if (m_controlType == CONTROL_TYPE_CAMERACOLLIDER)
		{

			cd[0].x.vx = m_hd.where.t[0];
			cd[0].x.vy = m_hd.where.t[1];
			cd[0].x.vz = m_hd.where.t[2];

			cd[0].vel.vx = 0;
			cd[0].vel.vz = 0;
			cd[0].length[1] = 5;
			cd[0].length[0] = gCameraDistance / 2;
		}
		else
#endif
		{
			gte_SetRotMatrix(&m_hd.where);
			gte_SetTransMatrix(&m_hd.where);

			boxDisp.vx = -m_cosmetics.cog.vx;
			boxDisp.vy = -m_cosmetics.cog.vy;
			boxDisp.vz = -m_cosmetics.cog.vz;

			gte_ldv0(&boxDisp);

			gte_rtv0tr();

			gte_stlvnl(&cd[0].x);

			cd[0].vel.vx = FIXEDH(m_st.n.linearVelocity[0]);
			cd[0].vel.vz = FIXEDH(m_st.n.linearVelocity[2]);

			m_hd.where.t[0] += cd[0].vel.vx;
			m_hd.where.t[2] += cd[0].vel.vz;

			cd[0].length[0] = m_cosmetics.colBox.vz + 15;
			cd[0].length[1] = m_cosmetics.colBox.vx + 15;

			if (m_cosmetics.handlingType.fourWheelDrive == 1 || m_hndType == 5)
				cd[0].length[1] = (cd[0].length[1] * 13) / 16;
		}

		cd[0].avel = FIXEDH(m_st.n.angularVelocity[1]) * 5 >> 5;

		cd[1].x.vx = m_hd.where.t[0] + (((building.pos.vx - m_hd.where.t[0]) << 0x10) >> 0x10);
		cd[1].x.vz = m_hd.where.t[2] + (((building.pos.vz - m_hd.where.t[2]) << 0x10) >> 0x10);

		cd[1].theta = building.theta;
		cd[1].length[0] = building.xsize;
		cd[1].length[1] = building.zsize;
		cd[1].vel.vx = 0;
		cd[1].vel.vz = 0;
		cd[1].avel = 0;

		if (m_controlType == CONTROL_TYPE_CAMERACOLLIDER)
		{
			collided = 0;// bcollided2d(cd, &gCameraBoxOverlap);
		}
		else
		{
			collided = bcollided2d(cd);

			if (collided)
			{
				bFindCollisionTime(cd, collisionResult);
				bFindCollisionPoint(cd, collisionResult);

				collisionResult.surfNormal.vx = -collisionResult.surfNormal.vx;
				collisionResult.surfNormal.vy = 0;
				collisionResult.surfNormal.vz = -collisionResult.surfNormal.vz;

				collisionResult.hit.vy = m_hd.where.t[1] + 41;

				// perform error correction
				m_hd.where.t[0] += FIXEDH(collisionResult.penetration * collisionResult.surfNormal.vx);
				m_hd.where.t[2] += FIXEDH(collisionResult.penetration * collisionResult.surfNormal.vz);

				lever[0] = collisionResult.hit.vx - m_hd.where.t[0];
				lever[1] = collisionResult.hit.vy - m_hd.where.t[1];
				lever[2] = collisionResult.hit.vz - m_hd.where.t[2];

				pointVel[0] = FIXEDH(m_st.n.angularVelocity[1] * lever[2] - m_st.n.angularVelocity[2] * lever[1]) + m_st.n.linearVelocity[0];
				pointVel[1] = FIXEDH(m_st.n.angularVelocity[2] * lever[0] - m_st.n.angularVelocity[0] * lever[2]) + m_st.n.linearVelocity[1];
				pointVel[2] = FIXEDH(m_st.n.angularVelocity[0] * lever[1] - m_st.n.angularVelocity[1] * lever[0]) + m_st.n.linearVelocity[2];
				/*
				if (flags & CollisionCheckFlag_IsVegasMovingTrain) // [A] Vegas train velocity - added here
				{
					pointVel[2] += 700000;
				}*/

				strikeVel = -((pointVel[0] / 256) * (collisionResult.surfNormal.vx / 16) +
					(pointVel[1] / 256) * (collisionResult.surfNormal.vy / 16) +
					(pointVel[2] / 256) * (collisionResult.surfNormal.vz / 16));

				if (strikeVel > 0)
				{
					if (m_controlType == CONTROL_TYPE_PLAYER)
					{
						if (strikeVel < 32)
							scale = ((strikeVel << 23) >> 16);
						else
							scale = 4096;

#if 0
						if (model->flags2 & MODEL_FLAG_SMASHABLE)
							NoteFelony(&felonyData, 7, scale);
						else
							NoteFelony(&felonyData, 6, scale);
#endif
					}

					collisionResult.hit.vy = -collisionResult.hit.vy;

					velocity.vx = m_st.n.linearVelocity[0] / ONE;
					velocity.vy = -17;
					velocity.vz = m_st.n.linearVelocity[2] / ONE;

					if (model->flags2 & MODEL_FLAG_SMASHABLE)
					{
#if 0
						// smash object
						damage_object(cop, &velocity);

						// smash object
						if ((model->shape_flags & SHAPE_FLAG_TRANS) == 0)
						{
							SMASHABLE_OBJECT* match;
							SMASHABLE_OBJECT* sip;

							sip = smashable;
							match = sip;

							while (sip->name != NULL)
							{
								if (sip->modelIdx == cop->type)
								{
									match = sip;
									break;
								}
								sip++;
							}

							chan = GetFreeChannel();
							player_id = GetPlayerId(cp);

							if (NumPlayers > 1 && NoPlayerControl == 0)
								SetPlayerOwnsChannel(chan, player_id);

							Start3DSoundVolPitch(chan, SOUND_BANK_SFX, match->sound,
								collisionResult.hit.vx, -collisionResult.hit.vy, collisionResult.hit.vz,
								match->volume, match->pitch + (((velocity.vx ^ velocity.vz) * (collisionResult.hit.vx ^ collisionResult.hit.vz) & 0x3ff) - 0x200));
						}

						m_hd.where.t[0] = tempwhere.vx;
						m_hd.where.t[2] = tempwhere.vz;

						collisionResult.hit.vy += 30;

						Setup_Sparks(&collisionResult.hit, &velocity, 10, 0);
						Setup_Debris(&collisionResult.hit, &velocity, 5, 0);
						Setup_Debris(&collisionResult.hit, &velocity, 5, debris_colour << 0x10);

						if (m_controlType == CONTROL_TYPE_PLAYER)
							SetPadVibration(*m_ai.padid, 3);
#endif
						m_hd.where.t[0] = tempwhere.vx;
						m_hd.where.t[2] = tempwhere.vz;

						return 0;
					}
#if 0
					// add leaves
					if (strikeVel > 0x3600 && m_hd.wheel_speed + 16000U > 32000)
					{
						if (model->flags2 & MODEL_FLAG_TREE)
						{
							LeafPosition.vx = collisionResult.hit.vx;
							LeafPosition.vy = -((rand() & 0xfe) + 600) - collisionResult.hit.vy;
							LeafPosition.vz = collisionResult.hit.vz;

							AddLeaf(&LeafPosition, 3, 1);
						}
						else
						{
							if (gNight && (model->flags2 & MODEL_FLAG_LAMP))
							{
								if (damage_lamp(cop))
								{
									ClearMem((char*)&lamp_velocity, sizeof(lamp_velocity));

									collisionResult.hit.vy -= 730;
									Setup_Sparks(&collisionResult.hit, &lamp_velocity, 0x14, 0);
									collisionResult.hit.vy += 730;
								}
							}

							velocity.vy -= 20;
							collisionResult.hit.vy += 30;

							Setup_Sparks(&collisionResult.hit, &velocity, 4, 0);

							collisionResult.hit.vy -= 30;
							velocity.vy += 20;
						}

						if (strikeVel > 0x1b000)
						{
							Setup_Debris(&collisionResult.hit, &velocity, 6, debris_colour << 0x10);

							if (m_controlType == CONTROL_TYPE_PLAYER)
								SetPadVibration(*m_ai.padid, 1);
						}
					}
#endif

					DamageCar(cd, &collisionResult, strikeVel);

					displacement = FIXEDH(lever[0] * collisionResult.surfNormal.vx + lever[1] * collisionResult.surfNormal.vy + lever[2] * collisionResult.surfNormal.vz);
					displacement = FIXEDH(((lever[0] * lever[0] + lever[2] * lever[2]) - displacement * displacement) * m_cosmetics.twistRateY) + 4096;

					if (strikeVel < 0x7f001)
						denom = (strikeVel * 4096) / displacement;
					else
						denom = (strikeVel / displacement) * 4096;

					denom /= 64;

					reaction[0] = denom * (collisionResult.surfNormal.vx / 64);
					reaction[1] = denom * (collisionResult.surfNormal.vy / 64);
					reaction[2] = denom * (collisionResult.surfNormal.vz / 64);

					m_hd.aacc[1] += FIXEDH(lever[2] * reaction[0]) - FIXEDH(lever[0] * reaction[2]);

					// angular impulse calculation and modifiers
					if (m_controlType != CONTROL_TYPE_LEAD_AI)
					{
						temp = FIXEDH(lever[1] * reaction[2]);

						if (m_controlType == CONTROL_TYPE_PURSUER_AI)
							temp >>= 1;

						m_hd.aacc[0] += temp;

						temp = FIXEDH(lever[2] * reaction[1]);

						if (m_controlType == CONTROL_TYPE_PURSUER_AI)
							temp >>= 1;

						m_hd.aacc[0] -= temp;

						temp = FIXEDH(lever[0] * reaction[1]);

						if (m_controlType == CONTROL_TYPE_PURSUER_AI)
							temp >>= 1;

						m_hd.aacc[2] += temp;

						temp = FIXEDH(lever[1] * reaction[0]);

						if (m_controlType == CONTROL_TYPE_PURSUER_AI)
							temp >>= 1;

						m_hd.aacc[2] -= temp;

						m_st.n.linearVelocity[1] += reaction[1];
					}

					m_st.n.linearVelocity[0] += reaction[0];
					m_st.n.linearVelocity[2] += reaction[2];
				}
			}

			m_hd.where.t[0] -= FIXEDH(m_st.n.linearVelocity[0]);
			m_hd.where.t[2] -= FIXEDH(m_st.n.linearVelocity[2]);
		}
	}

	return collided;
}
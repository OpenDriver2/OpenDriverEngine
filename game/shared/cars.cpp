#include "game/pch.h"
#include "cars.h"

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
	wheelspinMaxSpeed = 452952;
	susCoeff = 4096;
	baseRPM = 1500;
	cbYoffset = 0;
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

	extraInfo.hasBackLights = srcCos.extraInfo.hasBackLights;
	extraInfo.frontDouble = srcCos.extraInfo.frontDouble;
	extraInfo.backDouble = srcCos.extraInfo.backDouble;
	extraInfo.frontVertical = srcCos.extraInfo.frontVertical;
	extraInfo.backVertical = srcCos.extraInfo.backVertical;

	extraInfo.backOffset = srcCos.extraInfo.backOffset << 2;
	extraInfo.frontOffset = srcCos.extraInfo.frontOffset << 2;

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
	baseRPM = 1500;

	// FIXME: this might change if this become array!
	memcpy(cPoints, srcCos.cPoints, sizeof(cPoints));
	memcpy(wheelDisp, srcCos.wheelDisp, sizeof(wheelDisp));
}

void CarCosmetics::InitFrom(const CAR_COSMETICS_D1& srcCos)
{
	headLight = srcCos.headLight;
	frontInd = srcCos.frontInd;
	backInd = srcCos.backInd;
	brakeLight = srcCos.brakeLight;
	revLight = srcCos.revLight;
	exhaust = srcCos.exhaust;
	smoke = srcCos.smoke;
	fire = srcCos.fire;

	susCompressionLimit = 60;
	susTopLimit = 32767;					// D1 has no limit
	traction = 4096;
	wheelSize = 70;
	wheelspinMaxSpeed = 663552;
	powerRatio = 5833, // or 6000 for superfly

	colBox = SVECTOR{ 320, 122, 760, 0 },	// TODO: find
	cog = SVECTOR{ 0, -25, -45, 0 };		// TODO: find
	twistRateX = 200;
	twistRateY = 110;
	twistRateZ = 550;
	mass = 4096;
	baseRPM = 1000;

	memcpy(wheelDisp, srcCos.wheelDisp, sizeof(wheelDisp));
}

int gCopDifficultyLevel = 0;		// TODO: Lua parameter on difficulty?
int wetness = 0;					// TODO: CWorld::GetWetness()

//--------------------------------------------------------


void CCar::Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();

	{
		LUADOC_TYPE();
		lua.new_usertype<GEAR_DESC>(
			LUADOC_T("GEAR_DESC"),
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
			LUADOC_P("lowidl_ws"), &GEAR_DESC::lowidl_ws,
			LUADOC_P("low_ws"), &GEAR_DESC::low_ws,
			LUADOC_P("hi_ws"), &GEAR_DESC::hi_ws,
			LUADOC_P("ratio_ac"), &GEAR_DESC::ratio_ac,
			LUADOC_P("ratio_id"), &GEAR_DESC::ratio_id
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<HANDLING_TYPE>(
			LUADOC_T("HANDLING_TYPE"),
			sol::call_constructor, sol::factories(
				[](const sol::table& table) {
					return HANDLING_TYPE{
						table["frictionScaleRatio"],
						table["aggressiveBraking"],
						table["fourWheelDrive"],
						table["autoBrakeOn"],
					};
				}),
			LUADOC_P("frictionScaleRatio"), &HANDLING_TYPE::frictionScaleRatio,
			LUADOC_P("aggressiveBraking"), &HANDLING_TYPE::aggressiveBraking,
			LUADOC_P("fourWheelDrive"), &HANDLING_TYPE::fourWheelDrive,
			LUADOC_P("autoBrakeOn"), &HANDLING_TYPE::autoBrakeOn
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<ExtraLightInfo>(
			LUADOC_T("ExtraLightInfo"),
			sol::call_constructor, sol::factories(
				[](const sol::table& table) {
					return ExtraLightInfo{
						table["backOffset"],
						table["frontOffset"],
						table["frontDouble"],
						table["backDouble"],
						table["frontVertical"],
						table["backVertical"],
						table["hasBackLights"]
					};
				}),
			LUADOC_P("backOffset"), &ExtraLightInfo::backOffset,
			LUADOC_P("frontOffset"), &ExtraLightInfo::frontOffset,
			LUADOC_P("frontDouble"), &ExtraLightInfo::frontDouble,
			LUADOC_P("backDouble"), &ExtraLightInfo::backDouble,
			LUADOC_P("frontVertical"), &ExtraLightInfo::frontVertical,
			LUADOC_P("backVertical"), &ExtraLightInfo::backVertical,
			LUADOC_P("hasBackLights"), &ExtraLightInfo::hasBackLights
		);
	}

	{
		LUADOC_TYPE();
		lua.new_usertype<CarCosmetics>(
			LUADOC_T("CarCosmetics"),
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
						for (uint i = 0; i < gearsTable.size(); i++)
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

					newCosmetics.wheelspinMaxSpeed = table["wheelspinMaxSpeed"];
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
					newCosmetics.baseRPM = table["baseRPM"];

					newCosmetics.revSample = table["revSample"];
					newCosmetics.idleSample = table["idleSample"];
					newCosmetics.hornSample = table["hornSample"];

					return newCosmetics;
				}),
			LUADOC_M("ToTable"), [](CarCosmetics& self, sol::this_state s) {
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

				table["wheelspinMaxSpeed"] = self.wheelspinMaxSpeed;
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
				table["baseRPM"] = self.baseRPM;

				table["revSample"] = self.revSample;
				table["idleSample"] = self.idleSample;
				table["hornSample"] = self.hornSample;

				return table;
			},
			LUADOC_P("handlingType"), & CarCosmetics::handlingType,
			LUADOC_P("headLight"), &CarCosmetics::headLight,
			LUADOC_P("frontInd"), &CarCosmetics::frontInd,
			LUADOC_P("backInd"), &CarCosmetics::backInd,
			LUADOC_P("brakeLight"), &CarCosmetics::brakeLight,
			LUADOC_P("revLight"), &CarCosmetics::revLight,
			LUADOC_P("policeLight"), &CarCosmetics::policeLight,
			LUADOC_P("exhaust"), &CarCosmetics::exhaust,
			LUADOC_P("smoke"), &CarCosmetics::smoke,
			LUADOC_P("fire"), &CarCosmetics::fire,
			// TODO: gears table
			LUADOC_M("wheelDisp"), 
				[](CarCosmetics& self, int i) {
					return self.wheelDisp[i - 1];
				},
			LUADOC_M("setWheelDisp"),
				[](CarCosmetics& self, int i, SVECTOR& v) {
					self.wheelDisp[i - 1] = v;
				},
			LUADOC_P("wheelspinMaxSpeed"), &CarCosmetics::wheelspinMaxSpeed,
			LUADOC_P("extraInfo"), &CarCosmetics::extraInfo,
			LUADOC_P("powerRatio"), &CarCosmetics::powerRatio,
			LUADOC_P("cbYoffset"), &CarCosmetics::cbYoffset,
			LUADOC_P("susCoeff"), &CarCosmetics::susCoeff,
			LUADOC_P("traction"), &CarCosmetics::traction,
			LUADOC_P("wheelSize"), &CarCosmetics::wheelSize,
			LUADOC_M("cPoints"), [](CarCosmetics& self, int i) {
				return self.cPoints[i];
			},
			LUADOC_M("setcPoints"),
				[](CarCosmetics& self, int i, SVECTOR& v) {
				self.cPoints[i - 1] = v;
			},
			LUADOC_P("colBox"), &CarCosmetics::colBox,
			LUADOC_P("cog"), &CarCosmetics::cog,
			LUADOC_P("twistRateX"), &CarCosmetics::twistRateX,
			LUADOC_P("twistRateY"), &CarCosmetics::twistRateY,
			LUADOC_P("twistRateZ"), &CarCosmetics::twistRateZ,
			LUADOC_P("mass"), &CarCosmetics::mass,
			LUADOC_P("baseRPM"), &CarCosmetics::baseRPM,

			LUADOC_P("revSample", "audio sample"),
			&CarCosmetics::revSample,

			LUADOC_P("idleSample", "audio sample"), 
			&CarCosmetics::idleSample,

			LUADOC_P("hornSample", "audio sample"),
			&CarCosmetics::hornSample
		);
	}

	{
		LUADOC_TYPE();
		LUA_BEGIN_ENUM(ECarControlType);
		lua.new_enum<ECarControlType>(LUADOC_T("CarControlType"),{ 
			LUA_ENUM(CONTROL_TYPE_NONE, "None"),
			LUA_ENUM(CONTROL_TYPE_PLAYER, "Player", "controlled by player inputs"),
			LUA_ENUM(CONTROL_TYPE_CIV_AI, "CivAI", "Civilian car. May be a passive cop car with CONTROL_FLAG_COP flag."),
			LUA_ENUM(CONTROL_TYPE_PURSUER_AI, "PursuerAI", "Police pursuer car. Always chases player"),
			LUA_ENUM(CONTROL_TYPE_LEAD_AI, "LeadAI", "FreeRoamer AI"),
			LUA_ENUM(CONTROL_TYPE_CUTSCENE, "Cutscene", "Pretty same as player car but controllled by cutscene. Can be a chase car."),
		});
	}

	{
		MAKE_PROPERTY_REF(lua, CCar*);
		LUADOC_TYPE();
		lua.new_usertype<CCar>(
			LUADOC_T("CCar"),

			LUADOC_M("Destroy", "Marks object for destruction in next physics frame"),
			& CCar::Destroy,

			LUADOC_P("controlType", "<CarControlType> - Car control type ID"),
			&CCar::m_controlType,

			LUADOC_P("cosmetics", "<CarCosmetics> - Car configuration"),
			&CCar::m_cosmetics,

			// inputs
			LUADOC_P("thrust", "<short> - accelerator value"),
			&CCar::m_thrust,

			LUADOC_P("wheelAngle", "<short>"),
			&CCar::m_wheel_angle,

			LUADOC_P("handbrake", "<boolean>"),
			&CCar::m_handbrake,

			LUADOC_P("wheelspin", "<boolean>"),
			&CCar::m_wheelspin,

			LUADOC_P("autobrake", "<boolean>"),
			sol::property(&CCar::GetAutobrake, &CCar::SetAutobrake),

			// driving properties
			LUADOC_P("changingGear", "<boolean> (readonly)"),
			sol::property(&CCar::GetChangingGear),

			LUADOC_P("wheelSpeed", "<int> (readonly)"),
			sol::property(&CCar::GetWheelSpeed),

			LUADOC_P("speed", "<int> (readonly)"),
			sol::property(&CCar::GetSpeed),

			// physics properties
			LUADOC_P("linearVelocity", "<fix.VECTOR> (readonly)"),
			sol::property(&CCar::GetLinearVelocity),

			LUADOC_P("angularVelocity", "<fix.VECTOR> (readonly)"),
			sol::property(&CCar::GetAngularVelocity),

			// transform
			LUADOC_P("position", "<fix.VECTOR> (readonly)"),
			sol::property(&CCar::GetPosition, &CCar::SetPosition),

			LUADOC_P("cogPosition", "<fix.VECTOR> (readonly)"),
			sol::property(&CCar::GetCogPosition),

			LUADOC_P("direction", "<int> (readonly)"),
			sol::property(&CCar::GetDirection, &CCar::SetDirection),

			// interpolated transform
			LUADOC_P("i_position", "<fix.VECTOR> (readonly) - interpolated transform"),
			sol::property(&CCar::GetInterpolatedPosition),

			LUADOC_P("i_cogPosition", "<fix.VECTOR> (readonly) - interpolated transform"),
			sol::property(&CCar::GetInterpolatedCogPosition),

			LUADOC_P("i_direction", "<int> (readonly) - interpolated transform"),
			sol::property(&CCar::GetInterpolatedDirection),

			LUADOC_P("i_drawMatrix", "<mat3> (readonly) - interpolated transform"),
			sol::property(&CCar::GetInterpolatedDrawMatrix),

			// Events
			LUADOC_P("eventCallback", "<(self, name: 'HitGround' | 'HitCurb' | 'HitCellObject' | 'HitSmashable' | 'CarsCollision' | 'HitCar', params: table)> - events callback"),
			&CCar::m_carEventsLua
		);
	}
}

//--------------------------------------------------------

CCar::CCar()
{
	memset(&m_ap, 0, sizeof(m_ap));
	memset(&m_hd, 0, sizeof(m_hd));

	memset(&m_rbState, 0, sizeof(m_rbState));
	memset(&m_rbDelta, 0, sizeof(m_rbDelta));
}

CCar::~CCar()
{
	Destroy();
}

void CCar::Destroy()
{
	// too simple
	m_controlType = CONTROL_TYPE_NONE;

	StopSounds();
}

void CCar::StartSounds()
{
	ISoundSource* skidSample = m_owner->GetSoundSource("SkidLoop");

	if (!m_engineSound)
		m_engineSound = IAudioSystem::Instance->CreateSource();
	m_engineSound->Setup(0, m_cosmetics.revSample, &EngineSoundUpdateCb, this);

	if (!m_idleSound)
		m_idleSound = IAudioSystem::Instance->CreateSource();
	m_idleSound->Setup(0, m_cosmetics.idleSample, &IdleSoundUpdateCb, this);

	if (!m_skidSound)
		m_skidSound = IAudioSystem::Instance->CreateSource();
	m_skidSound->Setup(0, skidSample, &SkidSoundUpdateCb, this);

	IAudioSource::Params params;
	params.set_state(IAudioSource::PLAYING);
	params.set_looping(true);
	params.set_referenceDistance(512 / ONE_F);
	params.set_releaseOnStop(true);

	m_skidSound->UpdateParams(params);
	m_engineSound->UpdateParams(params);
	m_idleSound->UpdateParams(params);
}

void CCar::StopSounds()
{
	if (m_engineSound)
		m_engineSound->Release();

	if (m_idleSound)
		m_idleSound->Release();

	if (m_skidSound)
		m_skidSound->Release();

	if (m_dirtSound)
		m_dirtSound->Release();
}

void CCar::StartStaticSound(const char* type, float refDist, float volume, float pitch)
{
	ISoundSource* soundSample = m_owner->GetSoundSource(type);

	if (!soundSample)
	{
		MsgError("StartStaticSound - '%s' is not valid sound name\n", type);
		return;
	}

	IAudioSource* staticSound = IAudioSystem::Instance->CreateSource();

	IAudioSource::Params params;
	params.set_state(IAudioSource::PLAYING);
	params.set_position(FromFixedVector(GetPosition()));
	params.set_releaseOnStop(true);
	params.set_referenceDistance(refDist);
	params.set_volume(volume);
	params.set_pitch(pitch);

	staticSound->Setup(0, soundSample, nullptr, this);
	staticSound->UpdateParams(params);
}

void CCar::CollisionSound(int impact, bool car_vs_car)
{
	if (m_crashTimer > 0)
		return;

	if (impact < 25)
		return;

	const char* soundType = "Hit_Car_1";

	int refDist = 256;

	if (car_vs_car)
	{
		if (impact > 900)
		{
			soundType = "Hit_Car_3";
			refDist = 512;
		}
		else if (impact > 380)
			soundType = "Hit_Car_2";
	}
	else
	{
		if (impact > 780)
		{
			soundType = "Hit_Car_3";
			refDist = 512;
		}
		else if (impact > 350)
			soundType = "Hit_Car_2";
	}

	StartStaticSound(soundType, refDist / ONE_F, 1.0f, 1.0f);
	m_crashTimer = 2;
}

void CCar::AddWheelForcesDriver1(CAR_LOCALS& cl)
{
	int oldSpeed = m_hd.speed * 3 >> 1;

	if (oldSpeed < 32)
		oldSpeed = oldSpeed * -72 + 3696;
	else
		oldSpeed = 1424 - oldSpeed;

	int dir = m_hd.direction;
	const int cdx = isin(dir);
	const int cdz = icos(dir);

	dir += m_wheel_angle;
	const int sdx = isin(dir);
	const int sdz = icos(dir);

	//int player_id = GetPlayerId(cp);

	int frontFS, rearFS;
	GetFrictionScalesDriver1(cl, frontFS, rearFS);

	m_hd.front_vel = 0;
	m_hd.rear_vel = 0;

	if (oldSpeed > 3300)
		oldSpeed = 3300;

	for (int i = 3; i >= 0; i--)
	{
		WHEEL* wheel = &m_hd.wheel[i];

		sdPlane Surface;
		VECTOR_NOPAD wheelPos, surfacePoint, surfaceNormal;
		gte_ldv0(&m_cosmetics.wheelDisp[i]);

		gte_rtv0tr();
		gte_stlvnl(&wheelPos);

		int surfaceFactor = 4096;
		CWorld::FindSurface(wheelPos, surfaceNormal, surfacePoint, Surface);

		if (Surface.surfaceType == (int)SurfaceType::Grass)
		{
			// TODO: (gInGameCutsceneActive && gCurrentMissionNumber == 23 && gInGameCutsceneID == 0)
			const bool d1Roughness = (m_cosmetics.wheelDisp[i].vy < -100);
			const int roughness = isin((surfacePoint[0] + surfacePoint[2]) * 2) >> 8;

			surfacePoint[1] += d1Roughness ? (roughness >> 1) : (roughness / 3);
			surfaceFactor >>= 1;
		}

		int friction_coef = (surfaceFactor * (32400 - wetness) >> 15) + 500;

		wheel->onGrass = Surface.surfaceType == (short)SurfaceType::Grass;
		wheel->surface = 0;

		{
			switch ((SurfaceType)Surface.surfaceType)
			{
				case SurfaceType::Grass:
				case SurfaceType::Water:
				case SurfaceType::DeepWater:
				case SurfaceType::Sand:
					wheel->surface = 0x80;
					break;
				default:
					wheel->surface = 0;
			}

			// [A] indication of Event surface which means we can't add tyre tracks for that wheel
			if (Surface.surfaceType - 16U < 16)
				wheel->surface |= 0x8;

			switch ((SurfaceType)Surface.surfaceType)
			{
				case SurfaceType::Alley:
					wheel->surface |= 0x2;
					break;
				case SurfaceType::Water:
				case SurfaceType::DeepWater:
					wheel->surface |= 0x1;
					break;
				case SurfaceType::Sand:
					wheel->surface |= 0x3;
					break;
			}
		}

		const int oldCompression = wheel->susCompression;
		int newCompression = FIXEDH((surfacePoint[1] - wheelPos[1]) * surfaceNormal[1]) + 14;

		if (newCompression < 0)
			newCompression = 0;

		if (newCompression > m_cosmetics.susTopLimit)
			newCompression = 12;

		// play wheel curb hit sound
		// and apply vibration to player controller
		if (m_controlType == CONTROL_TYPE_PLAYER)
		{
			const int compressionDiff = abs(newCompression - oldCompression);
			if (compressionDiff > 12 && (i & 1U) != 0)
			{
				StartStaticSound("HitCurb", 128 / ONE_F, 0.7f, 400 / ONE_F);
			}

			// Lua interaction
			if (compressionDiff > 12)
			{
				if (m_carEventsLua.valid())
				{
					try {
						sol::state_view sv(m_carEventsLua.lua_state());
						sol::table tbl = sv.create_table_with(
							"wheelNum", i,
							"newCompression", compressionDiff
						);
						m_carEventsLua.call(this, "HitCurb", tbl);
					}
					catch (const sol::error& e)
					{
						MsgError("CCar event call error: %s\n", e.what());
					}
				}
			}
#if 0
			if (newCompression >= 65)
				SetPadVibration(*ai.padid, 1);
			else if (newCompression >= 35)
				SetPadVibration(*ai.padid, 2);
			else if (newCompression > 25)
				SetPadVibration(*ai.padid, 3);
#endif
		}

		if (newCompression > m_cosmetics.susCompressionLimit)
			newCompression = m_cosmetics.susCompressionLimit;

		if (newCompression == 0 && oldCompression == 0)
		{
			wheel->susCompression = 0;
		}
		else
		{
			VECTOR_NOPAD force, pointVel;

			wheelPos[0] -= m_hd.where.t[0];
			wheelPos[1] -= m_hd.where.t[1];
			wheelPos[2] -= m_hd.where.t[2];

			force = 0;

			pointVel[0] = FIXEDH(cl.avel[1] * wheelPos[2] - cl.avel[2] * wheelPos[1]) + cl.vel[0];
			pointVel[2] = FIXEDH(cl.avel[0] * wheelPos[1] - cl.avel[1] * wheelPos[0]) + cl.vel[2];

			// that's our spring
			const int susForce = newCompression * 230 - oldCompression * 100;

			int lfx, lfz;
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

			int slidevel = (pointVel[0] / 64) * (lfx / 64) + (pointVel[2] / 64) * (lfz / 64);
			int wheelspd = abs((oldSpeed / 64) * (slidevel / 64));

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

			int sidevel;

			if (i & 1)
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

					int forcefac = FixDivHalfRound(FIXEDH(-sidevel * lfx) * sdz - FIXEDH(-sidevel * lfz) * sdx, 11);

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

			int angle = m_hd.where.m[1][1];

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
			else if(m_cosmetics.wheelDisp[i].vy < -100) // this will turn it into Driver 1 wheel forces with car always tend to flip over
			{
				wheelPos[1] = 5 * wheelPos[1] / 16;
			}

			m_hd.acc += force;

			m_hd.aacc[0] += FIXEDH(wheelPos[1] * force.vz - wheelPos[2] * force.vy);
			m_hd.aacc[1] += FIXEDH(wheelPos[2] * force.vx - wheelPos[0] * force.vz);
			m_hd.aacc[2] += FIXEDH(wheelPos[0] * force.vy - wheelPos[1] * force.vx);

			wheel->susCompression = newCompression;
		}
	}

	int wheelSpeed = cdz / 64 * (cl.vel[2] / 64) + cdx / 64 * (cl.vel[0] / 64);;

	if (m_hd.wheel[0].susCompression != 0 || m_hd.wheel[2].susCompression != 0)
	{
		m_hd.front_wheel_speed = wheelSpeed;
	}

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
		m_hd.wheel_speed = wheelSpeed;
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

	zd = (twistZ - twistY) * FIXEDH(m_hd.where.m[0][2] * m_hd.aacc[0] + m_hd.where.m[1][2] * m_hd.aacc[1] + m_hd.where.m[2][2] * m_hd.aacc[2]);

	for (i = 0; i < 3; i++)
	{
		m_hd.aacc[i] = m_hd.aacc[i] * twistY + FIXEDH(m_hd.where.m[i][2] * zd - cl.avel[i] * 128);

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

	int impulse;

	int friToUse;
	int lift;
	int a, b, speed;
	int count, i;
	CAR_LOCALS _cl;
	LONGVECTOR4 deepestNormal, deepestLever, deepestPoint;
	LONGVECTOR4 lever, reaction;

	VECTOR_NOPAD pointPos, surfacePoint, surfaceNormal;
	VECTOR_NOPAD direction;

	sdPlane Surface;

	// FIXME: redundant?
	// if (m_controlType == CONTROL_TYPE_NONE)
	// 	return;

	m_prevPosition = GetPosition();
	m_prevCogPosition = GetCogPosition();
	m_prevDirection = m_hd.direction;
	m_prevDrawCarMatrix = m_drawCarMatrix;

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

		gte_stlvnl(&pointPos);

		lever[0] = pointPos.vx - m_hd.where.t[0];
		lever[1] = pointPos.vy - m_hd.where.t[1];
		lever[2] = pointPos.vz - m_hd.where.t[2];

		CWorld::FindSurface(pointPos, surfaceNormal, surfacePoint, Surface);

		if ((surfacePoint.vy - pointPos.vy) - 1U < 799)
		{
			int newLift;

			newLift = FIXEDH((surfacePoint.vy - pointPos.vy) * surfaceNormal.vy);

			if (lift < newLift)
			{
				friToUse = 0;

				deepestNormal[0] = surfaceNormal.vx;
				deepestNormal[1] = surfaceNormal.vy;
				deepestNormal[2] = surfaceNormal.vz;

				deepestLever[0] = lever[0];
				deepestLever[1] = lever[1];
				deepestLever[2] = lever[2];

				deepestPoint[0] = surfacePoint.vx;
				deepestPoint[1] = surfacePoint.vy;
				deepestPoint[2] = surfacePoint.vz;

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

		// Lua interaction
		if (m_carEventsLua.valid())
		{
			try {
				sol::state_view sv(m_carEventsLua.lua_state());
				sol::table tbl = sv.create_table_with(
					"position", LuaPropertyRef(surfacePoint),
					"normal", LuaPropertyRef(surfaceNormal),
					"strikeVel", LuaPropertyRef(impulse)		// in reversed code it's probably named incorrectly
				);
				m_carEventsLua.call(this, "HitGround", tbl);
			}
			catch (const sol::error& e)
			{
				MsgError("CCar event call error: %s\n", e.what());
			}
		}

		if (impulse > 20000)
		{
#if 0
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
#endif
			if ((Surface.surfaceType != 9) && (Surface.surfaceType != 6))
			{
				CollisionSound((impulse / 6 + (impulse >> 0x1f) >> 3) - (impulse >> 0x1f), false);
			}
		}


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

	if (acc == 0 && newRevs < 7001)
	{
		acc = m_revsvol;

		m_idlevol += 200;
		m_revsvol = acc - 200;

		if (m_idlevol > -6000)
			m_idlevol = -6000;

		if (m_revsvol < -10000)
			m_revsvol = -10000;
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

		m_idlevol += acc;

		if (spin == 0)
			acc = 175;
		else
			acc = 700;

		m_revsvol = m_revsvol + acc;

		if (m_idlevol < -10000)
			m_idlevol = -10000;

		if (m_revsvol > revsmax)
			m_revsvol = revsmax;
	}
}

//---------------------------------------------------

void CCar::InitOrientedBox()
{
	SVECTOR_NOPAD boxDisp;

	short length;

	gte_SetRotMatrix(&m_hd.where);
	gte_SetTransMatrix(&m_hd.where);

	boxDisp = -m_cosmetics.cog.p();

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

	VECTOR_NOPAD _zero{ 0 };
	gte_SetTransVector(&_zero);

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

void CCar::EngineSoundUpdateCb(void* obj, IAudioSource::Params& params)
{
	CCar* thisCar = (CCar*)obj;

	if (thisCar->m_controlType == CONTROL_TYPE_NONE)
	{
		params.set_state(IAudioSource::STOPPED);
		return;
	}

	int pitch = thisCar->m_hd.revs / 4 + thisCar->m_revsvol / 64 + thisCar->m_cosmetics.baseRPM;
	params.set_volume((10000 + thisCar->m_revsvol) / 10000.0f);
	params.set_pitch(pitch / ONE_F);
	params.set_position(FromFixedVector(thisCar->GetPosition()));
	params.set_velocity(FromFixedVector(thisCar->GetLinearVelocity()));
}

void CCar::IdleSoundUpdateCb(void* obj, IAudioSource::Params& params)
{
	CCar* thisCar = (CCar*)obj;

	if (thisCar->m_controlType == CONTROL_TYPE_NONE)
	{
		params.set_state(IAudioSource::STOPPED);
		return;
	}

	int pitch = thisCar->m_hd.revs / 4 + 4096;
	params.set_volume((10000 + thisCar->m_idlevol) / 10000.0f);
	params.set_pitch(pitch / ONE_F);
	params.set_position(FromFixedVector(thisCar->GetPosition()));
	params.set_velocity(FromFixedVector(thisCar->GetLinearVelocity()));
}

void CCar::SkidSoundUpdateCb(void* obj, IAudioSource::Params& params)
{
	CCar* thisCar = (CCar*)obj;

	if (thisCar->m_controlType == CONTROL_TYPE_NONE)
	{
		params.set_state(IAudioSource::STOPPED);
		return;
	}

	int skidsound = 0;
	bool wheels_on_ground = false;
	bool rear_only = false;
	bool lay_down_tracks = false;
	bool tracks_and_smoke = false;

	for (int cnt = 0; cnt < 4; cnt++)
	{
		if (thisCar->m_hd.wheel[cnt].susCompression != 0)
			wheels_on_ground = true;
	}

	skidsound = 0;

	// make tyre tracks and skid sound if needed
	if (wheels_on_ground)
	{
		int rear_vel, front_vel;
		rear_vel = abs(thisCar->m_hd.rear_vel);
		front_vel = abs(thisCar->m_hd.front_vel);

		if (rear_vel > 22000 || thisCar->m_wheelspin)
		{
			rear_only = 1;
			lay_down_tracks = true;

			if (thisCar->m_wheelspin == 0)
				skidsound = (rear_vel - 11100) / 2 + 1;
			else
				skidsound = 13000;

			if (skidsound > 13000)
				skidsound = 13000;
		}
		else if (front_vel > 50000)
		{
			rear_only = 0;
			lay_down_tracks = true;
		}

		tracks_and_smoke = !(thisCar->m_hd.wheel[1].surface & 0x8) && !(thisCar->m_hd.wheel[3].surface & 0x8);
	}

	if (skidsound != 0 && !((thisCar->m_hd.wheel[1].surface & 0x80) == 0 || (thisCar->m_hd.wheel[3].surface & 0x80) == 0))
	{
		skidsound = 0;
	}

	if (skidsound == 0)
	{
		params.set_state(IAudioSource::PAUSED);
		return;
	}

	if (params.state != IAudioSource::PLAYING)
	{
		params.set_state(IAudioSource::PLAYING);
	}

	const int volume = (skidsound - 10000) * 3 / 4 - 5000;
	const int pitch = skidsound * 1024 / 13000 + 3072;

	params.set_volume((10000 + volume) / 10000.0f);
	params.set_pitch(pitch / ONE_F);
	params.set_position(FromFixedVector(thisCar->GetPosition()));
	params.set_velocity(FromFixedVector(thisCar->GetLinearVelocity()));		
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

void CCar::DamageCar(CDATA2D* cd, const CRET2D& collisionResult, int strikeVel)
{
	// UNIMPLEMENTED

	if (strikeVel >= 20480 && m_hd.speed > 9)
	{
		CollisionSound(strikeVel / 600, false);
	}
}

bool CCar::DamageCar3D(const VECTOR_NOPAD& delta, int strikeVel, CCar* pOtherCar)
{
	// UNIMPLEMENTED

	strikeVel = strikeVel * 375 >> 8;

	if (strikeVel < 40960)
	{
		if (m_totalDamage == 0)
			m_totalDamage = 1;

		return false;
	}

	if(m_id >= pOtherCar->m_id)
		CollisionSound(strikeVel / 128, true);

	return true;
}

void CCar::InitCarPhysics(const VECTOR_NOPAD& startpos, int direction)
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

	m_st.n.fposition[0] = startpos[0] << 4;
	m_st.n.fposition[1] = startpos[1] << 4;
	m_st.n.fposition[2] = startpos[2] << 4;

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

	// TEMPORARY
	StartSounds();
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

	if (m_hd.wheel[0].locked == 0)
	{
		frontWheelSpeed = m_hd.front_wheel_speed / (m_cosmetics.wheelSize*5);
		m_frontWheelRotation += frontWheelSpeed;
		m_frontWheelRotation &= 0xfff;
	}

	if (m_hd.wheel[3].locked == 0)
	{
		backWheelSpeed = m_hd.wheel_speed / (m_cosmetics.wheelSize * 5);

		if (m_wheelspin != 0)
			backWheelSpeed = 700;

		m_backWheelRotation += backWheelSpeed;
		m_backWheelRotation &= 0xfff;
	}

	if(m_crashTimer)
		m_crashTimer--;
}

//----------------------------------------------------------

void CCar::ResetInterpolation()
{
	m_prevDrawCarMatrix = m_drawCarMatrix = FromFixedMatrix(m_hd.where);
	m_prevCogPosition = GetCogPosition();
	m_prevPosition = GetPosition();
	m_prevDirection = GetDirection(); 
}

void CCar::UpdateCarDrawMatrix()
{
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

float CCar::GetLerpValue() const
{
	const float factor = clamp(m_owner->GetInterpTime() / Car_Fixed_Timestep, 0.0f, 1.0f);
	return 0.5f + factor;
}

VECTOR_NOPAD CCar::GetInterpolatedCogPosition() const
{
	return ToFixedVector(lerp(FromFixedVector(m_prevCogPosition), FromFixedVector(GetCogPosition()), GetLerpValue()));
}

VECTOR_NOPAD CCar::GetInterpolatedPosition() const
{
	return ToFixedVector(lerp(FromFixedVector(m_prevPosition), FromFixedVector(GetPosition()), GetLerpValue()));
}

float CCar::GetInterpolatedDirection() const
{
	int shortest_angle = DIFF_ANGLES(m_prevDirection, m_hd.direction);

	return float(m_prevDirection) + float(shortest_angle) * GetLerpValue();
}

Matrix3x3 CCar::GetInterpolatedDrawMatrix() const
{
	const float factor = GetLerpValue();
	return Matrix3x3(
		lerp(m_prevDrawCarMatrix.rows[0].xyz(), m_drawCarMatrix.rows[0].xyz(), factor),
		lerp(m_prevDrawCarMatrix.rows[1].xyz(), m_drawCarMatrix.rows[1].xyz(), factor),
		lerp(m_prevDrawCarMatrix.rows[2].xyz(), m_drawCarMatrix.rows[2].xyz(), factor)
	);
}

Matrix4x4 CCar::GetInterpolatedDrawMatrix4() const
{
	const float factor = GetLerpValue();
	return Matrix4x4(
		lerp(m_prevDrawCarMatrix.rows[0], m_drawCarMatrix.rows[0], factor),
		lerp(m_prevDrawCarMatrix.rows[1], m_drawCarMatrix.rows[1], factor),
		lerp(m_prevDrawCarMatrix.rows[2], m_drawCarMatrix.rows[2], factor),
		lerp(m_prevDrawCarMatrix.rows[3], m_drawCarMatrix.rows[3], factor)
	);
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

	ResetInterpolation();
}

int	CCar::GetDirection() const
{
	return m_hd.direction;
}

void CCar::SetDirection(const int& newDir)
{
	m_hd.direction = newDir;
	TempBuildHandlingMatrix(0);

	ResetInterpolation();
}

const VECTOR_NOPAD& CCar::GetLinearVelocity() const
{
	return *(VECTOR_NOPAD*)&m_st.n.linearVelocity;
}

const VECTOR_NOPAD& CCar::GetAngularVelocity() const
{
	return *(VECTOR_NOPAD*)&m_st.n.angularVelocity;
}

const MATRIX& CCar::GetMatrix() const
{
	return m_hd.where;
}

const OrientedBox& CCar::GetOrientedBox() const
{
	return m_hd.oBox;
}

const CarCosmetics& CCar::GetCosmetics() const
{
	return m_cosmetics;
}

bool CCar::GetChangingGear() const
{
	return m_hd.changingGear;
}

int	CCar::GetWheelSpeed() const
{
	return m_hd.wheel_speed;
}

int	CCar::GetSpeed() const
{
	return m_hd.speed;
}

int8 CCar::GetAutobrake() const
{
	return m_hd.autoBrake;
}

void CCar::SetAutobrake(const int8& value)
{
	m_hd.autoBrake = value;
}



void CCar::DrawCar()
{
	// this potentially could warp matrix. PLEASE consider using quaternions in future
	Matrix4x4 drawCarMat = GetInterpolatedDrawMatrix4();

	// UNIMPEMENTED!!!
	CRenderModel::SetupModelShader();

	Vector3D cog = FromFixedVector(m_cosmetics.cog);

	Matrix4x4 objectMatrix = drawCarMat * (translate(-cog) * rotateY4(DEG2RAD(180)));

	GR_SetMatrix(MATRIX_WORLD, objectMatrix);
	GR_UpdateMatrixUniforms();
	GR_SetCullMode(CULL_FRONT);

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

		FrontWheelIncrement = m_hd.front_wheel_speed >> 8;
		BackWheelIncrement = m_hd.wheel_speed >> 8;

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

		const float sizeScale = ((wheelSize * 14142) / 10000) / ONE_F;

		float wheelSizeInvScale;
		{
			renderModel = (CRenderModel*)wheelModelFront->userData;

			Vector3D wheelMins, wheelMaxs;
			renderModel->GetExtents(wheelMins, wheelMaxs);

			wheelSizeInvScale = sizeScale / length(wheelMaxs.yz());
		}

		GR_SetCullMode(CULL_NONE);

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

			Matrix4x4 wheelScaleMat = scale4(1.0f, wheelSizeInvScale, wheelSizeInvScale);
			Matrix4x4 wheelMat = objectMatrix * translate(wheelPos);

			if ((i & 1) != 0)
			{
				renderModel = (CRenderModel*)wheelModelBack->userData;
				renderModel->SetupRendering(true, true);

				wheelMat = wheelMat * wheelScaleMat;

				GR_SetMatrix(MATRIX_WORLD, wheelMat);
				GR_UpdateMatrixUniforms();

				renderModel->DrawBatch(1);
				
				wheelMat = wheelMat * rotateX4(-float(m_backWheelRotation) * TO_RADIAN);
			}
			else
			{
				renderModel = (CRenderModel*)wheelModelFront->userData;
				renderModel->SetupRendering(true, true);
				
				wheelMat = wheelMat * rotateY4(-float(m_wheel_angle) * TO_RADIAN) * wheelScaleMat;

				GR_SetMatrix(MATRIX_WORLD, wheelMat);
				GR_UpdateMatrixUniforms();

				renderModel->DrawBatch(1);

				wheelMat = wheelMat * rotateX4(-float(m_frontWheelRotation) * TO_RADIAN);
			}

			GR_SetMatrix(MATRIX_WORLD, wheelMat);
			GR_UpdateMatrixUniforms();

			renderModel->DrawBatch(0);
		}
	}
}

bool CCar::CarBuildingCollision(const BUILDING_BOX& building, CELL_OBJECT* cop, int flags)
{
	CDATA2D cd[2] = { 0 };
	CRET2D collisionResult = { 0 };

	MODEL* model = building.modelRef->model;

	const int boxDiffY = abs(m_hd.oBox.location.vy + building.pos.vy);
	const int buildingHeightY = building.height >> 1;

	if (boxDiffY >= buildingHeightY + (m_hd.oBox.length[1] >> 1) ||
		(model->shape_flags & SHAPE_FLAG_NOCOLLIDE) ||
		(cop->pos.vx == OBJECT_SMASHED_MARK))
	{
		return false;
	}

	cd[0].ignoreBarrier = (m_controlType == CONTROL_TYPE_TANNERCOLLIDER);
	cd[1].ignoreBarrier = (model->flags2 & MODEL_FLAG_BARRIER) == 0;

	bool collided = false;
	int debris_colour = 0;// GetDebrisColour(cp);

	cd[0].theta = m_hd.direction;
#if 0
	if (m_controlType == CONTROL_TYPE_TANNERCOLLIDER)
	{
		cd[0].x.vx = m_hd.where.t[0];
		cd[0].x.vy = m_hd.where.t[1];
		cd[0].x.vz = m_hd.where.t[2];

		cd[0].vel.vx = FIXEDH(m_st.n.linearVelocity[0]);
		cd[0].vel.vz = FIXEDH(m_st.n.linearVelocity[2]);

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

		SVECTOR_NOPAD boxDisp = -m_cosmetics.cog.p();
		gte_ldv0(&boxDisp);
		gte_rtv0tr();
		gte_stlvnl(&cd[0].x);

		cd[0].vel.vx = FIXEDH(m_st.n.linearVelocity[0]);
		cd[0].vel.vz = FIXEDH(m_st.n.linearVelocity[2]);

		cd[0].length[0] = m_cosmetics.colBox.vz + 15;
		cd[0].length[1] = m_cosmetics.colBox.vx + 15;

		if (m_cosmetics.handlingType.fourWheelDrive == 1 || m_hndType == 5)
			cd[0].length[1] = (cd[0].length[1] * 13) / 16;
	}

	VECTOR_NOPAD tempwhere;
	tempwhere.vx = m_hd.where.t[0];
	tempwhere.vz = m_hd.where.t[2];

	cd[0].avel = FIXEDH(m_st.n.angularVelocity[1]) * 5 >> 5;

	m_hd.where.t[0] += cd[0].vel.vx;
	m_hd.where.t[2] += cd[0].vel.vz;

	cd[1].x.vx = m_hd.where.t[0] + (short)(building.pos.vx - m_hd.where.t[0]);
	cd[1].x.vz = m_hd.where.t[2] + (short)(building.pos.vz - m_hd.where.t[2]);

	cd[1].theta = building.theta;
	cd[1].length[0] = building.xsize;
	cd[1].length[1] = building.zsize;
	cd[1].vel.vx = 0;
	cd[1].vel.vz = 0;
	cd[1].avel = 0;

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

		VECTOR_NOPAD pointVel, reaction, lever;

		lever[0] = collisionResult.hit.vx - m_hd.where.t[0];
		lever[1] = collisionResult.hit.vy - m_hd.where.t[1];
		lever[2] = collisionResult.hit.vz - m_hd.where.t[2];

		pointVel[0] = FIXEDH(m_st.n.angularVelocity[1] * lever[2] - m_st.n.angularVelocity[2] * lever[1]) + m_st.n.linearVelocity[0];
		pointVel[1] = FIXEDH(m_st.n.angularVelocity[2] * lever[0] - m_st.n.angularVelocity[0] * lever[2]) + m_st.n.linearVelocity[1];
		pointVel[2] = FIXEDH(m_st.n.angularVelocity[0] * lever[1] - m_st.n.angularVelocity[1] * lever[0]) + m_st.n.linearVelocity[2];

		// Lua interaction
		if (m_carEventsLua.valid())
		{
			try {
				sol::state_view sv(m_carEventsLua.lua_state());
				sol::table tbl = sv.create_table_with(
					"model", building.modelRef,
					"cellObject", cop,
					"position", LuaPropertyRef(collisionResult.hit),
					"normal", LuaPropertyRef(collisionResult.surfNormal),
					"pointVel", LuaPropertyRef(pointVel)
				);
				m_carEventsLua.call(this, "HitCellObject", tbl);
			}
			catch (const sol::error& e)
			{
				MsgError("CCar event call error: %s\n", e.what());
			}
		}

		/*
		if (flags & CollisionCheckFlag_IsVegasMovingTrain) // [A] Vegas train velocity - added here
		{
			pointVel[2] += 700000;
		}*/

		int strikeVel = -((pointVel[0] / 256) * (collisionResult.surfNormal.vx / 16) +
						  (pointVel[1] / 256) * (collisionResult.surfNormal.vy / 16) +
						  (pointVel[2] / 256) * (collisionResult.surfNormal.vz / 16));

		if (strikeVel > 0)
		{
#if 0
			if (m_controlType == CONTROL_TYPE_PLAYER)
			{
				short scale;
				if (strikeVel < 32)
					scale = ((strikeVel << 23) >> 16);
				else
					scale = 4096;

				if (model->flags2 & MODEL_FLAG_SMASHABLE)
					NoteFelony(&felonyData, 7, scale);
				else
					NoteFelony(&felonyData, 6, scale);
			}
#endif
			collisionResult.hit.vy = -collisionResult.hit.vy;

			VECTOR_NOPAD velocity;
			velocity.vx = m_st.n.linearVelocity[0] / ONE;
			velocity.vy = -17;
			velocity.vz = m_st.n.linearVelocity[2] / ONE;

			if (model->flags2 & MODEL_FLAG_SMASHABLE)
			{
				// TODO: World lua callback on smashables

				// Lua interaction
				if (m_carEventsLua.valid())
				{
					try {
						sol::state_view sv(m_carEventsLua.lua_state());
						sol::table tbl = sv.create_table_with(
							"model", building.modelRef,
							"cellObject", cop,
							"position", LuaPropertyRef(collisionResult.hit),
							"normal", LuaPropertyRef(collisionResult.surfNormal),
							"velocity", LuaPropertyRef(velocity),
							"strikeVel", LuaPropertyRef(strikeVel)
						);
						m_carEventsLua.call(this, "HitSmashable", tbl);
					}
					catch (const sol::error& e)
					{
						MsgError("CCar event call error: %s\n", e.what());
					}
				}

				cop->pos.vx = OBJECT_SMASHED_MARK;
#if 0
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

			DamageCar(cd, collisionResult, strikeVel);

			int displacement = FIXEDH(lever[0] * collisionResult.surfNormal.vx + lever[1] * collisionResult.surfNormal.vy + lever[2] * collisionResult.surfNormal.vz);
			displacement = FIXEDH(((lever[0] * lever[0] + lever[2] * lever[2]) - displacement * displacement) * m_cosmetics.twistRateY) + 4096;

			int denom;
			if (strikeVel <= 127 * 4096)
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
				int temp = FIXEDH(lever[1] * reaction[2]);

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

	return collided;
}

bool CCar::CarCarCollision(CCar* other, int RKstep)
{
	/*
	// Lua interaction
	if (m_carEvents.valid())
	{
		try {
			sol::state_view sv(m_carEvents.lua_state());
			sol::table tbl = sv.create_table_with(
				"model", building.modelRef,
				"cellObject", cop,
				"position", collisionResult.hit,
				"normal", collisionResult.surfNormal,
				"strikeVel", strikeVel
			);
			m_carEvents.call("HitCellObject", tbl);
		}
		catch (const sol::error& e)
		{
			MsgError("CCar event call error: %s\n", e.what());
		}
	}
	*/
	CRET3D collResult;

	if (!collided3d(this, other, collResult))
		return false;

	// TODO: separate collision response code
	CCar* cp = this;
	CCar* c1 = other;

	RigidBodyState& cpDelta = cp->m_rbDelta[RKstep];		// thisDelta[i]
	RigidBodyState& cpState = cp->m_rbState[RKstep];		// thisState_i

	RigidBodyState& c1Delta = c1->m_rbDelta[RKstep];		// thisDelta[j]
	RigidBodyState& c1State = c1->m_rbState[RKstep];		// thisState_j

	int c1InfiniteMass;
	int c2InfiniteMass;

	collResult.location.vy -= 0;

	VECTOR_NOPAD lever0, lever1;
	lever0[0] = collResult.location.vx - cp->m_hd.where.t[0];
	lever0[1] = collResult.location.vy - cp->m_hd.where.t[1];
	lever0[2] = collResult.location.vz - cp->m_hd.where.t[2];

	lever1[0] = collResult.location.vx - c1->m_hd.where.t[0];
	lever1[1] = collResult.location.vy - c1->m_hd.where.t[1];
	lever1[2] = collResult.location.vz - c1->m_hd.where.t[2];

	int strength = 47 - (lever0[1] + lever1[1]) / 2;

	lever0[1] += strength;
	lever1[1] += strength;

	int strikeVel = collResult.depth * 0xc000;

	VECTOR_NOPAD pointVel0;
	pointVel0[0] = (FIXEDH(cpState.n.angularVelocity[1] * lever0[2] - cpState.n.angularVelocity[2] * lever0[1]) + cpState.n.linearVelocity[0]) -
		(FIXEDH(c1State.n.angularVelocity[1] * lever1[2] - c1State.n.angularVelocity[2] * lever1[1]) + c1State.n.linearVelocity[0]);

	pointVel0[1] = (FIXEDH(cpState.n.angularVelocity[2] * lever0[0] - cpState.n.angularVelocity[0] * lever0[2]) + cpState.n.linearVelocity[1]) -
		(FIXEDH(c1State.n.angularVelocity[2] * lever1[0] - c1State.n.angularVelocity[0] * lever1[2]) + c1State.n.linearVelocity[1]);

	pointVel0[2] = (FIXEDH(cpState.n.angularVelocity[0] * lever0[1] - cpState.n.angularVelocity[1] * lever0[0]) + cpState.n.linearVelocity[2]) -
		(FIXEDH(c1State.n.angularVelocity[0] * lever1[1] - c1State.n.angularVelocity[1] * lever1[0]) + c1State.n.linearVelocity[2]);

	int howHard =	(pointVel0[0] / 256) * (collResult.normal.vx / 32) +
					(pointVel0[1] / 256) * (collResult.normal.vy / 32) +
					(pointVel0[2] / 256) * (collResult.normal.vz / 32);

	if (howHard > 0 && RKstep > -1)
	{
		if (c1->DamageCar3D(lever1, howHard >> 1, cp))
			c1->m_ap.needsDenting = 1;

		if (cp->DamageCar3D(lever0, howHard >> 1, c1))
			cp->m_ap.needsDenting = 1;

		if (howHard > 2048 * 100)
		{
			if (cp->m_controlType == CONTROL_TYPE_CIV_AI)
				cp->m_ai.c.carMustDie = 1;

			if (c1->m_controlType == CONTROL_TYPE_CIV_AI)
				c1->m_ai.c.carMustDie = 1;
		}

#if 0
		// wake up cops if they've ben touched
		// [A] check player felony.
		// If player touch them without felony player will be charged with felony (hit cop car)
		if (numCopCars < 4 && numActiveCops < maxCopCars && GameType != GAME_GETAWAY && *felony >= FELONY_PURSUIT_MIN_VALUE)
		{
			if (cp->m_controlType == CONTROL_TYPE_PLAYER && IS_ROADBLOCK_CAR(c1))
			{
				InitCopState(c1, NULL);
				c1->m_ai.p.justPinged = 0;
			}

			if (c1->m_controlType == CONTROL_TYPE_PLAYER && IS_ROADBLOCK_CAR(cp))
			{
				InitCopState(cp, NULL);
				cp->m_ai.p.justPinged = 0;
			}
		}
#endif

#if 0
		if (howHard > 0x1b00)
		{
			velocity.vy = -17;
			velocity.vx = FIXED(cp->m_st.n.linearVelocity[0]);
			velocity.vz = FIXED(cp->m_st.n.linearVelocity[2]);

			collResult.location[1] = -collResult.location[1];

			if (cp->m_controlType == CONTROL_TYPE_PLAYER || c1->m_controlType == CONTROL_TYPE_PLAYER)
			{
				Setup_Sparks((VECTOR*)collResult.location, &velocity, 6, 0);

				if (cp->m_controlType == CONTROL_TYPE_PLAYER)
					SetPadVibration(*cp->m_ai.padid, 1);

				if (c1->m_controlType == CONTROL_TYPE_PLAYER)
					SetPadVibration(*c1->m_ai.padid, 1);
			}

			if (howHard > 0x2400)
			{
				int debris1;
				int debris2;

				debris1 = GetDebrisColour(cp);
				debris2 = GetDebrisColour(c1);

				Setup_Debris((VECTOR*)collResult.location, &velocity, 3, 0);
				Setup_Debris((VECTOR*)collResult.location, &velocity, 6, debris1 << 0x10);
				Setup_Debris((VECTOR*)collResult.location, &velocity, 2, debris2 << 0x10);
			}
		}
#endif
	}

	strikeVel += (howHard * 9) / 4;

	if (strikeVel > 0x69000)
		strikeVel = 0x69000;

	int m1 = cp->m_cosmetics.mass;
	int m2 = c1->m_cosmetics.mass;

	// defaults
	c1InfiniteMass = (cp->m_controlType == CONTROL_TYPE_CUTSCENE || m1 == 0x7fff);
	c2InfiniteMass = (c1->m_controlType == CONTROL_TYPE_CUTSCENE || m2 == 0x7fff);

	// Lua interaction
	if (m_carEventsLua.valid())
	{
		try {
			sol::state_view sv(m_carEventsLua.lua_state());
			sol::table tbl = sv.create_table_with(
				"car1", this,
				"car2", other,
				"position", LuaPropertyRef(collResult.location),
				"normal", LuaPropertyRef(collResult.normal),
				"strikeVel", LuaPropertyRef(strikeVel),
				"mass1", LuaPropertyRef(m1),
				"mass2", LuaPropertyRef(m2),
				"c1InfiniteMass", LuaPropertyRef(c1InfiniteMass),
				"c2InfiniteMass", LuaPropertyRef(c2InfiniteMass)
			);
			m_carEventsLua.call(this, "CarsCollision", tbl);
		}
		catch (const sol::error& e)
		{
			MsgError("CCar event call error: %s\n", e.what());
		}
	}

	int do1, do2;
	if (m2 < m1)
	{
		do1 = (m2 * 4096) / m1;
		do2 = 4096;
	}
	else
	{
		do2 = (m1 * 4096) / m2;
		do1 = 4096;
	}

	// reduce strike velocity for infinite mass cars (ReD2 change)
	if (c1InfiniteMass || c2InfiniteMass)
		strikeVel = strikeVel * 10 >> 2;

	CollisionResponse(cpDelta, other, strikeVel, do1, c1InfiniteMass, lever0, collResult);

	// don't forget to invert normal
	collResult.normal = -collResult.normal;

	other->CollisionResponse(c1Delta, this, strikeVel, do2, c2InfiniteMass, lever1, collResult);

#if 0
	if (cp->m_id == player[0].playerCarId || c1->m_id == player[0].playerCarId)
		RegisterChaseHit(cp->m_id, c1->m_id);

	if (cp->m_id == player[0].playerCarId)
		CarHitByPlayer(c1, howHard);

	if (c1->m_id == player[0].playerCarId)
		CarHitByPlayer(cp, howHard);
#endif

	return true;
}

void CCar::CollisionResponse(RigidBodyState& delta, CCar* other, int strikeVel, int doFactor, bool infiniteMass, const VECTOR_NOPAD& lever, const CRET3D& collResult)
{
	// Lua interaction
	if (m_carEventsLua.valid())
	{
		try {

			sol::state_view sv(m_carEventsLua.lua_state());
			sol::table tbl = sv.create_table_with(
				"other", other,
				"position", LuaPropertyRef(collResult.location),
				"normal", LuaPropertyRef(collResult.normal),
				"strikeVel", LuaPropertyRef(strikeVel),
				"infiniteMass", LuaPropertyRef(infiniteMass)
			);
			m_carEventsLua.call(this, "HitCar", tbl);
		}
		catch (const sol::error& e)
		{
			MsgError("CCar event call error: %s\n", e.what());
		}
	}

	// apply force to car 0
	if (!infiniteMass)
	{
		int strength1;

		if (m_controlType == CONTROL_TYPE_PURSUER_AI && other->m_controlType != CONTROL_TYPE_LEAD_AI && other->m_hndType != 0)
			strength1 = (strikeVel * (7 - gCopDifficultyLevel)) / 8;
		else if (m_controlType == CONTROL_TYPE_LEAD_AI && other->m_hndType != 0)
			strength1 = (strikeVel * 5) / 8;
		else
			strength1 = strikeVel;


		strength1 = FIXEDH(strength1) * doFactor >> 3;

		VECTOR_NOPAD velocity;
		velocity = (collResult.normal >> 3) * strength1 >> 6;

		delta.n.linearVelocity[0] -= velocity.vx;
		delta.n.linearVelocity[1] -= velocity.vy;
		delta.n.linearVelocity[2] -= velocity.vz;

		const int twistY = m_cosmetics.twistRateY / 2;

		VECTOR_NOPAD torque;
		torque[0] = FIXEDH(velocity.vy * lever[2] - velocity.vz * lever[1]) * twistY;
		torque[1] = FIXEDH(velocity.vz * lever[0] - velocity.vx * lever[2]) * twistY;
		torque[2] = FIXEDH(velocity.vx * lever[1] - velocity.vy * lever[0]) * twistY;

		if (other->m_controlType == CONTROL_TYPE_LEAD_AI)
		{
			torque[0] = 0;
			torque[2] = 0;
		}

		delta.n.angularVelocity[0] += torque[0];
		delta.n.angularVelocity[1] += torque[1];
		delta.n.angularVelocity[2] += torque[2];
	}
}
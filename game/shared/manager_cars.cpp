#include "core/ignore_vc_new.h"

#include <sol/sol.hpp>
#include <nstd/Array.hpp>

#include "core/cmdlib.h"

#include "renderer/gl_renderer.h"

#include "math/psx_math_types.h"
#include "math/ratan2.h"

#include "world.h"
#include "manager_cars.h"
#include "cars.h"


// TODO: move it
int	CameraCnt = 0;

static CManager_Cars s_carManagerInstance;
CManager_Cars* g_cars = &s_carManagerInstance;

/*static*/ void	CManager_Cars::Lua_Init(sol::state& lua)
{
	lua.new_usertype<CAR_COSMETICS>(
		"CAR_COSMETICS",
		"headLight", &CAR_COSMETICS::headLight,
		"frontInd", &CAR_COSMETICS::frontInd,
		"backInd", &CAR_COSMETICS::backInd,
		"brakeLight", &CAR_COSMETICS::brakeLight,
		"revLight", &CAR_COSMETICS::revLight,
		"policeLight", &CAR_COSMETICS::policeLight,
		"exhaust", &CAR_COSMETICS::exhaust,
		"smoke", &CAR_COSMETICS::smoke,
		"fire", &CAR_COSMETICS::fire,
		"wheelDisp", &CAR_COSMETICS::wheelDisp,
		"extraInfo", &CAR_COSMETICS::extraInfo,
		"powerRatio", &CAR_COSMETICS::powerRatio,
		"cbYoffset", &CAR_COSMETICS::cbYoffset,
		"susCoeff", &CAR_COSMETICS::susCoeff,
		"traction", &CAR_COSMETICS::traction,
		"wheelSize", &CAR_COSMETICS::wheelSize,
		"cPoints", &CAR_COSMETICS::cPoints,
		"colBox", &CAR_COSMETICS::colBox,
		"cog", &CAR_COSMETICS::cog,
		"twistRateX", &CAR_COSMETICS::twistRateX,
		"twistRateY", &CAR_COSMETICS::twistRateY,
		"twistRateZ", &CAR_COSMETICS::twistRateZ,
		"mass", &CAR_COSMETICS::mass);

	lua.new_usertype<CManager_Cars>(
		"CManager_Cars",
		"UpdateControl", &CManager_Cars::UpdateControl,
		"GlobalTimeStep", &CManager_Cars::GlobalTimeStep,
		"DoScenaryCollisions", &CManager_Cars::DoScenaryCollisions);

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Cars"] = g_cars;
}

CCar* CManager_Cars::Create(CAR_COSMETICS* cosmetic, int control, int palette, int controlType, POSITION_INFO& positionInfo)
{
	// not valid request
	if (control == CONTROL_TYPE_NONE)
		return nullptr;

	CCar* cp = new CCar();
	VECTOR_NOPAD tmpStart;

	//memset(cp, 0, sizeof(CCar));

	cp->m_wasOnGround = 1;

	cp->m_id = m_carIdCnt++;

	cp->m_ap.model = 0;// model;
	cp->m_ap.palette = palette;
	cp->m_lowDetail = -1;
	cp->m_ap.qy = 0;
	cp->m_ap.qw = 0;
	cp->m_ap.carCos = cosmetic;

	tmpStart.vx = positionInfo.position.vx;
	tmpStart.vy = positionInfo.position.vy;
	tmpStart.vz = positionInfo.position.vz;

	tmpStart.vy = CWorld::MapHeight(tmpStart) - cp->m_ap.carCos->wheelDisp[0].vy;

	cp->InitCarPhysics((LONGVECTOR4*)&tmpStart, positionInfo.direction);
	cp->m_controlType = control;

	switch (control)
	{
		case CONTROL_TYPE_PLAYER:
		case CONTROL_TYPE_CUTSCENE:
			// player car or cutscene car
			cp->m_ai.padid = 0;// extraData;

			//player[cp->m_id].worldCentreCarId = cp->m_id;
			cp->m_hndType = 0;
			break;
#if 0
		case CONTROL_TYPE_CIV_AI:
			cp->m_hndType = 1;

			if (extraData == NULL)
			{
				cp->m_controlFlags = 0;
				cp->m_ap.palette = 0;
			}
			else
			{
				cp->m_controlFlags = ((EXTRA_CIV_DATA*)extraData)->controlFlags;
				cp->m_ap.palette = ((EXTRA_CIV_DATA*)extraData)->palette;
			}

			InitCivState(cp, (EXTRA_CIV_DATA*)extraData);

			break;
		case CONTROL_TYPE_PURSUER_AI:
			InitCopState(cp, extraData);
			cp->m_ap.palette = 0;
			numCopCars++;
			break;
		case CONTROL_TYPE_LEAD_AI:
			// free roamer lead car
			InitLead(cp);
			leadCarId = cp->m_id;
			cp->hndType = 5;
			break;
#endif
	}

	cp->CreateDentableCar();
	cp->DentCar();

	active_cars.append(cp);

	return cp;
}

void CManager_Cars::UpdateControl()
{
	for (int i = 0; i < active_cars.size(); i++)
	{
		CCar* cp = active_cars[i];

#if 0
		// Update player inputs
		switch (cp->m_controlType)
		{
			case CONTROL_TYPE_PLAYER:
				t0 = Pads[*cp->ai.padid].mapped;	// [A] padid might be wrong
				t1 = Pads[*cp->ai.padid].mapanalog[2];
				t2 = Pads[*cp->ai.padid].type & 4;

				// [A] handle REDRIVER2 dedicated car exit button
				if (t0 & CAR_PAD_LEAVECAR_DED)
				{
					t0 &= ~CAR_PAD_LEAVECAR_DED;
					t0 |= CAR_PAD_LEAVECAR;
				}

				if (NoPlayerControl == 0)
				{
					if (gStopPadReads)
					{
						t0 = CAR_PAD_BRAKE;

						if (cp->hd.wheel_speed <= 0x9000)
							t0 = CAR_PAD_HANDBRAKE;

						t1 = 0;
						t2 = 1;
					}

					cjpRecord(*cp->ai.padid, &t0, &t1, &t2);
				}
				else
				{
					cjpPlay(*cp->ai.padid, &t0, &t1, &t2);
				}

				ProcessCarPad(cp, t0, t1, t2);
				break;
			case CONTROL_TYPE_CIV_AI:
				CivControl(cp);
				break;
			case CONTROL_TYPE_PURSUER_AI:
				CopControl(cp);
				break;
			case CONTROL_TYPE_LEAD_AI:
				t2 = 0;
				t1 = 0;
				t0 = 0;

				t0 = FreeRoamer(cp);

				if (t0 == 0)
				{
					cp->handbrake = 1;
					cp->wheel_angle = 0;
				}
				else
				{
					ProcessCarPad(cp, t0, t1, t2);
				}

				break;
			case CONTROL_TYPE_CUTSCENE:
				if (!_CutRec_RecordPad(cp, &t0, &t1, &t2))
					cjpPlay(-*cp->ai.padid, &t0, &t1, &t2);

				ProcessCarPad(cp, t0, t1, t2);
		}
#endif
		cp->StepCarPhysics();
	}
}

void CManager_Cars::GlobalTimeStep()
{
	Array<RigidBodyState> _tp;
	Array<RigidBodyState> _d0;
	Array<RigidBodyState> _d1;

	_tp.resize(active_cars.size());
	_d0.resize(active_cars.size());
	_d1.resize(active_cars.size());

	int mayBeCollidingBits;
	int howHard;
	int tmp;
	RigidBodyState* thisState_i;
	RigidBodyState* thisState_j;
	RigidBodyState* thisDelta;
	CCar* cp;
	CCar* c1;
	RigidBodyState* tp;
	RigidBodyState* d0;
	RigidBodyState* d1;
	LONGQUATERNION delta_orientation;
	LONGVECTOR4 normal, collisionpoint;
	LONGVECTOR4 AV, lever0, lever1, torque, pointVel0;
	VECTOR_NOPAD velocity;
	int RKstep, subframe;
	int i, j;
	int do1, do2;
	int m1, m2;
	int strikeVel, strength, depth;
	int carsDentedThisFrame;
	short* felony;

#if 0
	if (player[0].playerCarId < 0)
		felony = &pedestrianFelony;
	else
		felony = &car_data[player[0].playerCarId].felonyRating;
#endif
	StepCars();
	CheckCarToCarCollisions();

	// step car forces (when no collisions with them)
	// TODO: made into CCar::StepRigidBody()
	for (i = 0; i < active_cars.size(); i++)
	{
		cp = active_cars[i];

		RigidBodyState& st = cp->m_st;

		st.n.linearVelocity[0] += cp->m_hd.acc[0];
		st.n.linearVelocity[1] += cp->m_hd.acc[1];
		st.n.linearVelocity[2] += cp->m_hd.acc[2];

		st.n.angularVelocity[0] += cp->m_hd.aacc[0];
		st.n.angularVelocity[1] += cp->m_hd.aacc[1];
		st.n.angularVelocity[2] += cp->m_hd.aacc[2];

		cp->m_hd.aacc[0] = 0;
		cp->m_hd.aacc[1] = 0;
		cp->m_hd.aacc[2] = 0;

		if (st.n.linearVelocity[1] > 200000) // reduce vertical velocity
			st.n.linearVelocity[1] = (st.n.linearVelocity[1] * 3) / 4;

		if (cp->m_hd.speed == 0)
		{
			if (abs(st.n.linearVelocity[0]) + abs(st.n.linearVelocity[1]) + abs(st.n.linearVelocity[2]) < 1000)
			{
				st.n.linearVelocity[0] = 0;
				st.n.linearVelocity[1] = 0;
				st.n.linearVelocity[2] = 0;
				st.n.angularVelocity[0] = 0;
				st.n.angularVelocity[1] = 0;
				st.n.angularVelocity[2] = 0;
			}
			else
			{
				cp->m_hd.speed = 1;
			}
		}

		// limit angular velocity
		tmp = 0x800000;
		if ((tmp < st.n.angularVelocity[0]) || (tmp = -tmp, st.n.angularVelocity[0] < tmp))
			st.n.angularVelocity[0] = tmp;

		tmp = 0x800000;
		if ((tmp < st.n.angularVelocity[1]) || (tmp = -tmp, st.n.angularVelocity[1] < tmp))
			st.n.angularVelocity[1] = tmp;

		tmp = 0x800000;
		if ((tmp < st.n.angularVelocity[2]) || (tmp = -tmp, st.n.angularVelocity[2] < tmp))
			st.n.angularVelocity[2] = tmp;

		// without precision
		if (!cp->m_hd.mayBeColliding)
		{
			LONGQUATERNION& orient = st.n.orientation;	// LONGQUATERNION

			st.n.fposition[0] += st.n.linearVelocity[0] >> 8;
			st.n.fposition[1] += st.n.linearVelocity[1] >> 8;
			st.n.fposition[2] += st.n.linearVelocity[2] >> 8;

			AV[0] = FixHalfRound(st.n.angularVelocity[0], 13);
			AV[1] = FixHalfRound(st.n.angularVelocity[1], 13);
			AV[2] = FixHalfRound(st.n.angularVelocity[2], 13);

			// TODO: MulQuaternions macro
			delta_orientation[0] = -orient[1] * AV[2] + orient[2] * AV[1] + orient[3] * AV[0];
			delta_orientation[1] = orient[0] * AV[2] - orient[2] * AV[0] + orient[3] * AV[1];
			delta_orientation[2] = -orient[0] * AV[1] + orient[1] * AV[0] + orient[3] * AV[2];
			delta_orientation[3] = -orient[0] * AV[0] - orient[1] * AV[1] - orient[2] * AV[2];

			orient[0] += FIXEDH(delta_orientation[0]);
			orient[1] += FIXEDH(delta_orientation[1]);
			orient[2] += FIXEDH(delta_orientation[2]);
			orient[3] += FIXEDH(delta_orientation[3]);

			cp->RebuildCarMatrix(st);
		}
	}

	// do collision interactions
	for (subframe = 0; subframe < 4; subframe++)
	{
		RKstep = 0;

		for (RKstep = 0; RKstep < 2; RKstep++)
		{
			for (i = 0; i < active_cars.size(); i++)
			{
				cp = active_cars[i];

				// check collisions with buildings
				if (RKstep != 0 && (subframe & 1) != 0 && cp->m_controlType == CONTROL_TYPE_PLAYER)
				{
#if 0
					CWorld::CheckScenaryCollisions(cp);
#endif
				}

				mayBeCollidingBits = cp->m_hd.mayBeColliding;

				// if has any collision, process with double precision
				if (mayBeCollidingBits)
				{
					if (RKstep == 0)
					{
						thisState_i = &cp->m_st;
						thisDelta = _d0;
					}
					else
					{
						thisState_i = &_tp[i];
						thisDelta = _d1;
					}

					LONGQUATERNION& orient = thisState_i->n.orientation;	// LONGQUATERNION

					thisDelta[i].n.fposition[0] = thisState_i->n.linearVelocity[0] >> 8;
					thisDelta[i].n.fposition[1] = thisState_i->n.linearVelocity[1] >> 8;
					thisDelta[i].n.fposition[2] = thisState_i->n.linearVelocity[2] >> 8;

					AV[0] = FixHalfRound(thisState_i->n.angularVelocity[0], 13);
					AV[1] = FixHalfRound(thisState_i->n.angularVelocity[1], 13);
					AV[2] = FixHalfRound(thisState_i->n.angularVelocity[2], 13);

					thisDelta[i].n.orientation[0] = FIXEDH(-orient[1] * AV[2] + orient[2] * AV[1] + orient[3] * AV[0]);
					thisDelta[i].n.orientation[1] = FIXEDH(orient[0] * AV[2] - orient[2] * AV[0] + orient[3] * AV[1]);
					thisDelta[i].n.orientation[2] = FIXEDH(-orient[0] * AV[1] + orient[1] * AV[0] + orient[3] * AV[2]);
					thisDelta[i].n.orientation[3] = FIXEDH(-orient[0] * AV[0] - orient[1] * AV[1] - orient[2] * AV[2]);

					thisDelta[i].n.linearVelocity[0] = 0;
					thisDelta[i].n.linearVelocity[1] = 0;
					thisDelta[i].n.linearVelocity[2] = 0;
					thisDelta[i].n.angularVelocity[0] = 0;
					thisDelta[i].n.angularVelocity[1] = 0;
					thisDelta[i].n.angularVelocity[2] = 0;
#if 0 // car vs car collisions - disabled for now
					for (j = 0; j < i; j++)
					{
						c1 = active_cars[j];

						// [A] optimized run to not use the box checking
						// as it has already composed bitfield / pairs
						if ((mayBeCollidingBits & (1 << CAR_INDEX(c1))) != 0 && (c1->m_hd.speed != 0 || cp->m_hd.speed != 0))
						{
							if (CarCarCollision3(cp, c1, &depth, (VECTOR*)collisionpoint, (VECTOR*)normal))
							{
								if (RKstep > 0)
									thisState_j = &_tp[j];
								else
									thisState_j = &c1->m_st;

								int c1InfiniteMass;
								int c2InfiniteMass;

								collisionpoint[1] -= 0;

								lever0[0] = collisionpoint[0] - cp->m_hd.where.t[0];
								lever0[1] = collisionpoint[1] - cp->m_hd.where.t[1];
								lever0[2] = collisionpoint[2] - cp->m_hd.where.t[2];

								lever1[0] = collisionpoint[0] - c1->m_hd.where.t[0];
								lever1[1] = collisionpoint[1] - c1->m_hd.where.t[1];
								lever1[2] = collisionpoint[2] - c1->m_hd.where.t[2];

								strength = 47 - (lever0[1] + lever1[1]) / 2;

								lever0[1] += strength;
								lever1[1] += strength;

								strikeVel = depth * 0xc000;

								pointVel0[0] = (FIXEDH(thisState_i->n.angularVelocity[1] * lever0[2] - thisState_i->n.angularVelocity[2] * lever0[1]) + thisState_i->n.linearVelocity[0]) -
									(FIXEDH(thisState_j->n.angularVelocity[1] * lever1[2] - thisState_j->n.angularVelocity[2] * lever1[1]) + thisState_j->n.linearVelocity[0]);

								pointVel0[1] = (FIXEDH(thisState_i->n.angularVelocity[2] * lever0[0] - thisState_i->n.angularVelocity[0] * lever0[2]) + thisState_i->n.linearVelocity[1]) -
									(FIXEDH(thisState_j->n.angularVelocity[2] * lever1[0] - thisState_j->n.angularVelocity[0] * lever1[2]) + thisState_j->n.linearVelocity[1]);

								pointVel0[2] = (FIXEDH(thisState_i->n.angularVelocity[0] * lever0[1] - thisState_i->n.angularVelocity[1] * lever0[0]) + thisState_i->n.linearVelocity[2]) -
									(FIXEDH(thisState_j->n.angularVelocity[0] * lever1[1] - thisState_j->n.angularVelocity[1] * lever1[0]) + thisState_j->n.linearVelocity[2]);

								howHard = (pointVel0[0] / 256) * (normal[0] / 32) +
									(pointVel0[1] / 256) * (normal[1] / 32) +
									(pointVel0[2] / 256) * (normal[2] / 32);

								if (howHard > 0 && RKstep > -1)
								{
									if (DamageCar3D(c1, &lever1, howHard >> 1, cp))
										c1->m_ap.needsDenting = 1;

									if (DamageCar3D(cp, &lever0, howHard >> 1, c1))
										cp->m_ap.needsDenting = 1;

									if (howHard > 0x32000)
									{
										if (cp->m_controlType == CONTROL_TYPE_CIV_AI)
											cp->m_ai.c.carMustDie = 1;

										if (c1->m_controlType == CONTROL_TYPE_CIV_AI)
											c1->m_ai.c.carMustDie = 1;
									}

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

#if 0
									if (howHard > 0x1b00)
									{
										velocity.vy = -17;
										velocity.vx = FIXED(cp->m_st.n.linearVelocity[0]);
										velocity.vz = FIXED(cp->m_st.n.linearVelocity[2]);

										collisionpoint[1] = -collisionpoint[1];

										if (cp->m_controlType == CONTROL_TYPE_PLAYER || c1->m_controlType == CONTROL_TYPE_PLAYER)
										{
											Setup_Sparks((VECTOR*)collisionpoint, &velocity, 6, 0);

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

											Setup_Debris((VECTOR*)collisionpoint, &velocity, 3, 0);
											Setup_Debris((VECTOR*)collisionpoint, &velocity, 6, debris1 << 0x10);
											Setup_Debris((VECTOR*)collisionpoint, &velocity, 2, debris2 << 0x10);
										}
									}
#endif
								}

								strikeVel += (howHard * 9) / 4;

								if (strikeVel > 0x69000)
									strikeVel = 0x69000;

								m1 = cp->m_ap.carCos->mass;
								m2 = c1->m_ap.carCos->mass;

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

								c1InfiniteMass = cp->m_controlType == CONTROL_TYPE_CUTSCENE || m1 == 0x7fff;
								c2InfiniteMass = c1->m_controlType == CONTROL_TYPE_CUTSCENE || m2 == 0x7fff;

								// [A] if any checked cars has infinite mass, reduce bouncing
								// TODO: very easy difficulty
								if (c1InfiniteMass || c2InfiniteMass)
									strikeVel = strikeVel * 10 >> 2;

								// apply force to car 0
								if (!c1InfiniteMass)
								{
									int twistY, strength1;

									if (cp->m_controlType == CONTROL_TYPE_PURSUER_AI && c1->m_controlType != CONTROL_TYPE_LEAD_AI && c1->m_hndType != 0)
										strength1 = (strikeVel * (7 - gCopDifficultyLevel)) / 8;
									else if (cp->m_controlType == CONTROL_TYPE_LEAD_AI && c1->m_hndType != 0)
										strength1 = (strikeVel * 5) / 8;
									else
										strength1 = strikeVel;

									strength1 = FIXEDH(strength1) * do1 >> 3;

									velocity.vx = (normal[0] >> 3) * strength1 >> 6;
									velocity.vz = (normal[2] >> 3) * strength1 >> 6;
									velocity.vy = (normal[1] >> 3) * strength1 >> 6;

									thisDelta[i].n.linearVelocity[0] -= velocity.vx;
									thisDelta[i].n.linearVelocity[1] -= velocity.vy;
									thisDelta[i].n.linearVelocity[2] -= velocity.vz;

									twistY = cp->m_ap.carCos->twistRateY / 2;

									torque[0] = FIXEDH(velocity.vy * lever0[2] - velocity.vz * lever0[1]) * twistY;
									torque[1] = FIXEDH(velocity.vz * lever0[0] - velocity.vx * lever0[2]) * twistY;
									torque[2] = FIXEDH(velocity.vx * lever0[1] - velocity.vy * lever0[0]) * twistY;

									if (c1->m_controlType == CONTROL_TYPE_LEAD_AI)
									{
										torque[0] = 0;
										torque[2] = 0;
									}

									thisDelta[i].n.angularVelocity[0] += torque[0];
									thisDelta[i].n.angularVelocity[1] += torque[1];
									thisDelta[i].n.angularVelocity[2] += torque[2];
								}

								// apply force to car 1
								if (!c2InfiniteMass)
								{
									int twistY, strength2;

									if (cp->m_controlType == CONTROL_TYPE_PURSUER_AI && c1->m_controlType != CONTROL_TYPE_LEAD_AI && c1->m_hndType != 0)
										strength2 = (strikeVel * (7 - gCopDifficultyLevel)) / 8;
									else if (c1->m_controlType == CONTROL_TYPE_LEAD_AI && cp->m_hndType != 0)
										strength2 = (strikeVel * 5) / 8;
									else
										strength2 = strikeVel;

									strength2 = FIXEDH(strength2) * do2 >> 3;

									velocity.vx = (normal[0] >> 3) * strength2 >> 6;
									velocity.vy = (normal[1] >> 3) * strength2 >> 6;
									velocity.vz = (normal[2] >> 3) * strength2 >> 6;

									thisDelta[j].n.linearVelocity[0] += velocity.vx;
									thisDelta[j].n.linearVelocity[1] += velocity.vy;
									thisDelta[j].n.linearVelocity[2] += velocity.vz;

									twistY = c1->m_ap.carCos->twistRateY / 2;

									torque[0] = FIXEDH(lever1[1] * velocity.vz - lever1[2] * velocity.vy) * twistY;
									torque[1] = FIXEDH(lever1[2] * velocity.vx - lever1[0] * velocity.vz) * twistY;
									torque[2] = FIXEDH(lever1[0] * velocity.vy - lever1[1] * velocity.vx) * twistY;

									if (c1->m_controlType == CONTROL_TYPE_LEAD_AI)
									{
										torque[0] = 0;
										torque[2] = 0;
									}

									thisDelta[j].n.angularVelocity[0] += torque[0];
									thisDelta[j].n.angularVelocity[1] += torque[1];
									thisDelta[j].n.angularVelocity[2] += torque[2];
								}

								if (cp->m_id == player[0].playerCarId || c1->m_id == player[0].playerCarId)
									RegisterChaseHit(cp->m_id, c1->m_id);

								if (cp->m_id == player[0].playerCarId)
									CarHitByPlayer(c1, howHard);

								if (c1->m_id == player[0].playerCarId)
									CarHitByPlayer(cp, howHard);
							}
						} // maybe colliding
					} // j loop

#endif
				}
			}

			// update forces and rebuild matrix of the cars
			for (i = 0; i < active_cars.size(); i++)
			{
				cp = active_cars[i];

				// if has any collision, process with double precision
				if (cp->m_hd.mayBeColliding)
				{
					RigidBodyState& st = cp->m_st;
					tp = &_tp[i];
					d0 = &_d0[i];
					d1 = &_d1[i];

					if (RKstep == 0)
					{
						for (j = 0; j < 13; j++)
						{
							tp->v[j] = st.v[j] + (d0->v[j] >> 2);
						}

						cp->RebuildCarMatrix(*tp);
					}
					else if (RKstep == 1)
					{
						for (j = 0; j < 13; j++)
						{
							st.v[j] += d0->v[j] + d1->v[j] >> 3;
						}

						cp->RebuildCarMatrix(st);
					}
				}
			}
		}
	}

	// second sub frame passed, update matrices and physics direction
	// dent cars - no more than 5 cars in per frame
	carsDentedThisFrame = 0;

	for (i = 0; i < active_cars.size(); i++)
	{
		cp = active_cars[i];

		cp->UpdateCarDrawMatrix();
#if 0
		if (cp->m_ap.needsDenting != 0 && ((CameraCnt + i & 3U) == 0 || carsDentedThisFrame < 5))
		{
			cp->DentCar();

			cp->m_ap.needsDenting = 0;
			carsDentedThisFrame++;
		}
#endif
		cp->m_hd.direction = ratan2(cp->m_hd.where.m[0][2], cp->m_hd.where.m[2][2]);
	}
}

void CManager_Cars::CheckScenaryCollisions(CCar* cp)
{
	// UNIMPLEMENTED!!!
}

void CManager_Cars::CheckCarToCarCollisions()
{
	// UNIMPLEMENTED!!!
}

void CManager_Cars::DoScenaryCollisions()
{
	CCar* cp;

	auto i = active_cars.end();

	do
	{
		cp = *i;
		// civ AI and dead cop cars perform less collision detection frames
		if (cp->m_controlType == CONTROL_TYPE_CIV_AI ||
			cp->m_controlType == CONTROL_TYPE_PURSUER_AI && cp->m_ai.p.dying > 85)
		{
			if (cp->m_totalDamage != 0 && (cp->m_hd.speed > 10 || (cp->m_id + CameraCnt & 3) == 0))
			{
				CheckScenaryCollisions(cp);
			}
		}
		else
		{
			CheckScenaryCollisions(cp);
		}

	} while (i == active_cars.begin());
}

void CManager_Cars::StepCars()
{
	for (int i = 0; i < active_cars.size(); i++)
	{
		CCar* cp = active_cars[i];
		cp->StepOneCar();
		cp->ControlCarRevs();
	}
}

void CManager_Cars::DrawAllCars()
{
	for (int i = 0; i < active_cars.size(); i++)
	{
		CCar* cp = active_cars[i];
		cp->DrawCar();
	}
}
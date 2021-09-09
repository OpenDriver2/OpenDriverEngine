#include "game/pch.h"
#include "manager_cars.h"

// TODO: move it
int	CameraCnt = 0;

extern CBaseLevelMap* g_levMap;

static CManager_Cars s_carManagerInstance;
CManager_Cars* g_cars = &s_carManagerInstance;

/*static*/ void	CManager_Cars::Lua_Init(sol::state& lua)
{
	CCar::Lua_Init(lua);

	{
		LUADOC_GLOBAL();

		{
			LUADOC_TYPE();
			lua.new_usertype<POSITION_INFO>(
				LUADOC_T("POSITION_INFO", "Car creation position info"),
				sol::call_constructor, sol::factories(
					[](const int& x, const int& y, const int& z, const int& direction) {
						return POSITION_INFO{ VECTOR_NOPAD{x, y, z}, direction };
					},
					[](const sol::table& table) {
						return POSITION_INFO{ VECTOR_NOPAD{ table["x"], table["y"].get_or(0), table["z"] }, table["direction"].get_or(0) };
					},
					[]() { return POSITION_INFO{ 0 }; }),
				LUADOC_P("position"), &POSITION_INFO::position,
				LUADOC_P("direction"), &POSITION_INFO::direction
			);
		}

		//-------------------------------------------
		{
			LUADOC_TYPE();
			lua.new_usertype<CManager_Cars>(
				LUADOC_T("CManager_Cars", "Car manager"),

				LUADOC_M("LoadModel", "loads car model with specified index"),
				[](CManager_Cars& self, int residentModel) {
					return self.LoadModel(residentModel);
				},

				LUADOC_M("LoadCosmeticsFileD2", "Loads specified LCF file and cosmetic index"),
				[](CManager_Cars& self, std::string& filename, int residentModel, sol::this_state s) {
					sol::state_view lua(s);
					CarCosmetics cosmetic;
					bool result = self.LoadDriver2CosmeticsFile(cosmetic, filename.c_str(), residentModel);

					if (result)
						return sol::make_object(lua, cosmetic);
					return sol::make_object(lua, sol::nil);
				},

				LUADOC_M("LoadCosmeticsFileD1", "Loads specified LCF file and cosmetic index"),
				[](CManager_Cars& self, std::string& filename, int cosmeticIdx, sol::this_state s) {
					sol::state_view lua(s);
					CarCosmetics cosmetic;
					bool result = self.LoadDriver1CosmeticsFile(cosmetic, filename.c_str(), cosmeticIdx);

					if (result)
						return sol::make_object(lua, cosmetic);
					return sol::make_object(lua, sol::nil);
				},

				LUADOC_P("carModels", "Loaded car models table"),
				sol::property([](CManager_Cars& self, sol::this_state s)
				{
					sol::state_view lua(s);
					auto& table = lua.create_table();

					for (usize i = 0; i < self.m_carModels.size(); i++)
						table[i + 1] = self.m_carModels[i];

					return table;
				}),

				LUADOC_M("UnloadAllModels", "removes all cars and unload all models"), 
				&UnloadAllModels,

				LUADOC_M("Create", "create new car. <PARAMS>(cosmetic: CAR_COSMETICS, control: number, modelId: number, positionInfo: POSITION_INFO)"), 
				&Create,

				LUADOC_M("Remove", "removes specific car"), 
				&Remove,

				LUADOC_M("RemoveAll", "Deletes all car from the world"),
				&RemoveAll,

				LUADOC_M("UpdateControl", "updates car controls. Must be called before GlobalTimeStep"), 
				&UpdateControl,

				LUADOC_M("GlobalTimeStep", "updates car physics globally"), 
				&GlobalTimeStep,

				LUADOC_P("soundSourceGetCallback"), 
				&CManager_Cars::m_soundSourceGetCbLua,

				LUADOC_P("eventCallback"),
				&CManager_Cars::m_carEventsLua
				
			);
		}
	}

	auto engine = lua["engine"].get_or_create<sol::table>();

	engine["Cars"] = g_cars;
}

extern CDriverLevelModels g_levModels;

int CManager_Cars::LoadModel(int modelNumber, CDriverLevelModels* levelModels)
{
	// game can load specific car models from other cities
	if (!levelModels)
		levelModels = &g_levModels;

	for (usize i = 0; i < m_carModels.size(); i++)
	{
		// TODO: levelModels check
		if (m_carModels[i]->index == modelNumber)
			return i;
	}

	CarModelData_t* carModel = levelModels->GetCarModel(modelNumber);

	if (!carModel)
		return -1;

	if (!carModel->cleanmodel)
		return -1;

	ModelRef_t* ref = new ModelRef_t();
	ref->model = carModel->cleanmodel;
	ref->size = carModel->cleanSize;
	ref->index = modelNumber;

	CRenderModel* renderModel = new CRenderModel();

	if (renderModel->Initialize(ref))
		ref->userData = renderModel;
	else
		delete renderModel;

	int carModelIndex = m_carModels.size();

	m_carModels.append(ref);

	return carModelIndex;
}

bool CManager_Cars::LoadDriver2CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int residentModel)
{
	if (residentModel < 0 || residentModel >= MAX_CAR_MODELS)
	{
		MsgError("LoadDriver2Cosmetics error: residentModel %d is not valid", residentModel);
		return false;
	}

	FILE* fp = fopen(filename, "rb");
	if (!fp)
	{
		MsgError("Cannot open '%s'\n", filename);
		return false;
	}

	// read whole offsets table first
	int offsetTable[13];
	fread(offsetTable, 1, sizeof(offsetTable), fp);

	CAR_COSMETICS_D2 cosmetics;

	fseek(fp, offsetTable[residentModel], SEEK_SET);
	fread(&cosmetics, sizeof(cosmetics), 1, fp);

	outCosmetics.InitFrom(cosmetics);

	fclose(fp);

	return true;
}

bool CManager_Cars::LoadDriver1CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int cosmeticIndex)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp)
	{
		MsgError("Cannot open '%s'\n", filename);
		return false;
	}

	CAR_COSMETICS_D1 cosmetics;

	fseek(fp, sizeof(CAR_COSMETICS_D1) * cosmeticIndex, SEEK_SET);
	fread(&cosmetics, sizeof(cosmetics), 1, fp);

	outCosmetics.InitFrom(cosmetics);

	fclose(fp);

	return true;
}

void CManager_Cars::UnloadAllModels()
{
	RemoveAll();

	for (usize i = 0; i < m_carModels.size(); i++)
	{
		ModelRef_t* ref = m_carModels[i];

		CRenderModel* renderModel = (CRenderModel*)ref->userData;

		delete renderModel;
		delete ref;
	}
	m_carModels.clear();
}

CCar* CManager_Cars::Create(const CarCosmetics& cosmetic, int control, int modelId, int palette, POSITION_INFO& positionInfo)
{
	// not valid request
	if (control == CONTROL_TYPE_NONE)
		return nullptr;

	CCar* cp = new CCar();
	VECTOR_NOPAD tmpStart;

	cp->m_owner = this;
	cp->m_carEventsLua = m_carEventsLua;	// default callback is manager callback

	cp->m_wasOnGround = 1;

	cp->m_id = m_carIdCnt++;

	cp->m_ap.model = modelId;
	cp->m_ap.palette = palette;
	cp->m_lowDetail = -1;
	cp->m_ap.qy = 0;
	cp->m_ap.qw = 0;
	cp->m_cosmetics = cosmetic;

	cp->m_model = m_carModels[modelId];

	tmpStart.vx = positionInfo.position.vx;
	tmpStart.vy = positionInfo.position.vy;
	tmpStart.vz = positionInfo.position.vz;

	tmpStart.vy = CWorld::MapHeight(tmpStart) - cp->m_cosmetics.wheelDisp[0].vy;

	cp->InitWheelModels();
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

	m_active_cars.append(cp);

	return cp;
}

void CManager_Cars::RemoveAll()
{
	CManager_Players::GetLocalPlayer()->SetCurrentCar(nullptr);

	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];
		delete cp;
	}
	m_active_cars.clear();
}

void CManager_Cars::Remove(CCar* car)
{
	auto& foundCar = m_active_cars.find(car);
	if (*foundCar)
	{
		delete car;
		m_active_cars.remove(foundCar);
	}
}

void CManager_Cars::UpdateControl()
{
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];

		if (cp->m_controlType == CONTROL_TYPE_NONE)
		{
			delete cp;
			m_active_cars.remove(i);
			i--;
			continue;
		}

		if (cp->m_controlType == CONTROL_TYPE_PLAYER)
		{
			CPlayer* player = CManager_Players::GetPlayerByCar(cp);

			if (player)
				player->ProcessCarPad();
		}

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
	int mayBeCollidingBits;
	int howHard;
	int tmp;

	CCar* cp,* c1;
	LONGQUATERNION delta_orientation;
	LONGVECTOR4 AV;
	int carsDentedThisFrame;
	short* felony;

	// global timestep is a starting point for interpolation
	m_lastUpdateTime = m_curUpdateTime;// Time::microTicks();

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
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		cp = m_active_cars[i];

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

			AV[0] = FixDivHalfRound(st.n.angularVelocity[0], 13);
			AV[1] = FixDivHalfRound(st.n.angularVelocity[1], 13);
			AV[2] = FixDivHalfRound(st.n.angularVelocity[2], 13);

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
	for (int subframe = 0; subframe < 4; subframe++)
	{
		for (int RKstep = 0; RKstep < 2; RKstep++)
		{
			for (usize i = 0; i < m_active_cars.size(); i++)
			{
				cp = m_active_cars[i];

				// check collisions with buildings
				if (RKstep != 0 && (subframe & 1) != 0 && cp->m_controlType == CONTROL_TYPE_PLAYER)
				{
					CheckScenaryCollisions(cp);
				}

				mayBeCollidingBits = cp->m_hd.mayBeColliding;

				// if has any collision, process with double precision
				if (mayBeCollidingBits)
				{
					RigidBodyState& cpDelta = cp->m_rbDelta[RKstep];		// thisDelta[i]
					RigidBodyState& cpState = cp->m_rbState[RKstep];		// thisState_i

					LONGQUATERNION& orient = cpState.n.orientation;	// LONGQUATERNION

					// initialize delta frame
					cpDelta.n.fposition[0] = cpState.n.linearVelocity[0] >> 8;
					cpDelta.n.fposition[1] = cpState.n.linearVelocity[1] >> 8;
					cpDelta.n.fposition[2] = cpState.n.linearVelocity[2] >> 8;

					AV[0] = FixDivHalfRound(cpState.n.angularVelocity[0], 13);
					AV[1] = FixDivHalfRound(cpState.n.angularVelocity[1], 13);
					AV[2] = FixDivHalfRound(cpState.n.angularVelocity[2], 13);

					cpDelta.n.orientation[0] = FIXEDH(-orient[1] * AV[2] + orient[2] * AV[1] + orient[3] * AV[0]);
					cpDelta.n.orientation[1] = FIXEDH(orient[0] * AV[2] - orient[2] * AV[0] + orient[3] * AV[1]);
					cpDelta.n.orientation[2] = FIXEDH(-orient[0] * AV[1] + orient[1] * AV[0] + orient[3] * AV[2]);
					cpDelta.n.orientation[3] = FIXEDH(-orient[0] * AV[0] - orient[1] * AV[1] - orient[2] * AV[2]);

					cpDelta.n.linearVelocity[0] = 0;
					cpDelta.n.linearVelocity[1] = 0;
					cpDelta.n.linearVelocity[2] = 0;
					cpDelta.n.angularVelocity[0] = 0;
					cpDelta.n.angularVelocity[1] = 0;
					cpDelta.n.angularVelocity[2] = 0;

					for (usize j = 0; j < i; j++)
					{
						c1 = m_active_cars[j];

						// [A] optimized run to not use the box checking
						// as it has already composed bitfield / pairs
						if ((mayBeCollidingBits & (1 << j)) != 0 && (c1->m_hd.speed != 0 || cp->m_hd.speed != 0))
						{
							cp->CarCarCollision(c1, RKstep);
						} // maybe colliding
					} // j loop
				}
			}

			// update forces and rebuild matrix of the cars
			for (usize i = 0; i < m_active_cars.size(); i++)
			{
				cp = m_active_cars[i];

				// if has any collision, process with double precision
				if (cp->m_hd.mayBeColliding)
				{
					RigidBodyState& st = cp->m_st;
					RigidBodyState& tp = cp->m_rbState[1];
					RigidBodyState& d0 = cp->m_rbDelta[0];
					RigidBodyState& d1 = cp->m_rbDelta[1];

					if (RKstep == 0)
					{
						for (int j = 0; j < 13; j++)
						{
							tp.v[j] = st.v[j] + (d0.v[j] >> 2);
						}

						cp->RebuildCarMatrix(tp);
					}
					else if (RKstep == 1)
					{
						for (int j = 0; j < 13; j++)
						{
							st.v[j] += d0.v[j] + d1.v[j] >> 3;
						}

						cp->RebuildCarMatrix(st);
					}
				}
			}
		}
	}

	// update direction
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		cp = m_active_cars[i];
		cp->m_hd.direction = ratan2(cp->m_hd.where.m[0][2], cp->m_hd.where.m[2][2]);
	}

	DoScenaryCollisions();

	// second sub frame passed, update matrices and physics direction
	// dent cars - no more than 5 cars in per frame
	carsDentedThisFrame = 0;

	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		cp = m_active_cars[i];

		cp->UpdateCarDrawMatrix();
#if 0
		if (cp->m_ap.needsDenting != 0 && ((CameraCnt + i & 3U) == 0 || carsDentedThisFrame < 5))
		{
			cp->DentCar();

			cp->m_ap.needsDenting = 0;
			carsDentedThisFrame++;
		}
#endif
		cp->CheckCarEffects();
	}
}

ISoundSource* CManager_Cars::GetSoundSource(const char* name) const
{
	if (!m_soundSourceGetCbLua.valid())
		return nullptr;

	ISoundSource* result = nullptr;
	try {
		result = m_soundSourceGetCbLua.call(name);
	}
	catch (const sol::error& e)
	{
		MsgError("CManager_Cars::GetSoundSource error: %s\n", e.what());
	}

	return result;
}

void CManager_Cars::CheckScenaryCollisions(CCar* cp)
{
	int queryDist = 580 + cp->m_hd.speed;
	const VECTOR_NOPAD& position = cp->GetPosition();

	CWorld::QueryCollision(position, queryDist, [](BUILDING_BOX& bbox, CELL_OBJECT* co, void* object) {
		CCar* cp = (CCar*)object;
		cp->CarBuildingCollision(bbox, co, 0);
		return true;
	}, cp);
}

void CManager_Cars::CheckCarToCarCollisions()
{
	// recalculate bounding boxes
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];
		BOUND_BOX& bb = cp->m_bbox;

		if (cp->m_controlType == CONTROL_TYPE_NONE 
			/*|| cp->m_controlType == CONTROL_TYPE_PLAYER && playerghost && !playerhitcopsanyway*/) // [A] required as game crashing
		{
			bb.y1 = INT_MAX;
			continue;
		}

		const SVECTOR& colBox = cp->m_cosmetics.colBox;

		int hbod = colBox.vy;
		int lbod = colBox.vz * 9;
		int wbod = colBox.vx * 9;

		int sx = cp->m_hd.where.m[0][0] * wbod / 8;
		int sz = cp->m_hd.where.m[0][2] * lbod / 8;

		int fx = cp->m_hd.where.m[2][0] * wbod / 8;
		int fz = cp->m_hd.where.m[2][2] * lbod / 8;

		int xx = FIXEDH(abs(sz) + abs(sx)) + hbod;
		int zz = FIXEDH(abs(fz) + abs(fx)) + hbod;

		bb.x0 = (cp->m_hd.where.t[0] - xx) / 16;
		bb.z0 = (cp->m_hd.where.t[2] - zz) / 16;
		bb.x1 = (cp->m_hd.where.t[0] + xx) / 16;
		bb.z1 = (cp->m_hd.where.t[2] + zz) / 16;

		if (cp->m_st.n.linearVelocity[0] < 0)
			bb.x0 = (cp->m_hd.where.t[0] - xx) / 16 + FIXEDH(cp->m_st.n.linearVelocity[0]) / 8;
		else
			bb.x1 = (cp->m_hd.where.t[0] + xx) / 16 + FIXEDH(cp->m_st.n.linearVelocity[0]) / 8;

		if (cp->m_st.n.linearVelocity[2] < 0)
			bb.z0 = bb.z0 + (FIXEDH(cp->m_st.n.linearVelocity[2]) / 8);
		else
			bb.z1 = bb.z1 + (FIXEDH(cp->m_st.n.linearVelocity[2]) / 8);

		// [A] 2400 for box size - bye bye collision check performance under bridges
		bb.y0 = (cp->m_hd.where.t[1] - colBox.vy * 2) / 16;
		bb.y1 = (cp->m_hd.where.t[1] + colBox.vy * 4) / 16;

		// make player handled cars always processed with precision
		if (cp->m_hndType == 0)
		{
			cp->m_hd.mayBeColliding = (1 << 31);
		}
	}

	// check boxes intersection with each other
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		BOUND_BOX& bb1 = m_active_cars[i]->m_bbox;

		for (usize j = i+1; j < m_active_cars.size(); j++)
		{
			BOUND_BOX& bb2 = m_active_cars[j]->m_bbox;

			if (bb1.y1 != INT_MAX && bb2.y1 != INT_MAX &&
				bb2.x0 < bb1.x1 && bb2.z0 < bb1.z1 && bb1.x0 < bb2.x1 &&
				bb1.z0 < bb2.z1 && bb2.y0 < bb1.y1 && bb1.y0 < bb2.y1)
			{
				m_active_cars[i]->m_hd.mayBeColliding |= (1 << j);
				m_active_cars[j]->m_hd.mayBeColliding |= (1 << i);
			}
		}
	}
}

void CManager_Cars::DoScenaryCollisions()
{
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];
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
	}
}

void CManager_Cars::StepCars()
{
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];
		cp->StepOneCar();
		cp->ControlCarRevs();
	}
}

void CManager_Cars::DrawAllCars()
{
	for (usize i = 0; i < m_active_cars.size(); i++)
	{
		CCar* cp = m_active_cars[i];
		cp->DrawCar();
	}
}

double CManager_Cars::GetInterpTime() const
{
	const double ticks_to_ms = 1.0 / 1000000.0;
	return double(m_curUpdateTime - m_lastUpdateTime) * ticks_to_ms;
}

void CManager_Cars::Draw(const CameraViewParams& view)
{
	if (g_levRenderProps.nightMode)
		CRenderModel::SetupLightingProperties(g_levRenderProps.nightAmbientScale, g_levRenderProps.nightLightScale);
	else
		CRenderModel::SetupLightingProperties(g_levRenderProps.ambientScale, g_levRenderProps.lightScale);

	g_cars->DrawAllCars();
}

void CManager_Cars::UpdateTime(int64 ticks)
{
	g_cars->m_curUpdateTime = ticks;// Time::microTicks();
}

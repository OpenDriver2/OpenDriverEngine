#pragma once
#include "scripting/esl_luaref.h"
#include "math/psx_math_types.h"

class CViewParams;
class CCar;
class CDriverLevelModels;
class CManager_Cars;
struct CarCosmetics;
struct ModelRef_t;

class ISoundSource;
enum ECarControlType : int;

struct POSITION_INFO
{
	POSITION_INFO() = default;
	POSITION_INFO(const int x, const int y, const int z, const int direction);
	POSITION_INFO(const VECTOR_NOPAD& position, const int direction);
	POSITION_INFO(const esl::LuaTable& table);

	VECTOR_NOPAD position;
	int direction;
};

EQSCRIPT_BIND_TYPE_NO_PARENT(POSITION_INFO, "POSITION_INFO", BY_VALUE)
EQSCRIPT_BIND_TYPE_NO_PARENT(CManager_Cars, "CManager_Cars", BY_REF)

class CManager_Cars
{
	friend class CCar;
	EQSCRIPT_PUBLIC_BINDER(CManager_Cars);
public:
	int						LoadModel(int modelNumber, CDriverLevelModels* levelModels = nullptr);
	bool					LoadDriver2CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int residentModel);
	bool					LoadDriver1CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int cosmeticIndex);

	void					UnloadAllModels();

	CCar*					Create(const CarCosmetics& cosmetic, ECarControlType control, int modelId, int palette, POSITION_INFO& positionInfo);

	void					RemoveAll();
	void					Remove(CCar* car);

	void					UpdateControl();
	void					GlobalTimeStep();

	//------------------------------------------------------

	double					GetInterpTime() const;

	static void				Draw(const CViewParams& view);
	static void				UpdateTime(int64 ticks);

	static void				Lua_Init(const esl::ScriptState& state);
protected:

	void					StepCars();

	void					DoScenaryCollisions();
	void					CheckCarToCarCollisions();

	void					CheckScenaryCollisions(CCar* cp);

	EqStringRef				GetSoundScriptName(const char* name) const;

	Array<CCar*>			m_active_cars{ PP_SL };		// [A] to be renamed as m_carList
	int						m_carIdCnt{ 0 };

	Array<ModelRef_t*>		m_carModels{ PP_SL };	// TEMPORARY; Will use different container!
	int64					m_lastUpdateTime{ 0 };
	int64					m_curUpdateTime{ 0 };
	int						m_lastWorldStep{ -1 };

	esl::LuaFunctionRef		m_soundSourceGetCbLua;
	esl::LuaFunctionRef		m_carEventsLua;
};

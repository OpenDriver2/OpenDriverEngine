#ifndef MANAGER_CARS_H
#define MANAGER_CARS_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

class CCar;
class CDriverLevelModels;
struct CarCosmetics;
struct ModelRef_t;

struct POSITION_INFO
{
	VECTOR_NOPAD position;
	int direction;
};

class ISoundSource;

class CManager_Cars
{
	friend class CCar;
public:
	int						LoadModel(int modelNumber, CDriverLevelModels* levelModels = nullptr);
	bool					LoadDriver2CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int residentModel);
	bool					LoadDriver1CosmeticsFile(CarCosmetics& outCosmetics, const char* filename, int cosmeticIndex);

	void					UnloadAllModels();

	CCar*					Create(const CarCosmetics& cosmetic, int control, int modelId, int palette, POSITION_INFO& positionInfo);

	void					RemoveAll();
	void					Remove(CCar* car);

	void					UpdateControl();
	void					GlobalTimeStep();

	//------------------------------------------------------

	double					GetInterpTime() const;

	void					DrawAllCars();

	static void				Draw(const struct CameraViewParams& view);
	static void				UpdateTime(int64 ticks);

	static void				Lua_Init(sol::state& lua);
protected:

	void					StepCars();

	void					DoScenaryCollisions();
	void					CheckCarToCarCollisions();

	void					CheckScenaryCollisions(CCar* cp);

	ISoundSource*			GetSoundSource(const char* name) const;

	Array<CCar*>			m_active_cars;		// [A] to be renamed as m_carList
	int						m_carIdCnt{ 0 };

	Array<ModelRef_t*>		m_carModels;	// TEMPORARY; Will use different container!
	int64					m_lastUpdateTime{ 0 };
	int64					m_curUpdateTime{ 0 };

	sol::function			m_soundSourceGetCbLua;
	sol::function			m_carEventsLua;
};

#endif // MANAGER_CARS_H
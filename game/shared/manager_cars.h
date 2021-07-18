#ifndef MANAGER_CARS_H
#define MANAGER_CARS_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

class CCar;
class CDriverLevelModels;
struct CAR_COSMETICS;


struct POSITION_INFO
{
	VECTOR_NOPAD position;
	int direction;
};

class CManager_Cars
{
public:
	int				LoadModel(int modelNumber, CDriverLevelModels* levelModels = nullptr);
	CCar*			Create(const CAR_COSMETICS& cosmetic, int control, int modelId, int palette, POSITION_INFO& positionInfo);

	void			RemoveAll();
	void			Remove(CCar* car);

	void			UpdateControl();
	void			GlobalTimeStep();

	void			DoScenaryCollisions();

	//------------------------------------------------------

	double			GetInterpTime() const;

	void			DrawAllCars();

	static void		Draw(const struct CameraViewParams& view);

	static void		Lua_Init(sol::state& lua);
protected:

	void			StepCars();
	void			CheckCarToCarCollisions();

	void			CheckScenaryCollisions(CCar* cp);

	Array<CCar*>		m_active_cars;		// [A] to be renamed as m_carList
	int					m_carIdCnt{ 0 };

	Array<ModelRef_t*>	m_carModels;	// TEMPORARY; Will use different container!
	int64				m_lastUpdateTime{ 0 };
	int64				m_curUpdateTime{ 0 };
};

#endif // MANAGER_CARS_H
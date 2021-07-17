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
	CCar*			Create(CAR_COSMETICS* cosmetic, int control, int modelId, int palette, POSITION_INFO& positionInfo);

	void			UpdateControl();
	void			GlobalTimeStep();

	void			DoScenaryCollisions();

	//------------------------------------------------------

	void			DrawAllCars();

	static void		Draw(const CameraViewParams& view);

	static void		Lua_Init(sol::state& lua);
protected:

	void			StepCars();
	void			CheckCarToCarCollisions();

	void			CheckScenaryCollisions(CCar* cp);

	Array<CCar*>		m_active_cars;		// [A] to be renamed as m_carList
	int					m_carIdCnt{ 0 };

	Array<ModelRef_t*>	m_carModels;	// TEMPORARY; Will use different container!
};

#endif // MANAGER_CARS_H
#ifndef MANAGER_CARS_H
#define MANAGER_CARS_H

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

class CCar;
struct CAR_COSMETICS;
struct POSITION_INFO;

class CManager_Cars
{
public:
	//void			Load(CDriverLevelModels* level);
	//CCar*			Create(CAR_COSMETICS* cosmetic, int palette, int controlType, POSITION_INFO* positionInfo);

	void			UpdateControl();
	void			GlobalTimeStep();

	void			DoScenaryCollisions();

	static void		Lua_Init(sol::state& lua);
protected:

	Array<CCar*>	car_data;		// [A] to be renamed as m_carList
};

#endif // MANAGER_CARS_H
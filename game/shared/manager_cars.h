#ifndef MANAGER_CARS_H
#define MANAGER_CARS_H

class CCar;
struct CAR_COSMETICS;
struct POSITION_INFO;

class CManager_Cars
{
public:
	//void		Load(CDriverLevelModels* level);
	//CCar*		Create(CAR_COSMETICS* cosmetic, int palette, int controlType, POSITION_INFO* positionInfo);

	void		UpdateControl();
	void		GlobalTimeStep();

	void		DoScenaryCollisions();
protected:

	Array<CCar*>	car_data;		// [A] to be renamed as m_carList
	Array<CCar*>	active_cars;	// [A] to be renamed as m_activeCars
};

#endif // MANAGER_CARS_H
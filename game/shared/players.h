#ifndef PLAYERS_H
#define PLAYERS_H

#include <sol/forward.hpp>

class CCar;

enum EPlayerControlType
{
	PlayerControl_Local = 0,
	PlayerControl_NetworkPeer
};

class CPlayer
{
public:
	struct InputData
	{
		bool accel { false };
		bool brake { false };
		bool wheelspin { false };
		bool handbrake { false };
		bool fastSteer { false };
		int steering { 0 };
		int horn { 0 };
	};

	EPlayerControlType		GetControlType() const;

	void					SetCurrentCar(CCar* newCar);
	CCar*					GetCurrentCar() const;

	void					UpdateControls(const InputData& input);

	void					ProcessCarPad();

	static void				Lua_Init(sol::state& lua);

protected:

	InputData				m_currentInputs;
	CCar*					m_currentCar{ nullptr };
	EPlayerControlType		m_controlType{ PlayerControl_Local };
};

//---------------------------------------------------------------------------------
// Player manager
//---------------------------------------------------------------------------------

class CManager_Players
{
public:
	static void				Lua_Init(sol::state& lua);

	static void				Update();

	static CPlayer*			GetLocalPlayer();

	static CPlayer*			GetPlayerByCar(CCar* car);

protected:
	static CPlayer			m_localPlayer;
};

#endif // PLAYERS_H
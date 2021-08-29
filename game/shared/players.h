#ifndef PLAYERS_H
#define PLAYERS_H

#include <sol/forward.hpp>
#include <enet/enet.h>

class CCar;

enum EPlayerControlType
{
	PlayerControl_Local = 0,
	PlayerControl_NetworkPeer
};

class CPlayer
{
	friend class CManager_Players;
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

	void					Net_Init();
	void					Net_Finalize();

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

	Array<ENetAddress>		m_stationAddresses;
	ENetHost*				m_netHost{ nullptr };
};

//---------------------------------------------------------------------------------
// Player manager
//---------------------------------------------------------------------------------

class CManager_Players
{
public:
	static void				Update();

	static CPlayer*			GetLocalPlayer();

	static CPlayer*			GetPlayerByCar(CCar* car);

	static void				Net_Init();
	static void				Net_Finalize();

	static void				Net_Update();

	static void				AddNetworkPlayer(const char* peerAddress);

	//----------------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:
	static CPlayer			LocalPlayer;
};

#endif // PLAYERS_H
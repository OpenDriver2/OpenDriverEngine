#ifndef PLAYERS_H
#define PLAYERS_H

#include <sol/forward.hpp>

class CCar;
class CReplayStream;

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
		// TODO: pedestrian mapping
		bool accel { false };
		bool brake { false };
		bool wheelspin { false };
		bool handbrake { false };
		bool fastSteer { false };
		bool useAnalogue { false };
		int steering { 0 };
		int horn { 0 };
	};

	virtual ~CPlayer();

	void					Net_Init();
	void					Net_Finalize();

	void					InitReplay(CReplayStream* sourceStream = nullptr);

	EPlayerControlType		GetControlType() const;

	void					SetCurrentCar(CCar* newCar);
	CCar*					GetCurrentCar() const;
	
	// TODO: pedestrian

	void					UpdateControls(const InputData& input);
	void					ProcessCarPad();

	static void				Lua_Init(sol::state& lua);

protected:
	InputData						m_currentInputs;
	CCar*							m_currentCar{ nullptr };
	RefCount::Ptr<CReplayStream>	m_playbackStream;
	RefCount::Ptr<CReplayStream>	m_recordStream;
	EPlayerControlType				m_controlType{ PlayerControl_Local };
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

	static CPlayer*			CreatePlayer();
	static void				RemovePlayer(CPlayer* player);

	static void				RemoveAllPlayers();

	static void				Net_Init();
	static void				Net_Finalize();

	static void				Net_Update();

	static void				AddNetworkPlayer(const char* peerAddress);

	//----------------------------------------------------

	static void				Lua_Init(sol::state& lua);

protected:
	static CPlayer			LocalPlayer;
	static Array<CPlayer*>	Players;
};

#endif // PLAYERS_H
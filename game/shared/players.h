#pragma once
#include "math/psx_math_types.h"

class CCar;
class CReplayStream;
using CReplayStreamPtr = CRefPtr<CReplayStream>;

enum EPlayerControlType
{
	PlayerControl_Local = 0,
	PlayerControl_NetworkPeer
};

enum ERubberBandMode
{
	Rubberband_Off = 0,
	Rubberband_Chaser,
	Rubberband_Escape
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

	void				Net_Init();
	void				Net_Finalize();

	void				InitReplay(CReplayStream* sourceStream = nullptr);

	EPlayerControlType	GetControlType() const;

	void				SetCurrentCar(CCar* newCar);
	CCar*				GetCurrentCar() const;
	
	// TODO: pedestrian

	void				SetRubberbandPowerRatio(int powerRatio) { m_rubberbandPowerRatio = powerRatio; }
	int					GetRubberbandPowerRatio() const { return m_rubberbandPowerRatio; }

	void				SetRubberbandPoint(const VECTOR_NOPAD& position) { m_rubberbandPoint = position; }
	const VECTOR_NOPAD&	GetRubberbandPoint() const { return m_rubberbandPoint; }

	void				SetRubberbandMode(ERubberBandMode mode) { m_rubberbandMode = mode; }
	ERubberBandMode		GetRubberbandMode() const { return m_rubberbandMode; }

	void				UpdateControls(const InputData& input);
	void				ProcessCarPad();

	static void			Lua_Init(sol::state& lua);

protected:
	InputData			m_currentInputs;
	CCar*				m_currentCar{ nullptr };
	VECTOR_NOPAD		m_rubberbandPoint{ 0 };
	ERubberBandMode		m_rubberbandMode{ Rubberband_Off };
	int					m_rubberbandPowerRatio{ 4096 };

	CReplayStreamPtr	m_playbackStream;
	CReplayStreamPtr	m_recordStream;
	EPlayerControlType	m_controlType{ PlayerControl_Local };
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

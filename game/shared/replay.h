#pragma once
#include "math/psx_math_types.h"
#include "players.h"

// TODO: mission.h
struct SAVED_PLAYER_POS
{
	ushort type;
	short direction;
	int vx, vy, vz;
	uint felony;
	ushort totaldamage;
	short damage[6];
};

struct SAVED_CAR_POS
{
	char active;
	uint8 model, palette;
	ushort totaldamage;
	ushort damage[6];
	short direction;
	int vx, vy, vz;
};

struct MISSION_DATA
{
	SAVED_PLAYER_POS PlayerPos;
	SAVED_CAR_POS CarPos[6];
};

//-------------------------------------------------

// DO NOT CHANGE
struct STREAM_SOURCE
{
	uint8 type, model, palette;
	char controlType;
	ushort flags;
	ushort rotation;
	VECTOR_NOPAD position;
	int totaldamage;
	int damage[6];
};

//-------------------------------------------------

struct PADRECORD;

class CReplayStream : public RefCountedObject<CReplayStream>
{
public:
	CReplayStream() = default;
	CReplayStream(int bufferSize);

	~CReplayStream();

	CReplayStream*			Clone() const;

	STREAM_SOURCE&			GetSourceParams()  { return m_sourceType; }

	bool					IsEmpty() const { return m_padRecordBuffer == m_initialPadRecordBuffer; }
	bool					IsAtEnd() const { return m_padRecordBuffer == m_padRecordBufferEnd; }
	void					Reset();
	void					Purge();

	// Updates playback. Returns false if out of tape
	bool					Play(CPlayer::InputData& outInputs);

	// Records controls if there is difference. Returns false if out of tape
	bool					Record(CPlayer::InputData& inoutInputs);

protected:
	bool					Put(uint pt0);
	bool					Get(uint& pt0);

	STREAM_SOURCE			m_sourceType;
	PADRECORD*				m_initialPadRecordBuffer{ nullptr };
	PADRECORD*				m_padRecordBuffer{ nullptr };
	PADRECORD*				m_padRecordBufferEnd{ nullptr };
	uint8					m_playbackrun{ 0 };
	int						m_length{ 0 };			// CWorld::StepCnt
	int						m_padCount{ 0 };
	int						m_startStep{ 0 };
};

class CReplayData
{
public:
	static void				Lua_Init(sol::state& lua);
};

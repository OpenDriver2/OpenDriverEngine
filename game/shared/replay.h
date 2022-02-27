#ifndef REPLAY_H
#define REPLAY_H

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
	uchar model, palette;
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
	uchar type, model, palette;
	char controlType;
	ushort flags;
	ushort rotation;
	VECTOR_NOPAD position;
	int totaldamage;
	int damage[6];
};

//-------------------------------------------------

struct PADRECORD;

class CReplayStream
{
public:
	CReplayStream() = default;
	~CReplayStream();

	void					Initialize(int bufferSize);
	void					Cleanup();

	STREAM_SOURCE&			GetSourceParams()  { return m_sourceType; }

	bool					IsEmpty() const { return m_padRecordBuffer == m_initialPadRecordBuffer; }
	bool					IsAtEnd() const { return m_padRecordBuffer == m_padRecordBufferEnd; }
	void					Reset();

	// Updates playback. Returns false if out of tape
	bool					Play(CPlayer::InputData& outInputs);

	// Records a controls if there is difference. Returns false if out of tape
	bool					Record(CPlayer::InputData& inoutInputs);

protected:
	bool					Put(uint pt0);
	bool					Get(uint& pt0);

	STREAM_SOURCE			m_sourceType;
	PADRECORD*				m_initialPadRecordBuffer{ nullptr };
	PADRECORD*				m_padRecordBuffer{ nullptr };
	PADRECORD*				m_padRecordBufferEnd{ nullptr };
	uchar					m_playbackrun{ 0 };
	int						m_length{ 0 };			// CWorld::StepCnt
	int						m_padCount{ 0 };
};

class CReplayData
{
public:
	static void				Lua_Init(sol::state& lua);
};

#endif // REPLAY_H
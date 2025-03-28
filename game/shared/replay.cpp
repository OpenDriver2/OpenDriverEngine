#include "core/core_common.h"
#include "sys/scripting/sys_esl.h"
#include "replay.h"
#include "world.h"
#include "players.h"

static constexpr int DRIVER2_REPLAY_MAGIC = 0x14793209;
static constexpr int REDRIVER2_CHASE_MAGIC = (('D' << 24) | ('2' << 16) | ('C' << 8) | 'R');

static const int8 ReplayAnalogueUnpack[16] = {
	0, -51, -63, -75, -87, -99, -111, -123,
	0,  51,  63,  75,  87,  99,  111,  123
};

// mapped pad identifiers
static constexpr int MPAD_L2		= 0x1;
static constexpr int MPAD_R2		= 0x2;
static constexpr int MPAD_L1		= 0x4;
static constexpr int MPAD_R1		= 0x8;

static constexpr int MPAD_TRIANGLE	= 0x10;
static constexpr int MPAD_CIRCLE	= 0x20;
static constexpr int MPAD_CROSS		= 0x40;
static constexpr int MPAD_SQUARE	= 0x80;

static constexpr int MPAD_SELECT	= 0x100;
static constexpr int MPAD_L3		= 0x200;
static constexpr int MPAD_R3		= 0x400;
static constexpr int MPAD_START		= 0x800;

static constexpr int MPAD_D_UP		= 0x1000;
static constexpr int MPAD_D_RIGHT	= 0x2000;
static constexpr int MPAD_D_DOWN	= 0x4000;
static constexpr int MPAD_D_LEFT	= 0x8000;


//----------------------------------------
// replay file structures

struct REPLAY_PARAMETER_BLOCK
{
	int RecordingEnd;
	VECTOR_NOPAD lead_car_start;
	short Lead_car_dir;
	uint8 timeofday, weather;
};

struct REPLAY_SAVE_HEADER
{
	uint magic;
	uint8 GameLevel;
	uint8 GameType;
	uint8 reserved1;
	uint8 NumReplayStreams, NumPlayers;
	uint8 RandomChase;
	uint8 CutsceneEvent;
	uint8 gCopDifficultyLevel;
	MISSION_DATA SavedData;
	int ActiveCheats;
	int wantedCar[2];
	int MissionNumber;
	int HaveStoredData;
	int reserved2[6];
};

struct REPLAY_STREAM_HEADER
{
	STREAM_SOURCE SourceType;
	int Size, Length;
};

struct PADRECORD
{
	uint8 pad, analogue, run;
};

struct PING_PACKET
{
	ushort frame;
	char carId, cookieCount;
};

struct PLAYBACKCAMERA
{
	VECTOR_NOPAD position;
	SVECTOR angle;
	int FrameCnt;
	short CameraPosvy;
	short scr_z;
	short gCameraMaxDistance;
	short gCameraAngle;
	uint8 cameraview;
	uint8 next, prev, idx;
};

//----------------------------------------

EQSCRIPT_TYPE_BEGIN(STREAM_SOURCE)
	EQSCRIPT_BIND_VAR(type)
	EQSCRIPT_BIND_VAR(model)
	EQSCRIPT_BIND_VAR(palette)
	EQSCRIPT_BIND_VAR(flags)
	EQSCRIPT_BIND_VAR(rotation)
	EQSCRIPT_BIND_VAR(position)
	EQSCRIPT_BIND_VAR(totaldamage)
	//EQSCRIPT_BIND_VAR(damage)		// ESL TODO: FixedArray binding
EQSCRIPT_TYPE_END

EQSCRIPT_TYPE_BEGIN(CReplayStream)
	EQSCRIPT_BIND_FUNC(Reset)
	EQSCRIPT_BIND_FUNC(Purge)
	EQSCRIPT_BIND_FUNC(Clone)

	EQSCRIPT_BIND_FUNC(Play)
	EQSCRIPT_BIND_FUNC(Record)

	EQSCRIPT_BIND_FUNC(GetSourceParams)
	EQSCRIPT_BIND_FUNC(IsEmpty)
	EQSCRIPT_BIND_FUNC(IsAtEnd)
EQSCRIPT_TYPE_END

void CReplayData::Lua_Init(const esl::ScriptState& state)
{
	state.RegisterClass<STREAM_SOURCE>();
	state.RegisterClass<CReplayStream>();
}

//----------------------------------------

static void PackInput(uint& outPad, char& outSteer, char& outType, const PlayerInputData& inputs)
{
	outPad = 0;
	outPad |= inputs.accel ? MPAD_CROSS : 0;
	outPad |= inputs.brake ? MPAD_SQUARE : 0;
	outPad |= inputs.handbrake ? MPAD_TRIANGLE : 0;
	outPad |= inputs.wheelspin ? MPAD_CIRCLE : 0;
	outPad |= inputs.horn ? MPAD_R1 : 0;
	outPad |= inputs.fastSteer ? MPAD_L1 : 0;
	
	outType = inputs.useAnalogue ? 0x4 : 0;

	if (inputs.useAnalogue)
	{
		outSteer = inputs.steering;
		outType = 0x4;
	}
	else
	{
		outPad |= inputs.steering < 0 ? MPAD_D_LEFT : 0;
		outPad |= inputs.steering > 0 ? MPAD_D_RIGHT : 0;
		outType = 0;
	}
}

static void UnpackInput(PlayerInputData& outInputs, uint pad, char steer, char type)
{
	outInputs.accel = pad & MPAD_CROSS;
	outInputs.brake = pad & MPAD_SQUARE;
	outInputs.handbrake = pad & MPAD_TRIANGLE;
	outInputs.wheelspin = pad & MPAD_CIRCLE;
	outInputs.horn = pad & MPAD_R1;
	outInputs.fastSteer = pad & MPAD_L1;
	outInputs.useAnalogue = (type & 0x4);

	if (outInputs.useAnalogue) 
	{
		outInputs.steering = steer;
	}
	else 
	{
		outInputs.steering = 0;
		outInputs.steering -= (pad & MPAD_D_LEFT) ? 1 : 0;
		outInputs.steering += (pad & MPAD_D_RIGHT) ? 1 : 0;
	}
}

CReplayStream::CReplayStream(int bufferSize)
{
	m_initialPadRecordBuffer = PPNew PADRECORD[bufferSize];
	m_padRecordBufferEnd = m_initialPadRecordBuffer + bufferSize;

	Purge();
}

CReplayStream::~CReplayStream()
{
	SAFE_DELETE_ARRAY(m_initialPadRecordBuffer);
	m_length = 0;
	m_padCount = 0;
	Reset();
}

CReplayStream* CReplayStream::Clone() const
{
	const int bufferSize = m_padRecordBuffer - m_initialPadRecordBuffer;

	CReplayStream* cloned = new CReplayStream(bufferSize);

	if(m_initialPadRecordBuffer)
		memcpy(cloned->m_initialPadRecordBuffer, m_initialPadRecordBuffer, bufferSize * sizeof(PADRECORD));

	return cloned;
}

// reset stream position
void CReplayStream::Reset()
{
	m_padRecordBuffer = m_initialPadRecordBuffer;
	m_playbackrun = 0;
}

void CReplayStream::Purge()
{
	memset(&m_sourceType, 0, sizeof(m_sourceType));
	m_startStep = CWorld::StepCount;
	Reset();
}

// update replay
bool CReplayStream::Play(PlayerInputData& outInputs)
{
	// TODO: those Pack/Unpack input functions might be temporary and only for old replay format
	uint outPad;
	char outSteer;
	char outType;

	uint t0;
	bool ret = Get(t0);

	int t1 = (t0 >> 8) & 15;
	outPad = t0 & 0xF0FC;

	if (t1 == 0)
	{
		outSteer = 0;
		outType = 0;
	}
	else
	{
		outSteer = ReplayAnalogueUnpack[t1];
		outType = 4;
	}

	UnpackInput(outInputs, outPad, outSteer, outType);

	return ret;
}

bool CReplayStream::Record(PlayerInputData& inoutInputs)
{
	// TODO: those Pack/Unpack input functions might be temporary and only for old replay format
	uint inoutPad;
	char inoutSteer;
	char inoutType;
	PackInput(inoutPad, inoutSteer, inoutType, inoutInputs);

	int tmp, t1;

	if (inoutType & 0x4)
	{
		if (inoutSteer < -45)
		{
			tmp = -45 - inoutSteer >> 31;
			t1 = (((-45 - inoutSteer) / 6 + tmp >> 1) - tmp) + 1;
		}
		else if (inoutSteer < 46)
		{
			t1 = 8;
		}
		else
		{
			tmp = inoutSteer - 45 >> 31;
			t1 = (((inoutSteer - 45) / 6 + tmp >> 1) - tmp) + 9;
		}
	}
	else
	{
		t1 = 0;
	}

	uint t0 = (t1 & 15) << 8 | inoutPad & 0xF0FC;

	bool ret = Put(t0);
	m_length = CWorld::StepCount - m_startStep;

	t1 = (t0 >> 8) & 15;

	if (t1 == 0)
	{
		inoutSteer = 0;
		inoutType = 0;
	}
	else
	{
		inoutSteer = ReplayAnalogueUnpack[t1];
		inoutType = 0x4;
	}

	inoutPad = t0 & 0xF0FC;

	UnpackInput(inoutInputs, inoutPad, inoutSteer, inoutType);

	return ret;
}

bool CReplayStream::Get(uint& pt0)
{
	if (m_padRecordBuffer + 1 <= m_padRecordBufferEnd)
	{
		pt0 = (m_padRecordBuffer->pad << 8) | m_padRecordBuffer->analogue;

		if (m_playbackrun < m_padRecordBuffer->run)
		{
			m_playbackrun++;
		}
		else
		{
			m_padRecordBuffer++;
			m_playbackrun = 0;
		}

		return true;
	}

	pt0 = 0x10;
	return false;
}

bool CReplayStream::Put(uint t0)
{
	PADRECORD* padbuf;

	if (m_padRecordBuffer + 1 >= m_padRecordBufferEnd)
		return false;

	padbuf = m_padRecordBuffer;

	if ((CWorld::StepCount - m_startStep) != 0 && padbuf->run != 0xEE)
	{
		if (padbuf->pad == ((t0 >> 8) & 255) &&
			padbuf->analogue == (t0 & 255) &&
			padbuf->run != 143)
		{
			padbuf->run++;
			return true;
		}

		padbuf++;

		padbuf->pad = (t0 >> 8) & 255;
		padbuf->analogue = t0 & 255;
		padbuf->run = 0;

		m_padRecordBuffer = padbuf;
		m_padCount++;

		return true;
	}

	padbuf->pad = (t0 >> 8) & 255;
	padbuf->analogue = t0 & 255;
	padbuf->run = 0;

	return true;
}

#if 0
// Loads original cutscene replay inside CUT*.R
int CutRec_LoadCutsceneAsReplayFromBuffer(char* buffer)
{
	REPLAY_SAVE_HEADER* header;
	REPLAY_STREAM_HEADER* sheader;

	char* pt = buffer;

	header = (REPLAY_SAVE_HEADER*)pt;

	if (header->magic != DRIVER2_REPLAY_MAGIC &&
		header->magic != REDRIVER2_CHASE_MAGIC ||	// [A]
		header->NumReplayStreams == 0)
		return 0;

	ReplayStart = replayptr = (char*)_replay_buffer;

	GameLevel = header->GameLevel;
	GameType = (GAMETYPE)header->GameType;

	NumReplayStreams = header->NumReplayStreams;
	NumPlayers = header->NumPlayers;
	gRandomChase = header->RandomChase;
	CutsceneEventTrigger = header->CutsceneEvent;
	gCopDifficultyLevel = header->gCopDifficultyLevel;
	ActiveCheats = header->ActiveCheats; // TODO: restore old value

	wantedCar[0] = header->wantedCar[0];
	wantedCar[1] = header->wantedCar[1];

	memcpy((u_char*)&MissionEndData, (u_char*)&header->SavedData, sizeof(MISSION_DATA));

	pt = (char*)(header + 1);

	int maxLength = 0;
	for (int i = 0; i < NumReplayStreams; i++)
	{
		sheader = (REPLAY_STREAM_HEADER*)pt;
		pt += sizeof(REPLAY_STREAM_HEADER);

		REPLAY_STREAM* destStream = &ReplayStreams[i];

		// copy source type
		memcpy((u_char*)&destStream->SourceType, (u_char*)&sheader->SourceType, sizeof(STREAM_SOURCE));

		int size = (sheader->Size + sizeof(PADRECORD)) & -4;

		// init buffers
		AllocateReplayStream(destStream, 4000);

		// copy pad data and advance buffer
		memcpy((u_char*)destStream->PadRecordBuffer, pt, size);

		pt += size;

		destStream->padCount = size / sizeof(PADRECORD);
		destStream->length = sheader->Length;

		if (sheader->Length > maxLength)
			maxLength = sheader->Length;
	}

	// [A] REDRIVER2 chase replays skip cameras
	if (header->magic == REDRIVER2_CHASE_MAGIC)
	{
		ReplayParameterPtr = (REPLAY_PARAMETER_BLOCK*)replayptr;
		memset((u_char*)ReplayParameterPtr, 0, sizeof(REPLAY_PARAMETER_BLOCK));
		ReplayParameterPtr->RecordingEnd = maxLength;

		PlayerWayRecordPtr = (SXYPAIR*)(ReplayParameterPtr + 1);
		PlaybackCamera = (PLAYBACKCAMERA*)(PlayerWayRecordPtr + MAX_REPLAY_WAYPOINTS);
	}
	else
	{
		ReplayParameterPtr = (REPLAY_PARAMETER_BLOCK*)replayptr;
		memset((u_char*)ReplayParameterPtr, 0, sizeof(REPLAY_PARAMETER_BLOCK));
		ReplayParameterPtr->RecordingEnd = maxLength;

		PlayerWayRecordPtr = (SXYPAIR*)(ReplayParameterPtr + 1);
		memset(PlayerWayRecordPtr, 0, sizeof(SXYPAIR) * MAX_REPLAY_WAYPOINTS);

		PlaybackCamera = (PLAYBACKCAMERA*)(PlayerWayRecordPtr + MAX_REPLAY_WAYPOINTS);
		memcpy((u_char*)PlaybackCamera, (u_char*)pt, sizeof(PLAYBACKCAMERA) * MAX_REPLAY_CAMERAS);
		pt += sizeof(PLAYBACKCAMERA) * MAX_REPLAY_CAMERAS;
	}

	PingBufferPos = 0;
	PingBuffer = (PING_PACKET*)(PlaybackCamera + MAX_REPLAY_CAMERAS);
	memcpy((u_char*)PingBuffer, (u_char*)pt, sizeof(PING_PACKET) * MAX_REPLAY_PINGS);
	memcpy((u_char*)NewPingBuffer, (u_char*)pt, sizeof(PING_PACKET) * MAX_REPLAY_PINGS);
	pt += sizeof(PING_PACKET) * MAX_REPLAY_PINGS;

	replayptr = (char*)(PingBuffer + MAX_REPLAY_PINGS);

	if (header->HaveStoredData == 0x91827364)	// -0x6e7d8c9c
	{
		memcpy((u_char*)&MissionStartData, (u_char*)pt, sizeof(MISSION_DATA));
		gHaveStoredData = 1;
	}

	return 1;
}
#endif
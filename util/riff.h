//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: RIFF reader utility class
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_RIFF_H
#define SND_RIFF_H

#include "core/IVirtualStream.h"
#include "core/dktypes.h"

#define RIFF_ID				MCHAR4('R','I','F','F')
#define WAVE_ID				MCHAR4('W','A','V','E')
#define CHUNK_FMT			MCHAR4('f','m','t',' ')
#define CHUNK_CUE			MCHAR4('c','u','e',' ')
#define CHUNK_DATA			MCHAR4('d','a','t','a')
#define CHUNK_SAMPLE		MCHAR4('s','m','p','l')

// RIFF WAVE FILE HEADERS
typedef struct
{
	int		Id;
	int		Size;
	int		Type;
} RIFFhdr_t;

typedef struct
{
	int		Id;
	int		Size;
} RIFFchunk_t;

typedef struct // CHUNK_FMT
{
	ushort Format;
	ushort Channels;
	uint   SamplesPerSec;
	uint   BytesPerSec;
	ushort BlockAlign;
	ushort BitsPerSample;
} wavfmthdr_t;

typedef struct // CHUNK_SAMPLE
{
	uint   Manufacturer;
	uint   Product;
	uint   SamplePeriod;
	uint   Note;
	uint   FineTune;
	uint   SMPTEFormat;
	uint   SMPTEOffest;
	uint   Loops;
	uint   SamplerData;

	struct
	{
		uint Identifier;
		uint Type;
		uint Start;
		uint End;
		uint Fraction;
		uint Count;
	}Loop[1];
}wavsamplehdr_t;

typedef struct // CHUNK_CUE
{
	uint Name;
	uint Position;
	uint fccChunk;
	uint ChunkStart;
	uint BlockStart;
	uint SampleOffset;
} wavcuehdr_t;

//-----------------------------------------------------------

class CRIFF_Parser
{
public:
	CRIFF_Parser(const char *szFilename);
	CRIFF_Parser(ubyte* pChunkData, int nChunkSize);

	void			ChunkClose();
	bool			ChunkNext();

	uint			GetName();
	int             GetSize();

	int             GetPos();
	int             SetPos(int pos);

	int				ReadChunk(void* dest, int maxLen = -1);
	int				ReadData(void* dest, int len);
	int				ReadInt();

private:
	bool			ChunkSet();
	int				m_pos;

	RIFFchunk_t		m_curChunk;

	IVirtualStream*	m_riff;
	ubyte*			m_riffData;
};

#endif // SND_RIFF_H
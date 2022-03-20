//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Streamed WAVe source
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_WAV_STREAM_H
#define SND_WAV_STREAM_H

#include "snd_wav_source.h"

class CSoundSource_WaveStream : public CSoundSource_Wave
{
public:
	int				GetSamples (ubyte *pOutput, int nSamples, int nOffset, bool bLooping);
	ubyte*			GetDataPtr(int& dataSize) const { dataSize = 0; return nullptr; }

	bool			Load(const char *szFilename);
	void			Unload();

	bool			IsStreaming() const { return true; }

private:
	virtual void    ParseData(CRIFF_Parser &chunk);

	int             ReadData (ubyte *pOutput, int nStart, int nBytes);

	int				m_dataOffset;   // data chunk
	int				m_dataSize;     // in bytes

	CRIFF_Parser*	m_reader;
};

#endif // SND_WAV_STREAM_H
//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Ogg Vorbis source stream
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_OGG_STREAM_H
#define SND_OGG_STREAM_H

#include "snd_ogg_cache.h"

class IVirtualStream;

class CSoundSource_OggStream : public CSoundSource_OggCache
{
public:
	virtual int     GetSamples(ubyte *pOutput, int nSamples, int nOffset, bool bLooping);
	ubyte*			GetDataPtr(int& dataSize) const { dataSize = 0; return nullptr; }

	virtual bool	Load(const char* filename);
	virtual void	Unload();

	bool			IsStreaming() const { return true; }

protected:
	void			ParseData(OggVorbis_File* file);

	int				ReadData(ubyte *pOutput, int nStart, int nBytes);

	IVirtualStream*	m_oggFile;
	OggVorbis_File	m_oggStream;

	int				m_dataSize;     // in bytes
};

#endif // SND_OGG_STREAM_H
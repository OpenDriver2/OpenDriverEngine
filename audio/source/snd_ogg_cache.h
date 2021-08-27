//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Ogg Vorbis source cache
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_OGG_CACHE_H
#define SND_OGG_CACHE_H

#include "snd_ogg_source.h"

class CSoundSource_OggCache : public CSoundSource_Ogg
{
	friend class CSoundSource_OpenALCache;
public:
	int				GetSamples(ubyte *pOutput, int nSamples, int nOffset, bool bLooping);
	ubyte*			GetDataPtr(int& dataSize) const { dataSize = m_cacheSize; return m_dataCache; }

	bool			Load(const char* filename);
	void			Unload();

	bool			IsStreaming() const { return false; }

protected:
	virtual void	ParseData(OggVorbis_File* file);

	ubyte*			m_dataCache;   // data chunk
	int				m_cacheSize;    // in bytes
};


#endif // SND_OGG_CACHE_H
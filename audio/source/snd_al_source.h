//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: OpenAL-compatible sound source (alBuffer)
//				No streaming support for this type, it just holds alBuffer
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_AL_SOURCE_H
#define SND_AL_SOURCE_H

#include "snd_source.h"

#include <AL/al.h>

class CSoundSource_WaveCache;
class CSoundSource_OggCache;

class CSoundSource_OpenALCache : public ISoundSource
{
	friend class CAudioSystemAL;
	friend class CAudioSourceAL;
public:
	CSoundSource_OpenALCache(ISoundSource* source);

	virtual int             GetSamples(ubyte* pOutput, int nSamples, int nOffset, bool bLooping);
	virtual ubyte*			GetDataPtr(int& dataSize) const;

	virtual Format*	GetFormat() const;
	virtual const char*		GetFilename() const;
	virtual int				GetSampleCount() const;

	virtual float           GetLoopPosition(float flPosition) const;

	virtual bool			IsStreaming() const;

private:
	void					InitWav(CSoundSource_WaveCache* wav);
	void					InitOgg(CSoundSource_OggCache* ogg);

	virtual bool			Load(const char* szFilename);
	virtual void			Unload();

	ALuint					m_alBuffer;
	Format					m_format;
	String					m_filename;
};

#endif // SND_AL_SOURCE_H
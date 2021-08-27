//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: OpenAL-compatible sound source (alBuffer)
//				No streaming support for this type, it just holds alBuffer
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/String.hpp>
#include <nstd/File.hpp>
#include "snd_al_source.h"
#include <AL/alext.h>

#include "snd_ogg_cache.h"
#include "snd_wav_cache.h"

CSoundSource_OpenALCache::CSoundSource_OpenALCache(ISoundSource* source)
{
	String filename = String::fromCString(source->GetFilename());
	String sndExt = File::extension(filename);

	if (!sndExt.compareIgnoreCase("wav"))
	{
		CSoundSource_WaveCache* wav = (CSoundSource_WaveCache*)source;
		InitWav(wav);
	}
	else if (!sndExt.compareIgnoreCase("ogg"))
	{
		CSoundSource_OggCache* ogg = (CSoundSource_OggCache*)source;
		InitOgg(ogg);
	}
}

void CSoundSource_OpenALCache::InitWav(CSoundSource_WaveCache* wav)
{
	alGenBuffers(1, &m_alBuffer);

	m_filename = wav->m_filename;

	m_format = *wav->GetFormat();
	ALenum alFormat;

	if (m_format.bitwidth == 8)
		alFormat = m_format.channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
	else if (m_format.bitwidth == 16)
		alFormat = m_format.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	else
		alFormat = AL_FORMAT_MONO16;

	alBufferData(m_alBuffer, alFormat, wav->m_dataCache, wav->m_cacheSize, m_format.frequency);

	int loopStart = wav->m_loopStart;
	int loopEnd = wav->m_loopEnd;

	// setup additional loop points
	if (loopStart > 0)
	{
		if (loopEnd == -1)
			alGetBufferi(m_alBuffer, AL_SAMPLE_LENGTH_SOFT, &loopEnd); // loop to the end

		int sampleOffs[] = { loopStart, loopEnd };
		alBufferiv(m_alBuffer, AL_LOOP_POINTS_SOFT, sampleOffs);
	}
}

void CSoundSource_OpenALCache::InitOgg(CSoundSource_OggCache* ogg)
{
	alGenBuffers(1, &m_alBuffer);

	m_filename = ogg->m_filename;

	m_format = *ogg->GetFormat();
	ALenum alFormat;

	if (m_format.bitwidth == 8)
		alFormat = m_format.channels == 2 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	else if (m_format.bitwidth == 16)
		alFormat = m_format.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	else
		alFormat = AL_FORMAT_MONO16;

	alBufferData(m_alBuffer, alFormat, ogg->m_dataCache, ogg->m_cacheSize, m_format.frequency);
}

void CSoundSource_OpenALCache::Unload()
{
	alDeleteBuffers(1, &m_alBuffer);
}

int CSoundSource_OpenALCache::GetSamples(ubyte* pOutput, int nSamples, int nOffset, bool bLooping) 
{
	return 0; 
}

ubyte* CSoundSource_OpenALCache::GetDataPtr(int& dataSize) const 
{
	return 0; 
}

ISoundSource::Format* CSoundSource_OpenALCache::GetFormat() const
{
	return (Format*)&m_format;
}

const char* CSoundSource_OpenALCache::GetFilename() const 
{
	return m_filename; 
}

int CSoundSource_OpenALCache::GetSampleCount() const 
{ 
	return 0;
};

float CSoundSource_OpenALCache::GetLoopPosition(float flPosition) const
{
	return 0;
};

bool CSoundSource_OpenALCache::IsStreaming() const
{
	return false;
}

bool CSoundSource_OpenALCache::Load(const char* szFilename) 
{
	return false; 
}
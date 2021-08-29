//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Ogg Vorbis source cache
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/String.hpp>
#include "snd_ogg_cache.h"
#include <string.h>
#include "core/cmdlib.h"
#include "core/FileStream.h"

bool CSoundSource_OggCache::Load(const char* filename)
{
	// Open for binary reading
	FILE* fp = fopen(filename, "rb");

	if (!fp)
		return false;

	CFileStream file(fp, true);

	OggVorbis_File oggFile;

	ov_callbacks cb;

	cb.read_func = eqVorbisFile::fread;
	cb.close_func = eqVorbisFile::fclose;
	cb.seek_func = eqVorbisFile::fseek;
	cb.tell_func = eqVorbisFile::ftell;

	int ovResult = ov_open_callbacks(&file, &oggFile, NULL, 0, cb);

	if(ovResult < 0)
	{
		MsgError("Failed to load sound '%s', because it is not a valid Ogg file (%d)\n", filename, ovResult);
		return false;
	}

	vorbis_info* info = ov_info(&oggFile, -1);
	ParseFormat(*info);

	ParseData(&oggFile);

	ov_clear( &oggFile );

	return m_numSamples > 0;
}

void CSoundSource_OggCache::Unload()
{
	Memory::free(m_dataCache);
	m_dataCache = nullptr;
	m_cacheSize = 0;
	m_numSamples = 0;
}

void CSoundSource_OggCache::ParseData(OggVorbis_File* file)
{
	m_numSamples = (uint)ov_pcm_total(file, -1);

	m_cacheSize = m_numSamples * m_format.channels * sizeof(short); // Ogg Vorbis is always 16 bit
	m_dataCache = (ubyte*)Memory::alloc(m_cacheSize);

	int samplePos = 0;
	while (samplePos < m_cacheSize)
	{
		char* dest = ((char *)m_dataCache) + samplePos;
		int readBytes = ov_read(file, dest, m_cacheSize-samplePos, 0, 2, 1, NULL);

		if (readBytes <= 0)
			break;

		samplePos += readBytes;
	}
}

int CSoundSource_OggCache::GetSamples(ubyte *pOutput, int nSamples, int nOffset, bool bLooping)
{
	int     nRemaining, nCompleted = 0;
	int     nBytes, nStart;

	int     nSampleSize = m_format.channels * m_format.bitwidth / 8;

	nBytes = nSamples * nSampleSize;
	nStart = nOffset * nSampleSize;

	nRemaining = nBytes;

	if ( nStart + nBytes > m_cacheSize )
		nBytes = m_cacheSize - nStart;

	memcpy( (void *)pOutput, (void *)(m_dataCache+nStart), nBytes );

	nRemaining -= nBytes;
	nCompleted += nBytes;

	while ( nRemaining && bLooping )
	{
		nBytes = nRemaining;

		if ( nBytes > m_cacheSize )
			nBytes = m_cacheSize;

		memcpy( (void *)(pOutput+nCompleted), (void *)m_dataCache, nBytes );
		nRemaining -= nBytes;
		nCompleted += nBytes;
	}

	return nCompleted / nSampleSize;
}
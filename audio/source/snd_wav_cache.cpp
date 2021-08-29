//////////////////////////////////////////////////////////////////////////////////
// Copyright � Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Cached WAVe data
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/String.hpp>
#include "snd_wav_cache.h"
#include <string.h>
#include "core/cmdlib.h"

bool CSoundSource_WaveCache::Load(const char* szFilename)
{
	m_filename = String::fromCString(szFilename);
	CRIFF_Parser reader( szFilename );

	while ( reader.GetName( ) )
	{
		ParseChunk( reader );
		reader.ChunkNext();
	}

	reader.ChunkClose();

	return m_numSamples > 0;
}

void CSoundSource_WaveCache::Unload()
{
	Memory::free(m_dataCache);
	m_dataCache = nullptr;
	m_cacheSize = 0;
	m_numSamples = 0;
}

void CSoundSource_WaveCache::ParseData(CRIFF_Parser &chunk)
{
	int sample;

	m_dataCache = (ubyte *)Memory::alloc( chunk.GetSize( ) );
	m_cacheSize = chunk.GetSize( );

	m_numSamples = m_cacheSize / (m_format.channels * m_format.bitwidth / 8);

	//
	//  read
	//
	chunk.ReadChunk( m_dataCache );

	/*
	//
	//  convert
	//
	for ( int i = 0; i < m_numSamples; i++ )
	{
		if ( m_format.bitwidth == 16 )
		{
			sample = ((short *)m_dataCache)[i];
			((short *)m_dataCache)[i] = sample;
		}
		else
		{
			sample = (int)((unsigned char)(m_dataCache[i]) - 128);
			((signed char *)m_dataCache)[i] = sample;
		}
	}
	*/
}

int CSoundSource_WaveCache::GetSamples(ubyte *pOutput, int nSamples, int nOffset, bool bLooping)
{
	int     nRemaining, nCompleted = 0;
	int     nBytes, nStart;

	int     nSampleSize = m_format.channels * m_format.bitwidth / 8;

	nBytes = nSamples * nSampleSize;
	nStart = nOffset * nSampleSize;

	nRemaining = nBytes;

	int size = /*bLooping && */m_loopEnd ? m_loopEnd : m_cacheSize;

	if ( nStart + nBytes > size)
		nBytes = size - nStart;

	memcpy( (void *)pOutput, (void *)(m_dataCache+nStart), nBytes );

	nRemaining -= nBytes;
	nCompleted += nBytes;

	// if we still have remaining data to fill stream for loop, but stream is at EOF, read it again
	while ( nRemaining && bLooping )
	{
		nBytes = nRemaining;

		if ( m_loopStart > 0 )
		{
			int loopBytes = m_loopStart * nSampleSize;

			if ( loopBytes + nBytes > size)
				nBytes = size - loopBytes;

			memcpy( (void *)(pOutput+nCompleted), (void *)(m_dataCache+loopBytes), nBytes );
			nRemaining -= nBytes;
			nCompleted += nBytes;
		}
		else
		{
			if ( nBytes > size)
				nBytes = size;

			memcpy( (void *)(pOutput+nCompleted), (void *)m_dataCache, nBytes );
			nRemaining -= nBytes;
			nCompleted += nBytes;
		}
	}

	return nCompleted / nSampleSize;
}

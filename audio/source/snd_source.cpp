//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: WAVe source
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/File.hpp>
#include <nstd/String.hpp>
#include "snd_source.h"

#include "core/cmdlib.h"

#include "snd_wav_cache.h"
#include "snd_wav_stream.h"

#include "snd_ogg_cache.h"
#include "snd_ogg_stream.h"
#include "snd_xm_stream.h"

#define STREAM_THRESHOLD    (1024*1024)     // 1mb

//-----------------------------------------------------------------

ISoundSource* ISoundSource::CreateSound( const char* szFilename )
{
	String fileExt = File::extension(String::fromCString(szFilename));
	fileExt.replace('#', '\0');

	ISoundSource* pSource = nullptr;

	int fileLen = 0;

	File test;
	if (test.open(String::fromCString(szFilename), File::readFlag))
	{
		fileLen = test.size();
	}

	if ( !fileExt.compareIgnoreCase("wav"))
	{
		if (fileLen > STREAM_THRESHOLD )
			pSource = (ISoundSource*)new CSoundSource_WaveStream;
		else
			pSource = (ISoundSource*)new CSoundSource_WaveCache;
	}
	else if ( !fileExt.compareIgnoreCase("ogg"))
	{
		if (fileLen > STREAM_THRESHOLD )
			pSource = (ISoundSource*)new CSoundSource_OggStream;
		else
			pSource = (ISoundSource*)new CSoundSource_OggCache;
	}
	else if (!fileExt.compareIgnoreCase("xm"))
	{
		pSource = (ISoundSource*)new CSoundSource_ExtendedModule;
	}
	else
		MsgError( "Unknown sound format: %s\n", szFilename );

	if ( pSource )
	{
		if(!pSource->Load( szFilename ))
		{
			MsgError( "Cannot load sound '%s'\n", szFilename );
			delete pSource;
			pSource = nullptr;
		}

		return pSource;
	}
	else if ( pSource )
		delete pSource;

	return nullptr;
}

void ISoundSource::DestroySound(ISoundSource *pSound)
{
    if ( pSound )
    {
        pSound->Unload( );
        delete pSound;
    }
}
//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Ogg Vorbis source base class
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/String.hpp>
#include "snd_ogg_source.h"
#include "core/IVirtualStream.h"

namespace eqVorbisFile
{
	size_t fread(void *ptr, size_t size, size_t nmemb, void *datasource)
	{
		IVirtualStream* pFile = (IVirtualStream*)datasource;

		return pFile->Read(ptr, nmemb, size);
	}

	int	fseek(void *datasource, ogg_int64_t offset, int whence)
	{
		IVirtualStream* pFile = (IVirtualStream*)datasource;

		// let's do some undocumented features of ogg

		int returnVal;

		switch(whence)
		{
			case SEEK_SET:
			case SEEK_CUR:
			case SEEK_END:
				returnVal = pFile->Seek(offset, (VirtStreamSeek_e)whence);
				break;
			default: //Bad value
				return -1;
		}

		if(returnVal == 0)
			return 0;
		else
			return -1; //Could not do a seek. Device not capable of seeking. (Should never encounter this case)
	}

	long ftell(void *datasource)
	{
		IVirtualStream* pFile = (IVirtualStream*)datasource;

		return pFile->Tell();
	}

	int fclose(void *datasource)
	{
		return 1;
	}
};

void CSoundSource_Ogg::ParseFormat(vorbis_info& info)
{
	m_format.channels = info.channels;
	m_format.frequency = info.rate;
	m_format.dataFormat = 1;	// PCM
	m_format.bitwidth = 16; // Ogg Vorbis is always 16 bit
}

//---------------------------------------------------------

float CSoundSource_Ogg::GetLoopPosition(float flPosition) const
{
    while ( flPosition > m_numSamples )
        flPosition -= m_numSamples;

    return flPosition;
}
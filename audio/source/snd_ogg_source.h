//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Ogg Vorbis source base class
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_OGG_SOURCE_H
#define SND_OGG_SOURCE_H

#include "snd_source.h"
#include <vorbis/vorbisfile.h>

namespace eqVorbisFile
{
	size_t	fread(void *ptr, size_t size, size_t nmemb, void *datasource);
	int		fseek(void *datasource, ogg_int64_t offset, int whence);
	long	ftell(void *datasource);
	int		fclose(void *datasource);
};

//---------------------------------------------------------------------

class CSoundSource_Ogg : public ISoundSource
{
public:
	virtual Format*	GetFormat() const							{ return (Format*)&m_format; }
	virtual const char*		GetFilename() const					{ return m_filename; }
	virtual float			GetLoopPosition(float flPosition) const;
	virtual int				GetSampleCount() const				{ return m_numSamples; }

protected:
	void					ParseFormat(vorbis_info& info);
	virtual void			ParseData(OggVorbis_File* file) = 0;

	Format					m_format;
	String					m_filename;

	int						m_numSamples;
};

#endif // SND_OGG_SOURCE_H

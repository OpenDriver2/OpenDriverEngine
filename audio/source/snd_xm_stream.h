//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: WAVe source base class
//////////////////////////////////////////////////////////////////////////////////

#ifndef SND_XM_STREAM_H
#define SND_XM_STREAM_H

#include "snd_source.h"
#include "util/riff.h"

extern "C" {
	typedef struct xm_context_s xm_context_t;
}

//---------------------------------------------------------------------

class CSoundSource_ExtendedModule : public ISoundSource
{
public:
	CSoundSource_ExtendedModule();

	Format*			GetFormat() const { return (Format*)&m_format; }
	const char*		GetFilename() const { return m_filename; }
	float			GetLoopPosition(float flPosition) const;
	int				GetSampleCount() const { return m_numSamples; }

	int				GetSamples (ubyte *pOutput, int nSamples, int nOffset, bool bLooping);
	ubyte*			GetDataPtr(int& dataSize) const { dataSize = 0; return nullptr; }

	bool			Load(const char *szFilename);
	void			Unload();

	bool			IsStreaming() const { return true; }
protected:


	Format					m_format;
	String					m_filename;

	int						m_numSamples{ 0 };
	int						m_startPat{ 0 };

	xm_context_t*			m_xmCtx{nullptr};
};

#endif // SND_XM_STREAM_H

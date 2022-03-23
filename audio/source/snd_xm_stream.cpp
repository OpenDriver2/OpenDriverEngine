//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: WAVe source base class
//////////////////////////////////////////////////////////////////////////////////

#include <nstd/Debug.hpp>
#include <nstd/Math.hpp>
#include <nstd/String.hpp>
#include <core/cmdlib.h>
#include <core/FileStream.h>
#include <math/Vector.h>
#include <string.h>
#include <stdio.h>
#include <xm.h>

#include "snd_xm_stream.h"

const int XMSampleRate = 44100;

//---------------------------------------------------------------------

CSoundSource_ExtendedModule::CSoundSource_ExtendedModule()
{
	m_format.channels = 2;
	m_format.frequency = XMSampleRate;
	m_format.dataFormat = 1;		// PCM
	m_format.bitwidth = 16;
}

float CSoundSource_ExtendedModule::GetLoopPosition(float flPosition) const
{
    while (flPosition > m_numSamples)
        flPosition -= m_numSamples;

    return flPosition;
}

int CSoundSource_ExtendedModule::GetSamples(ubyte* pOutput, int nSamples, int nOffset, bool bLooping)
{
	int nSampleSize = m_format.channels * m_format.bitwidth / 8;
	float* tmpSamples = (float*)Memory::alloc(nSamples * sizeof(float));

	uint64_t sampleCnt;
	xm_get_position(m_xmCtx, nullptr, nullptr, nullptr, &sampleCnt);
	if (nOffset == 0 && sampleCnt != 0)
	{
		xm_seek(m_xmCtx, m_startPat, 0, 0);
	}

	short* tmpOutput = (short*)pOutput;
	xm_generate_samples(m_xmCtx, tmpSamples, nSamples / m_format.channels);
	for (int i = 0; i < nSamples; ++i)
	{
		tmpOutput[i] = clamp(short(tmpSamples[i] * SHRT_MAX * 2.0f), SHRT_MIN, SHRT_MAX);
	}

	Memory::free(tmpSamples);
	return nSamples / m_format.channels;
}

bool CSoundSource_ExtendedModule::Load(const char* szFilename)
{
	String fileName = String::fromCString(szFilename);
	fileName.replace('#', '\0');

	FILE* fp = fopen(fileName, "rb");

	if (!fp)
	{
		MsgError("LoadXM: cannot open '%s'\n", (const char*)fileName);
		return false;
	}

	const char* subStr = strchr(szFilename, '#');
	if (subStr)
	{
		m_startPat = atoi(subStr+1);
	}

	CFileStream fileStream(fp, true);
	long xmSize = fileStream.GetSize();

	char* xmData = (char*)Memory::alloc(fileStream.GetSize());
	fileStream.Read(xmData, xmSize, 1);

	switch (xm_create_context_safe(&m_xmCtx, xmData, xmSize, XMSampleRate))
	{
		case 0:
			break;
		case 1:
			MsgError("could not create XM context: module is not sane\n");
			break;

		case 2:
			MsgError("could not create XM context: malloc failed\n");
			break;

		default:
			MsgError("could not create XM context: unknown error\n");
			break;
	}

	Memory::free(xmData);

	m_numSamples = INT_MAX;

	if (m_xmCtx)
	{
		xm_seek(m_xmCtx, m_startPat, 0, 0);
	}

	return m_xmCtx != nullptr;
}

void CSoundSource_ExtendedModule::Unload()
{
	xm_free_context(m_xmCtx);
}

#include "VirtualStream.h"
#include <string.h> // va_*
#include <stdarg.h> // va_*

#include <nstd/String.hpp>

#define VSTREAM_GRANULARITY 1024	// 1kb

// prints string to stream
void IVirtualStream::Print(const char* pFmt, ...)
{
	va_list		argptr;

	static char	string[512];
	char* buf = string;

	va_start (argptr,pFmt);
	int wcount = vsnprintf(string, sizeof(string), pFmt, argptr);

	// print again into extended buffer
	if (wcount >= sizeof(string))
	{
		buf = new char[wcount+1];
		wcount = vsnprintf(buf, wcount + 1, pFmt, argptr);
	}

	va_end (argptr);

	Write(buf, 1, wcount);

	if (buf != string)
	{
		delete[] buf;
	}
}

//--------------------------
// CMemoryStream - File stream
//--------------------------

CMemoryStream::CMemoryStream()
{
	m_pCurrent = nullptr;
	m_pStart = nullptr;

	m_nAllocatedSize = 0;
	m_nUsageFlags = 0;
}

// destroys stream data
CMemoryStream::~CMemoryStream()
{
	// destroy memory
	m_pCurrent = nullptr;

	Memory::free( m_pStart );
	m_pStart = nullptr;
	m_nAllocatedSize = 0;
	m_nUsageFlags = 0;
}

// reads data from virtual stream
size_t CMemoryStream::Read(void *dest, size_t count, size_t size)
{
	if(!(m_nUsageFlags & VS_OPEN_READ) || m_nAllocatedSize == 0)
		return 0;

	long nReadBytes = size*count;

	long nCurPos = Tell();

	if(nCurPos+nReadBytes > m_nAllocatedSize)
		nReadBytes -= ((nCurPos+nReadBytes) - m_nAllocatedSize);

	// copy memory
	memcpy(dest, m_pCurrent, nReadBytes);

	m_pCurrent += nReadBytes;

	return nReadBytes;
}

// writes data to virtual stream
size_t CMemoryStream::Write(const void *src, size_t count, size_t size)
{
	if(!(m_nUsageFlags & VS_OPEN_WRITE))
		return 0;

	long nAddBytes = size*count;

	long nCurrPos = Tell();

	if(nCurrPos+nAddBytes > m_nAllocatedSize)
	{
		long mem_diff = (nCurrPos+nAddBytes) - m_nAllocatedSize;

		long newSize = m_nAllocatedSize + mem_diff + VSTREAM_GRANULARITY - 1;
		newSize -= newSize % VSTREAM_GRANULARITY;

		ReAllocate( newSize );
	}

	// copy memory
	memcpy(m_pCurrent, src, nAddBytes);

	m_pCurrent += nAddBytes;

	return count;
}

// seeks pointer to position
int CMemoryStream::Seek(long nOffset, VirtStreamSeek_e seekType)
{
	switch(seekType)
	{
		case VS_SEEK_SET:
			m_pCurrent = m_pStart+nOffset;
			break;
		case VS_SEEK_CUR:
			m_pCurrent = m_pCurrent+nOffset;
			break;
		case VS_SEEK_END:
			m_pCurrent = m_pStart + m_nAllocatedSize + nOffset;
			break;
	}

	return Tell();
}

// returns current pointer position
long CMemoryStream::Tell()
{
	return m_pCurrent - m_pStart;
}

// returns memory allocated for this stream
long CMemoryStream::GetSize()
{
	return m_nAllocatedSize;
}

// opens stream, if this is a file, data is filename
bool CMemoryStream::Open(ubyte* data, int nOpenFlags, int nDataSize)
{
	Close();

	m_nUsageFlags = nOpenFlags;

	if((m_nUsageFlags & VS_OPEN_WRITE) && !(m_nUsageFlags & VS_OPEN_READ))
	{
		// data will be written reset it
		if(m_pStart)
			Memory::free(m_pStart);

		m_nAllocatedSize = 0;

		m_pStart = nullptr;
		m_pCurrent = nullptr;
	}

	// make this as base buffer
	if(nDataSize > 0 && m_pStart == nullptr)
		ReAllocate(nDataSize);

	if((m_nUsageFlags & VS_OPEN_READ) && data)
	{
		memcpy(m_pStart, data, nDataSize);
	}

	return true;
}

// closes stream
void CMemoryStream::Close()
{
	m_nUsageFlags = 0;
	m_pCurrent = m_pStart;
}

// flushes stream, doesn't affects on memory stream
int CMemoryStream::Flush()
{
	// do nothing
	return 0;
}

// reallocates memory
void CMemoryStream::ReAllocate(long nNewSize)
{
	if(nNewSize == m_nAllocatedSize)
		return;

	long curPos = Tell();

	ubyte* pTemp = (ubyte*)Memory::alloc( nNewSize );

	if(m_pStart && m_nAllocatedSize > 0)
	{
		memcpy( pTemp, m_pStart, m_nAllocatedSize );

		Memory::free(m_pStart);
	}

	m_nAllocatedSize = nNewSize;

	m_pStart = pTemp;
	m_pCurrent = m_pStart+curPos;

	//Msg("CMemoryStream::ReAllocate(): New size: %d\n", nNewSize);
}

// saves stream to file for stream (only for memory stream )
void CMemoryStream::WriteToFileStream(FILE* pFile)
{
	int stream_size = m_pCurrent-m_pStart;

	fwrite(m_pStart, 1, stream_size, pFile);
}

// reads file to this stream
bool CMemoryStream::ReadFromFileStream( FILE* pFile )
{
	if(!(m_nUsageFlags & VS_OPEN_WRITE))
		return false;

	int rest_pos = ftell(pFile);

	// seek to end
	fseek(pFile,0,SEEK_END);
	int filesize = ftell(pFile);

	fseek(pFile, 0, SEEK_SET);

	// make enough space
	ReAllocate( filesize + 32 );

	// read to me
	fread(m_pStart, 1, filesize, pFile);

	// restore
	fseek(pFile, rest_pos, VS_SEEK_SET);

	// let user seek this stream after
	return true;
}

// returns current pointer to the stream (only memory stream)
ubyte* CMemoryStream::GetCurrentPointer()
{
	return m_pCurrent;
}

// returns base pointer to the stream (only memory stream)
ubyte* CMemoryStream::GetBasePointer()
{
	return m_pStart;
}

//--------------------------------------------------------------------


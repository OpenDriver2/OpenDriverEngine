#ifndef VIRTUALSTREAM_H
#define VIRTUALSTREAM_H

#include "IVirtualStream.h"
#include <stdio.h>

//--------------------------
// CMemoryStream - File stream
//--------------------------

class CMemoryStream : public IVirtualStream
{
public:
						CMemoryStream();
						~CMemoryStream();

	// reads data from virtual stream
	size_t				Read(void *dest, size_t count, size_t size);

	// writes data to virtual stream
	size_t				Write(const void *src, size_t count, size_t size);

	// seeks pointer to position
	int					Seek(long nOffset, VirtStreamSeek_e seekType);

	// returns current pointer position
	long				Tell();

	// returns memory allocated for this stream
	long				GetSize();

	// opens stream, if this is a file, data is filename
	bool				Open(ubyte* data, int nOpenFlags, int nDataSize);

	// closes stream
	void				Close();

	// flushes stream, doesn't affects on memory stream
	int					Flush();

	VirtStreamType_e	GetType() { return m_pStart ? VS_TYPE_MEMORY : VS_TYPE_INVALID; }

	// reads file to this stream
	bool				ReadFromFileStream( FILE* pFile );

	// saves stream to file for stream (only for memory stream )
	void				WriteToFileStream( FILE* pFile );

	// returns current pointer to the stream (only memory stream)
	ubyte*				GetCurrentPointer();

	// returns base pointer to the stream (only memory stream)
	ubyte*				GetBasePointer();

protected:

	// reallocates memory
	void				ReAllocate(long nNewSize);

private:

	ubyte*				m_pStart;
	ubyte*				m_pCurrent;

	long				m_nAllocatedSize;
	long				m_nUsageFlags;
};

class CFileStream : public IVirtualStream
{
public:
						CFileStream(FILE* pFile) : m_pFilePtr(pFile)
						{
						}

    int					Seek( long pos, VirtStreamSeek_e seekType );
    long				Tell();
    size_t				Read( void *dest, size_t count, size_t size);
    size_t				Write( const void *src, size_t count, size_t size);
    int					Error();
    int					Flush();
	void				Print(const char* pFmt, ...);

	long				GetSize();

	VirtStreamType_e	GetType() {return m_pFilePtr ? VS_TYPE_FILE : VS_TYPE_INVALID;}

protected:
	FILE*				m_pFilePtr;
};

#endif // VIRTUALSTREAM_H

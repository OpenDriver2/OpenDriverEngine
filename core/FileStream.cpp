#include "FileStream.h"
#include <stdarg.h>

//------------------------------------------------------------------------------
// File stream
//------------------------------------------------------------------------------

int	CFileStream::Seek(long pos, VirtStreamSeek_e seekType)
{
	return fseek(m_pFilePtr, pos, seekType);
}

long CFileStream::Tell()
{
	return ftell(m_pFilePtr);
}

size_t CFileStream::Read(void* dest, size_t count, size_t size)
{
	return fread(dest, size, count, m_pFilePtr);
}

size_t CFileStream::Write(const void* src, size_t count, size_t size)
{
	return fwrite(src, size, count, m_pFilePtr);
}

int	CFileStream::Error()
{
	return ferror(m_pFilePtr);
}

int	CFileStream::Flush()
{
	return fflush(m_pFilePtr);
}

long CFileStream::GetSize()
{
	long pos = Tell();

	Seek(0, VS_SEEK_END);

	long length = Tell();

	Seek(pos, VS_SEEK_SET);

	return length;
}
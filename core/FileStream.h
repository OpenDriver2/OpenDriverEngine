#ifndef FILESTREAM_H
#define FILESTREAM_H

#include "IVirtualStream.h"
#include <stdio.h>

class CFileStream : public IVirtualStream
{
public:
	CFileStream(FILE* pFile, bool autoClose)
		: m_pFilePtr(pFile), m_autoClose(autoClose)
	{
	}

	~CFileStream()
	{
		if(m_autoClose)
			fclose(m_pFilePtr);
	}

	int					Seek(long pos, VirtStreamSeek_e seekType);
	long				Tell();
	size_t				Read(void* dest, size_t count, size_t size);
	size_t				Write(const void* src, size_t count, size_t size);
	int					Error();
	int					Flush();

	long				GetSize();

	VirtStreamType_e	GetType() { return m_pFilePtr ? VS_TYPE_FILE : VS_TYPE_INVALID; }

protected:
	FILE* m_pFilePtr;
	bool m_autoClose;
};

#endif // FILESTREAM_H
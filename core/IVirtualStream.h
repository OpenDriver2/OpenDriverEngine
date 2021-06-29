#ifndef IVRITUALSTREAM_H
#define IVRITUALSTREAM_H

#include "core/dktypes.h"
#include <stddef.h>

enum VirtStreamType_e
{
	VS_TYPE_INVALID = -1,

	VS_TYPE_MEMORY = 0,
	VS_TYPE_FILE,
	VS_TYPE_FILE_PACKAGE,
};

// fancy check
#define IsFileType(type) (type >= VS_TYPE_FILE)

enum VirtStreamSeek_e
{
	VS_SEEK_SET = 0,	// set current position
	VS_SEEK_CUR,		// seek from last position
	VS_SEEK_END,		// seek to the end
};

//
enum VirtStreamOpenFlags_e
{
	VS_OPEN_READ	= (1 << 0),
	VS_OPEN_WRITE	= (1 << 1),

	// file stream only
	VS_OPEN_TEXT	= (1 << 2),
	VS_OPEN_REWRITE = (1 << 3), 
};

//--------------------------
// IVirtualStream - Virtual stream interface
//--------------------------

class IVirtualStream
{
public:
	virtual ~IVirtualStream() {}

	// reads data from virtual stream
	virtual size_t				Read( void *dest, size_t count, size_t size) = 0;

	// writes data to virtual stream
	virtual size_t				Write(const void *src, size_t count, size_t size) = 0;

	// seeks pointer to position
	virtual int					Seek( long nOffset, VirtStreamSeek_e seekType) = 0;

	// fprintf analog
	virtual	void				Print(const char* fmt, ...);

	// returns current pointer position
	virtual long				Tell() = 0;

	// returns memory allocated for this stream
	virtual long				GetSize() = 0;

	// flushes stream from memory
	virtual int					Flush() = 0;

	// returns stream type
	virtual VirtStreamType_e	GetType() = 0;
};

#endif // IVRITUALSTREAM_H
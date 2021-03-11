#ifndef PSX_MATH_TYPES_H
#define PSX_MATH_TYPES_H

struct VECTOR_NOPAD {
	int vx; // size=0, offset=0
	int vy; // size=0, offset=4
	int vz; // size=0, offset=8
};

struct USVECTOR_NOPAD {
	ushort vx; // size=0, offset=0
	ushort vy; // size=0, offset=2
	ushort vz; // size=0, offset=4
};

struct SVECTOR
{
	short x;
	short y;
	short z;
	short pad;
};

struct CVECTOR
{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte pad;
};

struct CVECTOR_NOPAD
{
	ubyte r;
	ubyte g;
	ubyte b;
};

struct XYPAIR
{
	int x;
	int y;
};

struct XZPAIR
{
	int x;
	int z;
};

struct UV_INFO
{
	uint8 u;
	uint8 v;
};

#endif // PSX_MATH_TYPES_H
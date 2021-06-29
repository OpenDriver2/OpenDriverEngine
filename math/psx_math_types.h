#ifndef PSX_MATH_TYPES_H
#define PSX_MATH_TYPES_H

#define ONE			4096
#define ONE_F		float(ONE)

struct VECTOR_NOPAD
{
	int vx;
	int vy;
	int vz;
};

struct USVECTOR_NOPAD
{
	ushort vx;
	ushort vy;
	ushort vz;
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

struct UV
{
	uint8 u0, v0;
	uint8 u1, v1;
	uint8 u2, v2;
	uint8 u3, v3;
};

#endif // PSX_MATH_TYPES_H
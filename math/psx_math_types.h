#ifndef PSX_MATH_TYPES_H
#define PSX_MATH_TYPES_H

#define ONE			4096
#define ONE_F		float(ONE)
#define ONE_BITS	12

#define FixHalfRound(x, bits)		(((x) + (1 << (bits-1))) >> bits)
#define	FIXEDH(a)					FixHalfRound(a, ONE_BITS)		// Fixed Half Round number
#define	FIXED(a)					((a) >> ONE_BITS)				// Fixed number (unsigned)

struct VECTOR_NOPAD
{
	int vx, vy, vz;
};

struct SVECTOR_NOPAD
{
	short vx, vy, vz;
};

struct USVECTOR_NOPAD
{
	uint16 vx, vy, vz;
};

struct SVECTOR
{
	short vx, vy, vz, pad;
};

struct CVECTOR
{
	uint8 r, g, b, pad;
};

struct CVECTOR_NOPAD
{
	uint8 r, g, b;
};

typedef struct {
	short	m[3][3];	/* 3x3 rotation matrix */
	long    t[3];		/* transfer vector */
} MATRIX;

struct XYPAIR
{
	int x, y;
};

struct XZPAIR
{
	int x, z;
};

struct SXYPAIR
{
	short x, y;
};

struct SXZPAIR
{
	short x, z;
};

struct UV_INFO
{
	uint8 u, v;
};

struct UV
{
	uint8 u0, v0;
	uint8 u1, v1;
	uint8 u2, v2;
	uint8 u3, v3;
};

#endif // PSX_MATH_TYPES_H
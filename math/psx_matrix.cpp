#include "core/dktypes.h"
#include <nstd/Math.hpp>
#include "psx_math_types.h"
#include "isin.h"
#include "psx_matrix.h"
#include "Vector.h"

#define	FIX_BIT			12

#define	FIX_1			(1<<FIX_BIT)
#define	FIX(a)			((a)>>FIX_BIT)

// Performs matrix multiplication
// m2 = m0 x m1
MATRIX* MulMatrix0(MATRIX* m0, MATRIX* m1, MATRIX* m2)
{
	int vx, vy, vz;
	MATRIX tmp;

	if (m0 == m2)
		tmp = *m0; m0 = &tmp;

	vx = m1->m[0][0];
	vy = m1->m[1][0];
	vz = m1->m[2][0];
	m2->m[0][0] = FIX(m0->m[0][0] * vx + m0->m[0][1] * vy + m0->m[0][2] * vz);
	m2->m[1][0] = FIX(m0->m[1][0] * vx + m0->m[1][1] * vy + m0->m[1][2] * vz);
	m2->m[2][0] = FIX(m0->m[2][0] * vx + m0->m[2][1] * vy + m0->m[2][2] * vz);

	vx = m1->m[0][1];
	vy = m1->m[1][1];
	vz = m1->m[2][1];

	m2->m[0][1] = FIX(m0->m[0][0] * vx + m0->m[0][1] * vy + m0->m[0][2] * vz);
	m2->m[1][1] = FIX(m0->m[1][0] * vx + m0->m[1][1] * vy + m0->m[1][2] * vz);
	m2->m[2][1] = FIX(m0->m[2][0] * vx + m0->m[2][1] * vy + m0->m[2][2] * vz);

	vx = m1->m[0][2];
	vy = m1->m[1][2];
	vz = m1->m[2][2];

	m2->m[0][2] = FIX(m0->m[0][0] * vx + m0->m[0][1] * vy + m0->m[0][2] * vz);
	m2->m[1][2] = FIX(m0->m[1][0] * vx + m0->m[1][1] * vy + m0->m[1][2] * vz);
	m2->m[2][2] = FIX(m0->m[2][0] * vx + m0->m[2][1] * vy + m0->m[2][2] * vz);

	return m2;
}

#define	APPLYMATRIX(m,v0,v1)	{\
	int vx = v0->vx;\
	int vy = v0->vy;\
	int vz = v0->vz;\
	v1->vx = FIX(m->m[0][0]*vx + m->m[0][1]*vy + m->m[0][2]*vz );\
	v1->vy = FIX(m->m[1][0]*vx + m->m[1][1]*vy + m->m[1][2]*vz );\
	v1->vz = FIX(m->m[2][0]*vx + m->m[2][1]*vy + m->m[2][2]*vz );\
}


// Applies matrix rotation to vector
// v1 = m x v0
VECTOR_NOPAD* ApplyMatrix(MATRIX* m, SVECTOR* v0, VECTOR_NOPAD* v1)
{
	APPLYMATRIX(m, v0, v1)
	return v1;
}

// Applies matrix rotation to short vector
//	v1 = m x v0
SVECTOR* ApplyMatrixSV(MATRIX* m, SVECTOR* v0, SVECTOR* v1)
{
	APPLYMATRIX(m, v0, v1)
	return v1;
}

// Applies matrix to long vector
//	v1 = m x v0
VECTOR_NOPAD* ApplyMatrixLV(MATRIX* m, VECTOR_NOPAD* v0, VECTOR_NOPAD* v1)
{
	APPLYMATRIX(m, v0, v1)
	return v1;
}

// creates a rotation matrix
// m = [r->vx] x [r->vy] x [r->vz]
MATRIX* RotMatrix(SVECTOR* r, MATRIX* m)
{
	int c0, c1, c2;
	int s0, s1, s2;
	int s2p0, s2m0, c2p0, c2m0;
	int	s2c0, s2s0, c2c0, c2s0;

	c0 = icos(r->vx);
	c1 = icos(r->vy);
	c2 = icos(r->vz);
	s0 = isin(r->vx);
	s1 = isin(r->vy);
	s2 = isin(r->vz);
	s2p0 = isin(r->vz + r->vx);
	s2m0 = isin(r->vz - r->vx);
	c2p0 = icos(r->vz + r->vx);
	c2m0 = icos(r->vz - r->vx);
	s2c0 = (s2p0 + s2m0) / 2;
	c2s0 = (s2p0 - s2m0) / 2;
	s2s0 = (c2m0 - c2p0) / 2;
	c2c0 = (c2m0 + c2p0) / 2;

	m->m[0][0] = FIX(c2 * c1);
	m->m[1][0] = s2c0 + FIX(c2s0 * s1);
	m->m[2][0] = s2s0 - FIX(c2c0 * s1);
	m->m[0][1] = -FIX(s2 * c1);
	m->m[1][1] = c2c0 - FIX(s2s0 * s1);
	m->m[2][1] = c2s0 + FIX(s2c0 * s1);
	m->m[0][2] = s1;
	m->m[1][2] = -FIX(c1 * s0);
	m->m[2][2] = FIX(c1 * c0);

	return m;
}

// rotates matrix by angle
// m = m * [r] x
MATRIX* RotMatrixX(long r, MATRIX* m)
{
	int s0 = isin(r);
	int c0 = icos(r);
	int t1, t2;
	t1 = m->m[1][0];
	t2 = m->m[2][0];
	m->m[1][0] = FIX(t1 * c0 - t2 * s0);
	m->m[2][0] = FIX(t1 * s0 + t2 * c0);
	t1 = m->m[1][1];
	t2 = m->m[2][1];
	m->m[1][1] = FIX(t1 * c0 - t2 * s0);
	m->m[2][1] = FIX(t1 * s0 + t2 * c0);
	t1 = m->m[1][2];
	t2 = m->m[2][2];
	m->m[1][2] = FIX(t1 * c0 - t2 * s0);
	m->m[2][2] = FIX(t1 * s0 + t2 * c0);

	return m;
}

// rotates matrix by angle
// m = m * [r] y
MATRIX* RotMatrixY(long r, MATRIX* m)
{
	int s0 = isin(r);
	int c0 = icos(r);
	int t1, t2;
	t1 = m->m[0][0];
	t2 = m->m[2][0];
	m->m[0][0] = FIX(t1 * c0 + t2 * s0);
	m->m[2][0] = FIX(-t1 * s0 + t2 * c0);
	t1 = m->m[0][1];
	t2 = m->m[2][1];
	m->m[0][1] = FIX(t1 * c0 + t2 * s0);
	m->m[2][1] = FIX(-t1 * s0 + t2 * c0);
	t1 = m->m[0][2];
	t2 = m->m[2][2];
	m->m[0][2] = FIX(t1 * c0 + t2 * s0);
	m->m[2][2] = FIX(-t1 * s0 + t2 * c0);

	return m;
}

// rotates matrix by angle
// m = m * [r] z
MATRIX* RotMatrixZ(long r, MATRIX* m)
{
	int s0 = isin(r);
	int c0 = icos(r);
	int t1, t2;
	t1 = m->m[0][0];
	t2 = m->m[1][0];
	m->m[0][0] = FIX(t1 * c0 - t2 * s0);
	m->m[1][0] = FIX(t1 * s0 + t2 * c0);
	t1 = m->m[0][1];
	t2 = m->m[1][1];
	m->m[0][0] = FIX(t1 * c0 - t2 * s0);
	m->m[1][1] = FIX(t1 * s0 + t2 * c0);
	t1 = m->m[0][2];
	t2 = m->m[1][2];
	m->m[0][2] = FIX(t1 * c0 - t2 * s0);
	m->m[1][2] = FIX(t1 * s0 + t2 * c0);

	return m;
}

// sets matrix translation
MATRIX* TransMatrix(MATRIX* m, VECTOR_NOPAD* v)
{
	m->t[0] = v->vx;
	m->t[1] = v->vy;
	m->t[2] = v->vz;
	return m;
}

// scales the matrix by arbitary value
MATRIX* ScaleMatrix(MATRIX* m, VECTOR_NOPAD* v)
{
	m->m[0][0] = FIX(m->m[0][0] * v->vx);
	m->m[0][1] = FIX(m->m[0][1] * v->vx);
	m->m[0][2] = FIX(m->m[0][2] * v->vx);
	m->m[1][0] = FIX(m->m[1][0] * v->vy);
	m->m[1][1] = FIX(m->m[1][1] * v->vy);
	m->m[1][2] = FIX(m->m[1][2] * v->vy);
	m->m[2][0] = FIX(m->m[2][0] * v->vz);
	m->m[2][1] = FIX(m->m[2][1] * v->vz);
	m->m[2][2] = FIX(m->m[2][2] * v->vz);
	return m;
}

//-------------------------------------------
// PSX GTE routines emulation
//-------------------------------------------

MATRIX _globmat;
VECTOR_NOPAD _vr0, _vr1, _vr2, _vr3;
VECTOR_NOPAD _mac;
SVECTOR_NOPAD _ir;

void gte_SetRotMatrix(MATRIX* m)
{
	_globmat.m[0][0] = m->m[0][0];
	_globmat.m[0][1] = m->m[0][1];
	_globmat.m[0][2] = m->m[0][2];
	_globmat.m[1][0] = m->m[1][0];
	_globmat.m[1][1] = m->m[1][1];
	_globmat.m[1][2] = m->m[1][2];
	_globmat.m[2][0] = m->m[2][0];
	_globmat.m[2][1] = m->m[2][1];
	_globmat.m[2][2] = m->m[2][2];
}

void gte_SetTransMatrix(MATRIX* m)
{
	_globmat.t[0] = m->t[0];
	_globmat.t[1] = m->t[1];
	_globmat.t[2] = m->t[2];
}

// rotate vector by matrix
void GTE_RTV()
{
	_mac.vx = ((int64)((int64)_globmat.t[0] << 12) + (_globmat.m[0][0] * _vr0.vx) + (_globmat.m[0][1] * _vr0.vy) + (_globmat.m[0][2] * _vr0.vz)) >> 12;
	_mac.vy = ((int64)((int64)_globmat.t[1] << 12) + (_globmat.m[1][0] * _vr0.vx) + (_globmat.m[1][1] * _vr0.vy) + (_globmat.m[1][2] * _vr0.vz)) >> 12;
	_mac.vz = ((int64)((int64)_globmat.t[2] << 12) + (_globmat.m[2][0] * _vr0.vx) + (_globmat.m[2][1] * _vr0.vy) + (_globmat.m[2][2] * _vr0.vz)) >> 12;

	_ir.vx = clamp(_mac.vx, SHRT_MIN, SHRT_MAX);
	_ir.vy = clamp(_mac.vy, SHRT_MIN, SHRT_MAX);
	_ir.vz = clamp(_mac.vz, SHRT_MIN, SHRT_MAX);
}
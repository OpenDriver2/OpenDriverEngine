#ifndef PSX_MATRIX_H
#define PSX_MATRIX_H

// Performs matrix multiplication
// m2 = m0 x m1
MATRIX* MulMatrix0(MATRIX* m0, MATRIX* m1, MATRIX* m2);

// Applies matrix rotation to vector
// v1 = m x v0
VECTOR_NOPAD* ApplyMatrix(MATRIX* m, SVECTOR* v0, VECTOR_NOPAD* v1);

// Applies matrix rotation to short vector
//	v1 = m x v0
SVECTOR* ApplyMatrixSV(MATRIX* m, SVECTOR* v0, SVECTOR* v1);

// Applies matrix to long vector
//	v1 = m x v0
VECTOR_NOPAD* ApplyMatrixLV(MATRIX* m, VECTOR_NOPAD* v0, VECTOR_NOPAD* v1);

// creates a rotation matrix
// m = [r->vx] x [r->vy] x [r->vz]
MATRIX* RotMatrix(SVECTOR* r, MATRIX* m);

// rotates matrix by angle
// m = m * [r] x
MATRIX* RotMatrixX(long r, MATRIX* m);

// rotates matrix by angle
// m = m * [r] y
MATRIX* RotMatrixY(long r, MATRIX* m);

// rotates matrix by angle
// m = m * [r] z
MATRIX* RotMatrixZ(long r, MATRIX* m);

// sets matrix translation
MATRIX* TransMatrix(MATRIX* m, VECTOR_NOPAD* v);

// scales the matrix by arbitary value
MATRIX* ScaleMatrix(MATRIX* m, VECTOR_NOPAD* v);

#define	InitMatrix( __m )	\
		( __m ).m[ 0 ][ 0 ] = ONE,	\
		( __m ).m[ 1 ][ 0 ] = 0,	\
		( __m ).m[ 2 ][ 0 ] = 0,	\
		( __m ).m[ 0 ][ 1 ] = 0,	\
		( __m ).m[ 1 ][ 1 ] = ONE,	\
		( __m ).m[ 2 ][ 1 ] = 0,	\
		( __m ).m[ 0 ][ 2 ] = 0,	\
		( __m ).m[ 1 ][ 2 ] = 0,	\
		( __m ).m[ 2 ][ 2 ] = ONE

extern VECTOR_NOPAD _vr0, _vr1, _vr2, _vr3;
extern VECTOR_NOPAD _mac;
extern SVECTOR_NOPAD _ir;

void gte_SetRotMatrix(const MATRIX* m);
void gte_SetTransMatrix(const MATRIX* m);
void gte_SetTransVector(const VECTOR_NOPAD* v);
void GTE_RTV();

// load short vector
#define gte_ldv0( x )						\
	_vr3 = _vr2;								\
	_vr2 = _vr1;								\
	_vr1 = _vr0;								\
	_vr0.vx = ( ( SVECTOR * )( x ) )->vx;	\
	_vr0.vy = ( ( SVECTOR * )( x ) )->vy;	\
	_vr0.vz = ( ( SVECTOR * )( x ) )->vz

// load long vector
#define gte_ldlvl(x) \
	_vr0.vx = ((VECTOR_NOPAD*)( x ))->vx;	\
	_vr0.vy = ((VECTOR_NOPAD*)( x ))->vy;	\
	_vr0.vz = ((VECTOR_NOPAD*)( x ))->vz

// store long vector
#define gte_stlvnl(x) \
	((VECTOR_NOPAD*)( x ))->vx = _mac.vx;	\
	((VECTOR_NOPAD*)( x ))->vy = _mac.vy;	\
	((VECTOR_NOPAD*)( x ))->vz = _mac.vz

// store short vector
#define gte_stsv(x) \
	((SVECTOR*)( x ))->vx = _ir.vx;	\
	((SVECTOR*)( x ))->vy = _ir.vy;	\
	((SVECTOR*)( x ))->vz = _ir.vz

#define gte_rtir()		GTE_RTV()
#define gte_rtv0tr()	GTE_RTV()

#endif // PSX_MATRIX_H
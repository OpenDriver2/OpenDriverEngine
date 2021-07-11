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

#endif // PSX_MATRIX_H
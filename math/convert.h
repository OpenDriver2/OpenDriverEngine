#ifndef CONVERT_H
#define CONVERT_H

#include "math/psx_math_types.h"
#include "math/Vector.h"

inline Vector3D FromFixedVector(const VECTOR_NOPAD& vec)
{
	float one_by_fixed = 1.0f / ONE_F;
	return Vector3D(vec.vx * one_by_fixed, vec.vy * one_by_fixed, vec.vz * one_by_fixed);
}

inline VECTOR_NOPAD ToFixedVector(const Vector3D& vec)
{
	return { (int)(vec.x * ONE_F), (int)(vec.y * ONE_F), (int)(vec.z * ONE_F) };
}

#endif // CONVERT_H
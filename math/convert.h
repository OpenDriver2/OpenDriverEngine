#ifndef CONVERT_H
#define CONVERT_H

#include "math/psx_math_types.h"
#include "math/Vector.h"

inline Vector3D FromFixedVector(const VECTOR_NOPAD& vec)
{
	return Vector3D(vec.vx / ONE_F, vec.vy / ONE_F, vec.vz / ONE_F);
}

inline VECTOR_NOPAD ToFixedVector(const Vector3D& vec)
{
	return { (int)(vec.x * ONE_F), (int)(vec.y * ONE_F), (int)(vec.z * ONE_F) };
}

#endif // CONVERT_H
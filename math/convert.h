#pragma once

#include "math/psx_math_types.h"
#include "math/Matrix.h"

template <typename T>
inline Vector3D FromFixedVector(const T& vec)
{
	float one_by_fixed = 1.0f / ONE_F;
	return Vector3D(vec.vx * one_by_fixed, vec.vy * one_by_fixed, vec.vz * one_by_fixed);
}

inline VECTOR_NOPAD ToFixedVector(const Vector3D& vec)
{
	return { (int)(vec.x * ONE_F), (int)(vec.y * ONE_F), (int)(vec.z * ONE_F) };
}

inline Matrix4x4 FromFixedMatrix(const MATRIX& m)
{
	return Matrix4x4(
		m.m[0][0] / ONE_F, m.m[0][1] / ONE_F, m.m[0][2] / ONE_F, m.t[0] / ONE_F,
		m.m[1][0] / ONE_F, m.m[1][1] / ONE_F, m.m[1][2] / ONE_F, m.t[1] / ONE_F,
		m.m[2][0] / ONE_F, m.m[2][1] / ONE_F, m.m[2][2] / ONE_F, m.t[2] / ONE_F,
		0.0f, 0.0f, 0.0f, 1.0f);
}

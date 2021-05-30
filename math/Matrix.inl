//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Matrix classes (m2x2,m3x3,m4x4)
//////////////////////////////////////////////////////////////////////////////////

#ifndef MATRIX_INLINE_H
#define MATRIX_INLINE_H

/* --------------------------------------------------------------------------------- */

#define rcDot2(r, c) (m.rows[r].x * n.rows[0][c] + m.rows[r].y * n.rows[1][c])
#define rcDot3(r, c) (m.rows[r].x * n.rows[0][c] + m.rows[r].y * n.rows[1][c] + m.rows[r].z * n.rows[2][c])
#define rcDot4(r, c) (m.rows[r].x * n.rows[0][c] + m.rows[r].y * n.rows[1][c] + m.rows[r].z * n.rows[2][c] + m.rows[r].w * n.rows[3][c])

template <typename T>
inline TMat2<T> operator + (const TMat2<T> &m, const TMat2<T> &n)
{
	return TMat2<T>(m.rows[0] + n.rows[0], m.rows[1] + n.rows[1]);
}

template <typename T>
inline TMat2<T> operator - (const TMat2<T> &m, const TMat2<T> &n)
{
	return TMat2<T>(m.rows[0] - n.rows[0], m.rows[1] - n.rows[1]);
}

template <typename T>
inline TMat2<T> operator - (const TMat2<T> &m)
{
	return TMat2<T>(-m.rows[0], -m.rows[1]);
}

template <typename T>
inline TMat2<T> operator * (const TMat2<T> &m, const TMat2<T> &n)
{
	return TMat2<T>(rcDot2(0, 0), rcDot2(0, 1), rcDot2(1, 0), rcDot2(1, 1));
}

template <typename T>
inline TVec2D<T> operator * (const TMat2<T> &m, const TVec2D<T> &v)
{
	return TVec2D<T>(dot(m.rows[0], v), dot(m.rows[1], v));
}

template <typename T>
inline TMat2<T> operator * (const TMat2<T> &m, const T x)
{
	return TMat2<T>(m.rows[0] * x, m.rows[1] * x);
}

template <typename T>
inline T det(const TMat2<T> &m)
{
	return (m.rows[0].x * m.rows[1].y - m.rows[0].y * m.rows[1].x);
}

template <typename T>
inline TMat2<T> operator ! (const TMat2<T> &m)
{
	T invDet = T(1.0f) / (T)det(m);

	return TMat2<T>(
		 m.rows[1].y, -m.rows[0].y,
		-m.rows[1].x,  m.rows[0].x) * invDet;
}

/* --------------------------------------------------------------------------------- */

template <typename T>
inline TMat3<T> operator + (const TMat3<T> &m, const TMat3<T> &n)
{
	return TMat3<T>(m.rows[0] + n.rows[0], m.rows[1] + n.rows[1], m.rows[2] + n.rows[2]);
}

template <typename T>
inline TMat3<T> operator - (const TMat3<T> &m, const TMat3<T> &n)
{
	return TMat3<T>(m.rows[0] - n.rows[0], m.rows[1] - n.rows[1], m.rows[2] - n.rows[2]);
}

template <typename T>
inline TMat3<T> operator - (const TMat3<T> &m)
{
	return TMat3<T>(-m.rows[0], -m.rows[1], -m.rows[2]);
}

template <typename T>
inline TMat3<T> operator * (const TMat3<T> &m, const TMat3<T> &n)
{
	return TMat3<T>(
		rcDot3(0, 0), rcDot3(0, 1), rcDot3(0, 2),
		rcDot3(1, 0), rcDot3(1, 1), rcDot3(1, 2),
		rcDot3(2, 0), rcDot3(2, 1), rcDot3(2, 2));
}

template <typename T>
inline TVec3D<T> operator * (const TMat3<T> &m, const TVec3D<T> &v)
{
	return TVec3D<T>(dot(m.rows[0], v), dot(m.rows[1], v), dot(m.rows[2], v));
}

template <typename T>
inline TMat3<T> operator * (const TMat3<T> &m, const T x)
{
	return TMat3<T>(m.rows[0] * x, m.rows[1] * x, m.rows[2] * x);
}

template <typename T>
inline T det(const TMat3<T> &m)
{
	return	m.rows[0].x * (m.rows[1].y * m.rows[2].z - m.rows[2].y * m.rows[1].z) -
			m.rows[0].y * (m.rows[1].x * m.rows[2].z - m.rows[1].z * m.rows[2].x) +
			m.rows[0].z * (m.rows[1].x * m.rows[2].y - m.rows[1].y * m.rows[2].x);
}

template <typename T>
inline TMat3<T> operator ! (const TMat3<T> &m)
{
	T invDet = T(1.0f) / (T)det(m);

	return TMat3<T>(m.rows[1].y * m.rows[2].z - m.rows[1].z * m.rows[2].y, m.rows[2].y * m.rows[0].z - m.rows[0].y * m.rows[2].z, m.rows[0].y * m.rows[1].z - m.rows[1].y * m.rows[0].z,
					m.rows[1].z * m.rows[2].x - m.rows[1].x * m.rows[2].z, m.rows[0].x * m.rows[2].z - m.rows[2].x * m.rows[0].z, m.rows[1].x * m.rows[0].z - m.rows[0].x * m.rows[1].z,
					m.rows[1].x * m.rows[2].y - m.rows[2].x * m.rows[1].y, m.rows[2].x * m.rows[0].y - m.rows[0].x * m.rows[2].y, m.rows[0].x * m.rows[1].y - m.rows[0].y * m.rows[1].x) * invDet;
}

/* --------------------------------------------------------------------------------- */


template <typename T>
inline const TVec3D<T>& TMat4<T>::getTranslationComponent() const
{
	return rows[3].xyz();
}

template <typename T>
inline TMat3<T> TMat4<T>::getRotationComponent() const
{
	return TMat3<T>(rows[0].xyz(), rows[1].xyz(), rows[2].xyz());
}

template <typename T>
inline const TVec3D<T> TMat4<T>::getTranslationComponentTransposed() const
{
	return TVec3D<T>(rows[0].w, rows[1].w, rows[2].w);
}

template <typename T>
inline TMat3<T> TMat4<T>::getRotationComponentTransposed() const
{
	return TMat3<T>(
		TVec3D<T>(rows[0].x, rows[1].x, rows[2].x),
		TVec3D<T>(rows[0].y, rows[1].y, rows[2].y),
		TVec3D<T>(rows[0].z, rows[1].z, rows[2].z));
}

template <typename T>
inline void TMat4<T>::translate(const TVec3D<T> &v)
{
	rows[0].w += dot(rows[0].xyz(), v);
	rows[1].w += dot(rows[1].xyz(), v);
	rows[2].w += dot(rows[2].xyz(), v);
	rows[3].w += dot(rows[3].xyz(), v);
}

template <typename T>
inline void TMat4<T>::setTranslation(const TVec3D<T> &v)
{
	rows[3][0] = v.x;
	rows[3][1] = v.y;
	rows[3][2] = v.z;
}

template <typename T>
inline void TMat4<T>::setRotation(const TVec3D<T> &v)
{
	float cx,sx,cy,sy,cz,sz;
	SinCos(v.x,&sx,&cx);
	SinCos(v.y,&sy,&cy);
	SinCos(v.z,&sz,&cz);

	rows[0][0] = cy * cz;
	rows[0][1] = cy * sz;
	rows[0][2] = -sy;

	rows[1][0] = sx * sy * cz - cx * sz;
	rows[1][1] = sx * sy * sz + cx * cz;
	rows[1][2] = sx * cy;

	rows[2][0] = cx * sy * cz + sx * sz;
	rows[2][1] = cx * sy * sz - sx * cz;
	rows[2][2] = cx * cy;

	rows[3][3] = 1.0f;
}

template <typename T>
inline TMat4<T> operator + (const TMat4<T> &m, const TMat4<T> &n)
{
	return TMat4<T>(m.rows[0] + n.rows[0], m.rows[1] + n.rows[1], m.rows[2] + n.rows[2], m.rows[3] + n.rows[3]);
}

template <typename T>
inline TMat4<T> operator - (const TMat4<T> &m, const TMat4<T> &n)
{
	return TMat4<T>(m.rows[0] - n.rows[0], m.rows[1] - n.rows[1], m.rows[2] - n.rows[2], m.rows[3] - n.rows[3]);
}

template <typename T>
inline TMat4<T> operator - (const TMat4<T> &m)
{
	return TMat4<T>(-m.rows[0], -m.rows[1], -m.rows[2], -m.rows[3]);
}

template <typename T>
inline TMat4<T> operator * (const TMat4<T> &m, const TMat4<T> &n)
{
	return TMat4<T>(
		rcDot4(0, 0), rcDot4(0, 1), rcDot4(0, 2), rcDot4(0, 3),
		rcDot4(1, 0), rcDot4(1, 1), rcDot4(1, 2), rcDot4(1, 3),
		rcDot4(2, 0), rcDot4(2, 1), rcDot4(2, 2), rcDot4(2, 3),
		rcDot4(3, 0), rcDot4(3, 1), rcDot4(3, 2), rcDot4(3, 3));
}

template <typename T>
inline TVec4D<T> operator * (const TMat4<T> &m, const TVec4D<T> &v)
{
	return TVec4D<T>(dot(m.rows[0], v), dot(m.rows[1], v), dot(m.rows[2], v), dot(m.rows[3], v));
}

template <typename T>
inline TVec3D<T> operator * (const TMat4<T> &m, const TVec3D<T> &v)
{
	return TVec3D<T>(dot(m.rows[0].xyz(), v), dot(m.rows[1].xyz(), v), dot(m.rows[2].xyz(), v));
}

template <typename T>
inline TMat4<T> operator * (const TMat4<T> &m, const T x)
{
	return TMat4<T>(m.rows[0] * x, m.rows[1] * x, m.rows[2] * x, m.rows[3] * x);
}

template <typename T>
inline TMat4<T> operator ! (const TMat4<T> &m)
{
	TMat4<T> mat;

	T p00 = m.rows[2][2] * m.rows[3][3];
	T p01 = m.rows[3][2] * m.rows[2][3];
	T p02 = m.rows[1][2] * m.rows[3][3];
	T p03 = m.rows[3][2] * m.rows[1][3];
	T p04 = m.rows[1][2] * m.rows[2][3];
	T p05 = m.rows[2][2] * m.rows[1][3];
	T p06 = m.rows[0][2] * m.rows[3][3];
	T p07 = m.rows[3][2] * m.rows[0][3];
	T p08 = m.rows[0][2] * m.rows[2][3];
	T p09 = m.rows[2][2] * m.rows[0][3];
	T p10 = m.rows[0][2] * m.rows[1][3];
	T p11 = m.rows[1][2] * m.rows[0][3];

	mat.rows[0][0] = (p00 * m.rows[1][1] + p03 * m.rows[2][1] + p04 * m.rows[3][1]) - (p01 * m.rows[1][1] + p02 * m.rows[2][1] + p05 * m.rows[3][1]);
	mat.rows[0][1] = (p01 * m.rows[0][1] + p06 * m.rows[2][1] + p09 * m.rows[3][1]) - (p00 * m.rows[0][1] + p07 * m.rows[2][1] + p08 * m.rows[3][1]);
	mat.rows[0][2] = (p02 * m.rows[0][1] + p07 * m.rows[1][1] + p10 * m.rows[3][1]) - (p03 * m.rows[0][1] + p06 * m.rows[1][1] + p11 * m.rows[3][1]);
	mat.rows[0][3] = (p05 * m.rows[0][1] + p08 * m.rows[1][1] + p11 * m.rows[2][1]) - (p04 * m.rows[0][1] + p09 * m.rows[1][1] + p10 * m.rows[2][1]);
	mat.rows[1][0] = (p01 * m.rows[1][0] + p02 * m.rows[2][0] + p05 * m.rows[3][0]) - (p00 * m.rows[1][0] + p03 * m.rows[2][0] + p04 * m.rows[3][0]);
	mat.rows[1][1] = (p00 * m.rows[0][0] + p07 * m.rows[2][0] + p08 * m.rows[3][0]) - (p01 * m.rows[0][0] + p06 * m.rows[2][0] + p09 * m.rows[3][0]);
	mat.rows[1][2] = (p03 * m.rows[0][0] + p06 * m.rows[1][0] + p11 * m.rows[3][0]) - (p02 * m.rows[0][0] + p07 * m.rows[1][0] + p10 * m.rows[3][0]);
	mat.rows[1][3] = (p04 * m.rows[0][0] + p09 * m.rows[1][0] + p10 * m.rows[2][0]) - (p05 * m.rows[0][0] + p08 * m.rows[1][0] + p11 * m.rows[2][0]);

	T q00 = m.rows[2][0] * m.rows[3][1];
	T q01 = m.rows[3][0] * m.rows[2][1];
	T q02 = m.rows[1][0] * m.rows[3][1];
	T q03 = m.rows[3][0] * m.rows[1][1];
	T q04 = m.rows[1][0] * m.rows[2][1];
	T q05 = m.rows[2][0] * m.rows[1][1];
	T q06 = m.rows[0][0] * m.rows[3][1];
	T q07 = m.rows[3][0] * m.rows[0][1];
	T q08 = m.rows[0][0] * m.rows[2][1];
	T q09 = m.rows[2][0] * m.rows[0][1];
	T q10 = m.rows[0][0] * m.rows[1][1];
	T q11 = m.rows[1][0] * m.rows[0][1];

	mat.rows[2][0] = (q00 * m.rows[1][3] + q03 * m.rows[2][3] + q04 * m.rows[3][3]) - (q01 * m.rows[1][3] + q02 * m.rows[2][3] + q05 * m.rows[3][3]);
	mat.rows[2][1] = (q01 * m.rows[0][3] + q06 * m.rows[2][3] + q09 * m.rows[3][3]) - (q00 * m.rows[0][3] + q07 * m.rows[2][3] + q08 * m.rows[3][3]);
	mat.rows[2][2] = (q02 * m.rows[0][3] + q07 * m.rows[1][3] + q10 * m.rows[3][3]) - (q03 * m.rows[0][3] + q06 * m.rows[1][3] + q11 * m.rows[3][3]);
	mat.rows[2][3] = (q05 * m.rows[0][3] + q08 * m.rows[1][3] + q11 * m.rows[2][3]) - (q04 * m.rows[0][3] + q09 * m.rows[1][3] + q10 * m.rows[2][3]);
	mat.rows[3][0] = (q02 * m.rows[2][2] + q05 * m.rows[3][2] + q01 * m.rows[1][2]) - (q04 * m.rows[3][2] + q00 * m.rows[1][2] + q03 * m.rows[2][2]);
	mat.rows[3][1] = (q08 * m.rows[3][2] + q00 * m.rows[0][2] + q07 * m.rows[2][2]) - (q06 * m.rows[2][2] + q09 * m.rows[3][2] + q01 * m.rows[0][2]);
	mat.rows[3][2] = (q06 * m.rows[1][2] + q11 * m.rows[3][2] + q03 * m.rows[0][2]) - (q10 * m.rows[3][2] + q02 * m.rows[0][2] + q07 * m.rows[1][2]);
	mat.rows[3][3] = (q10 * m.rows[2][2] + q04 * m.rows[0][2] + q09 * m.rows[1][2]) - (q08 * m.rows[1][2] + q11 * m.rows[2][2] + q05 * m.rows[0][2]);

	return mat * (T(1.0f) / (m.rows[0][0] * mat.rows[0][0] + m.rows[1][0] * mat.rows[0][1] + m.rows[2][0] * mat.rows[0][2] + m.rows[3][0] * mat.rows[0][3]));
}

template <typename T>
inline TMat2<T> transpose(const TMat2<T> &m)
{
	return TMat2<T>(m.rows[0].x, m.rows[1].x,
					m.rows[0].y, m.rows[1].y);
}

template <typename T>
inline TMat3<T> transpose(const TMat3<T> &m)
{
	return TMat3<T>(
		m.rows[0].x, m.rows[1].x, m.rows[2].x,
		m.rows[0].y, m.rows[1].y, m.rows[2].y,
		m.rows[0].z, m.rows[1].z, m.rows[2].z);
}

template <typename T>
inline TMat4<T> transpose(const TMat4<T> &m)
{
	return TMat4<T>(
		m.rows[0].x, m.rows[1].x, m.rows[2].x, m.rows[3].x,
		m.rows[0].y, m.rows[1].y, m.rows[2].y, m.rows[3].y,
		m.rows[0].z, m.rows[1].z, m.rows[2].z, m.rows[3].z,
		m.rows[0].w, m.rows[1].w, m.rows[2].w, m.rows[3].w);
}

template <typename T>
inline TMat4<T> shearX(const T shear)
{
	return TMat4<T>(1,shear,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
}

template <typename T>
inline TMat4<T> shearY(const T shear)
{
	return TMat4<T>(1,0,0,0, shear,1,0,0, 0,0,1,0, 0,0,0,1);
}

template <typename T>
inline TMat4<T> shearZ(const T shear)
{
	return TMat4<T>(1,0,0,0, 0,1,0,0, 0,shear,1,0, 0,0,0,1);
}

template <typename T>
inline TMat2<T> rotate2(const T angle)
{
	float cosA,sinA;
	SinCos((float)angle,&sinA,&cosA);

	return TMat2<T>(cosA, -sinA, sinA, cosA);
}

template <typename T>
inline TMat2<T> scale2(const T u, const T v)
{
	return TMat2<T>(u,0, 0,v);
}

//---------------------------------------------------------------------------------------------------------------

template <typename T>
inline TMat3<T> rotateX3(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat3<T>(
		1, 0,     0,
		0, cosA, -sinA,
		0, sinA,  cosA);
}

template <typename T>
inline TMat3<T> rotateY3(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat3<T>(
		cosA, 0, -sinA,
		0,    1,  0,
		sinA, 0,  cosA);
}

template <typename T>
inline TMat3<T> rotateZ3(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat3<T>(
		cosA, -sinA, 0,
		sinA,  cosA, 0,
		0,     0,    1);
}

template <typename T>
inline TMat3<T> rotateXY3(const T angleX, const T angleY)
{
	float cosX,sinX,cosY,sinY;

	SinCos(angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);

	return TMat3<T>(
		 cosY,        0,    -sinY,
		-sinX * sinY, cosX, -sinX * cosY,
		 cosX * sinY, sinX,  cosX * cosY);
}

template <typename T>
inline TMat3<T> rotateYX3(const T angleX, const T angleY)
{
	float cosX,sinX,cosY,sinY;

	SinCos(angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);

	return TMat3<T>(
		cosY, -sinX * sinY, -cosX * sinY,
		0,     cosX,        -sinX,
		sinY,  sinX * cosY,  cosX * cosY);
}

template <typename T>
inline TMat3<T> rotateXYZ3(const T angleX, const T angleY, const T angleZ)
{
	float cx,sx,cy,sy,cz,sz;

	SinCos(angleX,&sx,&cx);
	SinCos(angleY,&sy,&cy);
	SinCos(angleZ,&sz,&cz);

	float sxsy = sx*sy;
	float cxsy = cx*sy;

	return TMat3<T>(
		cy*cz,
		cy*sz,
		-sy,

		sxsy*cz + cx*-sz,
		sxsy*sz + cx*cz,
		sx*cy,

		cxsy*cz + sx*sz,
		cxsy*sz + -sx*cz,
		cx*cy
	);
}

template <typename T>
inline TMat3<T> rotateZXY3(const T angleX, const T angleY, const T angleZ)
{
	float cx,sx,cy,sy,cz,sz;

	SinCos(angleX,&sx,&cx);
	SinCos(angleY,&sy,&cy);
	SinCos(angleZ,&sz,&cz);

	float sxsy = sx*sy;
	float sxcy = sx*cy;
	float sysz = sy*sz;

	return TMat3<T>(
		cy*cz + sx*sysz,
		-cx*sz,
		sxcy*sz - sy*cz,

		cy*sz - sxsy*cz,
		cx*cz,
		-sysz - sxcy*cz,

		cx*sy,
		sx,
		cx*cy
		);
}

template <typename T>
inline TMat3<T> rotateYZX3(const T angleX, const T angleY, const T angleZ)
{
	float cosX,sinX,cosY,sinY,cosZ,sinZ;

	SinCos(-angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);
	SinCos(-angleZ,&sinZ,&cosZ);

	return TMat3<T>(
		cosY * cosZ,
		cosY*cosX * sinZ + sinY*sinX,
		cosY*sinX * sinZ - sinY*cosX,

		-sinZ,
		cosZ * cosX,
		cosZ * sinX,

		sinY * cosZ,
		sinZ * sinY*cosX - cosY*sinX,
		sinZ * sinY*sinX + cosY*cosX);
}

// maxtrix3 axis angle rotation
template <typename T>
inline TMat3<T> rotateAxis3(const TVec3D<T>& axis, T angle)
{
	// from page 32(45) of glspec.dvi
	TMat3<T> uut(	axis[0]*axis[0], axis[0]*axis[1], axis[0]*axis[2],
					axis[1]*axis[0], axis[1]*axis[1], axis[1]*axis[2],
					axis[2]*axis[0], axis[2]*axis[1], axis[2]*axis[2]);
  
	TMat3<T> s(	0,			-axis[2],	axis[1],
				axis[2],	0,			-axis[0],
				-axis[1],	axis[0],	0);
  
	return (uut + cos(angle) * (_identity3<T>() - uut) + sin(angle) * s);
}

//-------------------------------------------------------------------------------------------------------------------

template <typename T>
inline TMat4<T> rotateX4(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat4<T>(
		1, 0,     0,    0,
		0, cosA, -sinA, 0,
		0, sinA,  cosA, 0,
		0, 0,     0,    1);
}

template <typename T>
inline TMat4<T> rotateY4(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat4<T>(
		cosA, 0, -sinA, 0,
		0,    1,  0,    0,
		sinA, 0,  cosA, 0,
		0,    0,  0,    1);
}

template <typename T>
inline TMat4<T> rotateZ4(const T angle)
{
	float cosA,sinA;
	SinCos(angle,&sinA,&cosA);

	return TMat4<T>(
		cosA, -sinA, 0, 0,
		sinA,  cosA, 0, 0,
		0,     0,    1, 0,
		0,     0,    0, 1);
}

template <typename T>
inline TMat4<T> rotateXY4(const T angleX, const T angleY)
{
	float cosX,sinX,cosY,sinY;

	SinCos(angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);

	return TMat4<T>(
		 cosY,        0,    -sinY,        0,
		-sinX * sinY, cosX, -sinX * cosY, 0,
		 cosX * sinY, sinX,  cosX * cosY, 0,
		 0,           0,     0,           1);
}

template <typename T>
inline TMat4<T> rotateYX4(const T angleX, const T angleY)
{
	float cosX,sinX,cosY,sinY;

	SinCos(angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);

	return TMat4<T>(
		cosY, -sinX * sinY, -cosX * sinY, 0,
		0,     cosX,        -sinX,        0,
		sinY,  sinX * cosY,  cosX * cosY, 0,
		0,     0,            0,           1);
}

template <typename T>
inline TMat4<T> rotateXYZ4(const T angleX, const T angleY, const T angleZ)
{
	float cx,sx,cy,sy,cz,sz;

	SinCos(angleX,&sx,&cx);
	SinCos(angleY,&sy,&cy);
	SinCos(angleZ,&sz,&cz);

	float sxsy = sx*sy;
	float cxsy = cx*sy;

	return TMat4<T>(
		cy*cz,
		cy*sz,
		-sy,
		0,

		sxsy*cz + cx*-sz,
		sxsy*sz + cx*cz,
		sx*cy,

		0,
		cxsy*cz + sx*sz,
		cxsy*sz + -sx*cz,
		cx*cy,
		0,

		0,
		0,
		0,
		1
	);
}

template <typename T>
inline TMat4<T> rotateZXY4(const T angleX, const T angleY, const T angleZ)
{
	float cx,sx,cy,sy,cz,sz;

	SinCos(angleX,&sx,&cx);
	SinCos(angleY,&sy,&cy);
	SinCos(angleZ,&sz,&cz);

	float sxsy = sx*sy;
	float sxcy = sx*cy;
	float sysz = sy*sz;

	return TMat4<T>(
		cy*cz + sx*sysz,
		-cx*sz,
		sxcy*sz - sy*cz,
		0,

		cy*sz - sxsy*cz,
		cx*cz,
		-sysz - sxcy*cz,
		0,

		cx*sy,
		sx,
		cx*cy,
		0,

		0,
		0,
		0,
		1);
}

template <typename T>
inline TMat4<T> rotateYZX4(const T angleX, const T angleY, const T angleZ)
{
	float cosX,sinX,cosY,sinY,cosZ,sinZ;

	SinCos(-angleX,&sinX,&cosX);
	SinCos(angleY,&sinY,&cosY);
	SinCos(-angleZ,&sinZ,&cosZ);

	return TMat4<T>(
		cosY * cosZ,
		cosY*cosX * sinZ + sinY*sinX,
		cosY*sinX * sinZ - sinY*cosX,
		0,

		-sinZ,
		cosZ * cosX,
		cosZ * sinX,
		0,

		sinY * cosZ,
		sinZ * sinY*cosX - cosY*sinX,
		sinZ * sinY*sinX + cosY*cosX,
		0,

		0,
		0,
		0,
		1);
}

//-------------------------------------------------------------------------------------------------------------------

template <typename T>
inline TMat4<T> translate(const TVec3D<T> &v)
{
	return TMat4<T>(1,0,0,v.x, 0,1,0,v.y, 0,0,1,v.z, 0,0,0,1);
}

template <typename T>
inline TMat4<T> translate(const T x, const T y, const T z)
{
	return TMat4<T>(1,0,0,x, 0,1,0,y, 0,0,1,z, 0,0,0,1);
}

template <typename T>
inline TMat3<T> scale3(const T x, const T y, const T z)
{
	return TMat3<T>(x,0,0, 0,y,0, 0,0,z);
}

template <typename T>
inline TMat4<T> scale4(const T x, const T y, const T z)
{
	return TMat4<T>(x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1);
}

template <typename T>
inline TMat2<T> _identity2()
{
	return Matrix2x2(1,0,
					 0,1);
}

template <typename T>
inline TMat3<T> _identity3()
{
	return TMat3<T>(1,0,0,
					0,1,0,
					0,0,1);
}

template <typename T>
inline TMat4<T> _identity4()
{
	return TMat4<T>(1,0,0,0,
					0,1,0,0,
					0,0,1,0,
					0,0,0,1);
}

template <typename T>
inline TVec3D<T> transform3(const TVec3D<T> &in, const TMat3<T>& mat)
{
	return TVec3D<T>(
		(in[0] * mat.rows[0][0] +
	     in[1] * mat.rows[1][0] +
	     in[2] * mat.rows[2][0]),

	    (in[0] * mat.rows[0][1] +
	     in[1] * mat.rows[1][1] +
         in[2] * mat.rows[2][1]),

	    (in[0] * mat.rows[0][2] +
	     in[1] * mat.rows[1][2] +
	     in[2] * mat.rows[2][2]));
}

template <typename T>
inline TVec3D<T> transform3(const TVec3D<T> &in, const TMat4<T>& mat)
{
	return TVec3D<T>(
		(in[0] * mat.rows[0][0] +
	    in[1] * mat.rows[1][0] +
	    in[2] * mat.rows[2][0]),

		(in[0] * mat.rows[0][1] +
	    in[1] * mat.rows[1][1] +
	    in[2] * mat.rows[2][1]),

	    (in[0] * mat.rows[0][2] +
	    in[1] * mat.rows[1][2] +
	    in[2] * mat.rows[2][2]) );
}

template <typename T>
inline TVec3D<T> transform4(const TVec3D<T> &in, const TMat4<T>& mat)
{
	return TVec3D<T>(
		(in[0] *	mat.rows[0][0] +
	    in[1] *		mat.rows[1][0] +
	    in[2] *		mat.rows[2][0] +
	   mat.rows[3][0]),

		(in[0] *	mat.rows[0][1] +
	    in[1] *		mat.rows[1][1] +
	    in[2] *		mat.rows[2][1] +
	    mat.rows[3][1]),

	    (in[0] *	mat.rows[0][2] +
	    in[1] *		mat.rows[1][2] +
	    in[2] *		mat.rows[2][2]+
	    mat.rows[3][2]));
}

template <typename T>
inline TVec3D<T> inverseRotateVec(const TVec3D<T> &vector, const TMat4<T> &matrix)
{
	return TVec3D<T>(vector[0] * matrix.rows[0][0] + vector[1] * matrix.rows[0][1] + vector[2] * matrix.rows[0][2],
					 vector[0] * matrix.rows[1][0] + vector[1] * matrix.rows[1][1] + vector[2] * matrix.rows[1][2],
					 vector[0] * matrix.rows[2][0] + vector[1] * matrix.rows[2][1] + vector[2] * matrix.rows[2][2]);
}

template <typename T>
inline TVec3D<T> inverseTranslateVec(const TVec3D<T> &vector, const TMat4<T> &matrix)
{
	TVec3D<T> tmp;

	return TVec3D<T>(vector[0] - matrix.rows[3][0],
					 vector[1] - matrix.rows[3][1],
					 vector[2] - matrix.rows[3][2]);

	return tmp;
}

template <typename T>
inline TMat4<T> inverse(const TMat4<T> &m)
{
	return !m;
}

#endif // MATRIX_INLINE_H

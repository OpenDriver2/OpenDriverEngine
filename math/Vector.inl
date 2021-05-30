//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Vector math base - inline
//////////////////////////////////////////////////////////////////////////////////

#ifndef VECTOR_INLINE_H
#define VECTOR_INLINE_H

#include "Vector.h"
#include "stdlib.h"		// for strtol

/* --------------------------------------------------------------------------------- */

template <class T>
template <class T2>
inline void TVec2D<T>::operator += (const T2 s)
{
	x += s;
	y += s;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator += (const TVec2D<T2> &v)
{
	x += v.x;
	y += v.y;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator -= (const T2 s)
{
	x -= s;
	y -= s;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator -= (const TVec2D<T2> &v)
{
	x -= v.x;
	y -= v.y;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator *= (const T2 s)
{
	x *= s;
	y *= s;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator *= (const TVec2D<T2> &v)
{
	x *= v.x;
	y *= v.y;
}

template <class T>
template <class T2>
inline void TVec2D<T>::operator /= (const T2 s)
{
	x /= s;
	y /= s;
}

template <class T>
template <class T2>
void TVec2D<T>::operator /= (const TVec2D<T2> &v)
{
	x /= v.x;
	y /= v.y;
}

template <typename T, typename T2>
inline TVec2D<T> operator + (const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return TVec2D<T>(u.x + v.x, u.y + v.y);
}

template <typename T, typename T2>
inline TVec2D<T> operator + (const TVec2D<T> &v, const T2 s)
{
	return TVec2D<T>(v.x + s, v.y + s);
}

template <typename T, typename T2>
inline TVec2D<T> operator + (const T s, const TVec2D<T2> &v)
{
	return TVec2D<T>(v.x + s, v.y + s);
}

template <typename T, typename T2>
inline TVec2D<T> operator - (const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return TVec2D<T>(u.x - v.x, u.y - v.y);
}

template <typename T, typename T2>
inline TVec2D<T> operator - (const TVec2D<T> &v, const T2 s)
{
	return TVec2D<T>(v.x - s, v.y - s);
}

template <typename T, typename T2>
inline TVec2D<T> operator - (const T s, const TVec2D<T> &v)
{
	return TVec2D<T>(v.x - s, v.y - s);
}

template <typename T>
inline TVec2D<T> operator - (const TVec2D<T> &v)
{
	return TVec2D<T>(-v.x, -v.y);
}

template <typename T, typename T2>
inline TVec2D<T> operator * (const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return TVec2D<T>(u.x * v.x, u.y * v.y);
}

template <typename T, typename T2>
inline TVec2D<T> operator * (const T s, const TVec2D<T2> &v)
{
	return TVec2D<T>(v.x * s, v.y * s);
}

template <typename T, typename T2>
inline TVec2D<T> operator * (const TVec2D<T> &v, const T2 s)
{
	return TVec2D<T>(v.x * s, v.y * s);
}

template <typename T, typename T2>
inline TVec2D<T> operator / (const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return TVec2D<T>(u.x / v.x, u.y / v.y);
}

template <typename T, typename T2>
inline TVec2D<T> operator / (const TVec2D<T> &v, const T2 s)
{
	return TVec2D<T>(v.x / s, v.y / s);
}

template <typename T, typename T2>
inline TVec2D<T> operator / (const T s, const TVec2D<T2> &v)
{
	return TVec2D<T>(s / v.x, s / v.y);
}

template <typename T, typename T2>
inline bool operator<=(const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return u.x<=v.x && u.y<=v.y;
}

template <typename T, typename T2>
inline bool operator>=(const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return u.x>=v.x && u.y>=v.y;
}

template <typename T, typename T2>
inline bool operator<(const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return u.x<v.x && u.y<v.y;
}

template <typename T, typename T2>
inline bool operator>(const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return u.x>v.x && u.y>v.y;
}

template <typename T, typename T2>
inline bool operator == (const TVec2D<T> &u, const TVec2D<T2> &v)
{
	return (u.x == v.x && u.y == v.y);
}

/* --------------------------------------------------------------------------------- */

template <class T>
template <class T2>
inline void TVec3D<T>::operator += (const T2 s)
{
	x += s;
	y += s;
	z += s;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator += (const TVec3D<T2> &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator -= (const T2 s)
{
	x -= s;
	y -= s;
	z -= s;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator -= (const TVec3D<T2> &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator *= (const T2 s)
{
	x *= s;
	y *= s;
	z *= s;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator *= (const TVec3D<T2> &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator /= (const T2 s)
{
	x /= s;
	y /= s;
	z /= s;
}

template <class T>
template <class T2>
inline void TVec3D<T>::operator /= (const TVec3D<T2> &v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
}

template <typename T, typename T2>
inline TVec3D<T> operator + (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return TVec3D<T>(u.x + v.x, u.y + v.y, u.z + v.z);
}

template <typename T, typename T2>
inline TVec3D<T> operator + (const TVec3D<T> &v, const T2 s)
{
	return TVec3D<T>(v.x + s, v.y + s, v.z + s);
}

template <typename T, typename T2>
inline TVec3D<T> operator + (const T s, const TVec3D<T2> &v)
{
	return TVec3D<T>(v.x + s, v.y + s, v.z + s);
}

template <typename T, typename T2>
inline TVec3D<T> operator - (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return TVec3D<T>(u.x - v.x, u.y - v.y, u.z - v.z);
}

template <typename T, typename T2>
inline TVec3D<T> operator - (const TVec3D<T> &v, const T2 s)
{
	return TVec3D<T>(v.x - s, v.y - s, v.z - s);
}

template <typename T, typename T2>
inline TVec3D<T> operator - (const T s, const TVec3D<T2> &v)
{
	return TVec3D<T>(v.x - s, v.y - s, v.z - s);
}

template <typename T>
inline TVec3D<T> operator - (const TVec3D<T> &v)
{
	return TVec3D<T>(-v.x, -v.y, -v.z);
}

template <typename T, typename T2>
inline TVec3D<T> operator * (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return TVec3D<T>(u.x * v.x, u.y * v.y, u.z * v.z);
}

template <typename T, typename T2>
inline TVec3D<T> operator * (const T s, const TVec3D<T2> &v)
{
	return TVec3D<T>(v.x * s, v.y * s, v.z * s);
}

template <typename T, typename T2>
inline TVec3D<T> operator * (const TVec3D<T> &v, const T2 s)
{
	return TVec3D<T>(v.x * s, v.y * s, v.z * s);
}

template <typename T, typename T2>
inline TVec3D<T> operator / (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return TVec3D<T>(u.x / v.x, u.y / v.y, u.z / v.z);
}

template <typename T, typename T2>
TVec3D<T> operator / (const TVec3D<T> &v, const T2 s)
{
	return TVec3D<T>(v.x / s, v.y / s, v.z / s);
}

template <typename T, typename T2>
inline TVec3D<T> operator / (const T s, const TVec3D<T2> &v)
{
	return TVec3D<T>(s / v.x, s / v.y, s / v.z);
}

template <typename T, typename T2>
inline TVec3D<T> operator ^ (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return cross(u, v);
}

template <typename T, typename T2>
inline bool operator == (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return (u.x == v.x && u.y == v.y && u.z == v.z);
}

template <typename T, typename T2>
inline T operator & ( const TVec3D<T>& u, const TVec3D<T2>& v )
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}

template <typename T, typename T2>
inline bool operator <= (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return u.x<=v.x && u.y<=v.y && u.z<=v.z;
}

template <typename T, typename T2>
inline bool operator >= (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return u.x>=v.x && u.y>=v.y && u.z>=v.z;
}

template <typename T, typename T2>
inline bool operator < (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return u.x<v.x && u.y<v.y && u.z<v.z;
}

template <typename T, typename T2>
inline bool operator > (const TVec3D<T> &u, const TVec3D<T2> &v)
{
	return u.x>v.x && u.y>v.y && u.z>v.z;
}

/* --------------------------------------------------------------------------------- */

template <class T>
template <class T2>
inline void TVec4D<T>::operator += (const T2 s)
{
	x += s;
	y += s;
	z += s;
	w += s;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator += (const TVec4D<T2> &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator -= (const T2 s)
{
	x -= s;
	y -= s;
	z -= s;
	w -= s;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator -= (const TVec4D<T2> &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator *= (const T2 s)
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator *= (const TVec4D<T2> &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator /= (const T2 s)
{
	x /= s;
	y /= s;
	z /= s;
	w /= s;
}

template <class T>
template <class T2>
inline void TVec4D<T>::operator /= (const TVec4D<T2> &v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
}

template <typename T, typename T2>
inline TVec4D<T> operator + (const TVec4D<T> &u, const TVec4D<T2> &v)
{
	return TVec4D<T>(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
}

template <typename T, typename T2>
inline TVec4D<T> operator + (const TVec4D<T> &v, const T2 s)
{
	return TVec4D<T>(v.x + s, v.y + s, v.z + s, v.w + s);
}

template <typename T, typename T2>
inline TVec4D<T> operator + (const T s, const TVec4D<T2> &v)
{
	return TVec4D<T>(v.x + s, v.y + s, v.z + s, v.w + s);
}

template <typename T, typename T2>
inline TVec4D<T> operator - (const TVec4D<T> &u, const TVec4D<T2> &v)
{
	return TVec4D<T>(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
}

template <typename T, typename T2>
inline TVec4D<T> operator - (const TVec4D<T> &v, const T2 s)
{
	return TVec4D<T>(v.x - s, v.y - s, v.z - s, v.w - s);
}

template <typename T, typename T2>
inline TVec4D<T> operator - (const T s, const TVec4D<T2> &v)
{
	return TVec4D<T>(v.x - s, v.y - s, v.z - s, v.w - s);
}

template <typename T>
inline TVec4D<T> operator - (const TVec4D<T> &v)
{
	return TVec4D<T>(-v.x, -v.y, -v.z, -v.w);
}

template <typename T, typename T2>
inline TVec4D<T> operator * (const TVec4D<T> &u, const TVec4D<T2> &v)
{
	return TVec4D<T>(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w);
}

template <typename T, typename T2>
inline TVec4D<T> operator * (const T s, const TVec4D<T2> &v)
{
	return TVec4D<T>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template <typename T, typename T2>
inline TVec4D<T> operator * (const TVec4D<T> &v, const T2 s)
{
	return TVec4D<T>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template <typename T, typename T2>
inline TVec4D<T> operator / (const TVec4D<T> &u, const TVec4D<T2> &v)
{
	return TVec4D<T>(u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w);
}

template <typename T, typename T2>
inline TVec4D<T> operator / (const TVec4D<T> &v, const T2 s)
{
	return TVec4D<T>(v.x / s, v.y / s, v.z / s, v.w / s);
}

template <typename T, typename T2>
inline TVec4D<T> operator / (const T s, const TVec4D<T2> &v)
{
	return TVec4D<T>(s / v.x, s / v.y, s / v.z, s / v.w);
}

template <typename T, typename T2>
inline bool operator == (const TVec4D<T> &u, const TVec4D<T2> &v)
{
	return (u.x == v.x && u.y == v.y && u.z == v.z && u.w && v.w);
}

/* --------------------------------------------------------------------------------- */

// compares vector2 with epsilon
template <typename T>
inline bool compare_epsilon(const TVec2D<T> &u, const TVec2D<T> &v, const T eps)
{
	return	fsimilar(u.x, v.x, eps) &&
			fsimilar(u.y, v.y, eps);
}

// compares vector3 with epsilon
template <typename T>
inline bool compare_epsilon(const TVec3D<T> &u, const TVec3D<T> &v, const T eps)
{
	return	fsimilar(u.x, v.x, eps) &&
			fsimilar(u.y, v.y, eps) &&
			fsimilar(u.z, v.z, eps);
}

template <typename T>
inline bool compare_epsilon(const TVec4D<T> &u, const TVec4D<T> &v, const T eps)
{
	return	fsimilar(u.x, v.x, eps) &&
			fsimilar(u.y, v.y, eps) &&
			fsimilar(u.z, v.z, eps) &&
			fsimilar(u.w, v.w, eps);
}

template <typename T>
inline T dot(const TVec2D<T> &u, const TVec2D<T> &v)
{
	return u.x * v.x + u.y * v.y;
}

template <typename T>
inline T dot(const TVec3D<T> &u, const TVec3D<T> &v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

template <typename T>
inline T dot(const TVec4D<T> &u, const TVec4D<T> &v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

template <typename T>
inline T lerp(const T u, const T v, const float x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec2D<T> lerp(const TVec2D<T> &u, const TVec2D<T> &v, const float x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec3D<T> lerp(const TVec3D<T> &u, const TVec3D<T> &v, const float x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec4D<T> lerp(const TVec4D<T> &u, const TVec4D<T> &v, const float x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec2D<T> lerp(const TVec2D<T> &u, const TVec2D<T> &v, const TVec2D<T> &x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec3D<T> lerp(const TVec3D<T> &u, const TVec3D<T> &v, const TVec3D<T> &x)
{
	return u + x * (v - u);
}

template <typename T>
inline TVec4D<T> lerp(const TVec4D<T> &u, const TVec4D<T> &v, const TVec4D<T> &x)
{
	return u + x * (v - u);
}

template <typename T>
inline T cerp(const T u0, const T u1, const T u2, const T u3, float x)
{
	T u01 = (u0 - u1);

	T p = (u3 - u2) - u01;
	T q = u01 - p;
	T r = u2 - u0;
	return x * (x * (x * p + q) + r) + u1;
}

template <typename T>
inline TVec2D<T> cerp(const TVec2D<T> &u0, const TVec2D<T> &u1, const TVec2D<T> &u2, const TVec2D<T> &u3, float x)
{
	TVec2D<T> u01 = (u0 - u1);

	TVec2D<T> p = (u3 - u2) - u01;
	TVec2D<T> q = u01 - p;
	TVec2D<T> r = u2 - u0;
	return x * (x * (x * p + q) + r) + u1;
}

template <typename T>
inline TVec3D<T> cerp(const TVec3D<T> &u0, const TVec3D<T> &u1, const TVec3D<T> &u2, const TVec3D<T> &u3, float x)
{
	TVec3D<T> u01 = (u0 - u1);

	TVec3D<T> p = (u3 - u2) - u01;
	TVec3D<T> q = u01 - p;
	TVec3D<T> r = u2 - u0;
	return x * (x * (x * p + q) + r) + u1;
}

template <typename T>
inline TVec4D<T> cerp(const TVec4D<T> &u0, const TVec4D<T> &u1, const TVec4D<T> &u2, const TVec4D<T> &u3, float x)
{
	TVec4D<T> u01 = (u0 - u1);

	TVec4D<T> p = (u3 - u2) - u01;
	TVec4D<T> q = u01 - p;
	TVec4D<T> r = u2 - u0;
	return x * (x * (x * p + q) + r) + u1;
}

template <typename T>
inline T sameSign(T a, T b)
{
	return a * b >= 0;
}

template <typename T>
inline T sign(const T v)
{
	return (T)(v > 0) - (v < 0);
}

template <typename T>
inline TVec2D<T> sign(const TVec2D<T> &v)
{
	return TVec2D<T>(sign(v.x), sign(v.y));
}

template <typename T>
inline TVec3D<T> sign(const TVec3D<T> &v)
{
	return TVec3D<T>(sign(v.x), sign(v.y), sign(v.z));
}

template <typename T>
inline TVec4D<T> sign(const TVec4D<T> &v)
{
	return TVec4D<T>(sign(v.x), sign(v.y), sign(v.z), sign(v.w));
}

template <typename T>
T approachValue(T v, T t, T s)
{
	const T newValue = v + s;

	const T diffBefore = v - t;
	const T diffAfter = newValue - t;

	return diffBefore * diffAfter < 0 ? t : newValue;
}

template <typename T, typename T2>
inline T clamp(const T v, const T2 c0, const T2 c1)
{
	return min((T)max((T)v, (T)c0), (T)c1);
}

template <typename T, typename T2>
inline TVec2D<T> clamp(const TVec2D<T> &v, const T2 c0, const T2 c1)
{
	return TVec2D<T>(min(max(v.x, c0), c1), min(max(v.y, c0), c1));
}

template <typename T>
inline TVec2D<T> clamp(const TVec2D<T> &v, const TVec2D<T> &c0, const TVec2D<T> &c1)
{
	return TVec2D<T>(min(max(v.x, c0.x), c1.x), min(max(v.y, c0.y), c1.y));
}

template <typename T, typename T2>
inline TVec3D<T> clamp(const TVec3D<T> &v, const T2 c0, const T2 c1)
{
	return TVec3D<T>(min(max(v.x, c0), c1), min(max(v.y, c0), c1), min(max(v.z, c0), c1));
}

template <typename T>
inline TVec3D<T> clamp(const TVec3D<T> &v, const TVec3D<T> &c0, const TVec3D<T> &c1)
{
	return TVec3D<T>(min(max(v.x, c0.x), c1.x), min(max(v.y, c0.y), c1.y), min(max(v.z, c0.z), c1.z));
}

template <typename T, typename T2>
inline TVec4D<T> clamp(const TVec4D<T> &v, const T2 c0, const T2 c1)
{
	return TVec4D<T>(min(max(v.x, c0), c1), min(max(v.y, c0), c1), min(max(v.z, c0), c1), min(max(v.z, c0), c1));
}

template <typename T>
inline TVec4D<T> clamp(const TVec4D<T> &v, const TVec4D<T> &c0, const TVec4D<T> &c1)
{
	return TVec4D<T>(min(max(v.x, c0.x), c1.x), min(max(v.y, c0.y), c1.y), min(max(v.z, c0.z), c1.z), min(max(v.w, c0.w), c1.w));
}

template <typename T>
inline T normalize(const T v)
{
	T invLen = T(1.0) / sqrt(v * v);
	return v * invLen;
}

template <typename T>
inline TVec2D<T> normalize(const TVec2D<T> &v)
{
	T invLen = T(1.0) / sqrt(v.x * v.x + v.y * v.y);
	return v * invLen;
}

template <typename T>
inline TVec3D<T> normalize(const TVec3D<T> &v)
{
	T invLen = T(1.0) / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return v * invLen;
}

template <typename T>
inline TVec4D<T> normalize(const TVec4D<T> &v)
{
	T invLen = T(1.0) / sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	return v * invLen;
}

template <typename T>
inline TVec2D<T> fastNormalize(const TVec2D<T> &v)
{
	T invLen = rsqrtf(v.x * v.x + v.y * v.y);
	return v * invLen;
}

template <typename T>
inline TVec3D<T> fastNormalize(const TVec3D<T> &v)
{
	T invLen = rsqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return v * invLen;
}

template <typename T>
inline TVec4D<T> fastNormalize(const TVec4D<T> &v)
{
	T invLen = rsqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	return v * invLen;
}

//-----------------------

template <typename T>
inline T length(const TVec2D<T> &v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

template <typename T>
inline T length(const TVec3D<T> &v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <typename T>
inline T length(const TVec4D<T> &v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

// computes squared length of vector2
template <typename T>
inline T lengthSqr(const TVec2D<T> &v)
{
	return dot(v,v);
}

// computes squared length of vector3
template <typename T>
inline T lengthSqr(const TVec3D<T> &v)
{
	return dot(v,v);
}

// computes squared length of vector4
template <typename T>
inline T lengthSqr(const TVec4D<T> &v)
{
	return dot(v,v);
}

//-----------------------

template <typename T>
inline TVec3D<T> reflect(const TVec3D<T> &v, const TVec3D<T> &normal)
{
	T n = dot(v, normal);
	return v - 2 * n * normal;
}

template <typename T>
inline T distance(const TVec2D<T> &u, const TVec2D<T> &v)
{
    TVec2D<T> d = u - v;
	return dot(d, d);
}

template <typename T>
inline T distance(const TVec3D<T> &u, const TVec3D<T> &v)
{
    TVec3D<T> d = u - v;
	return dot(d, d);
}

template <typename T>
inline T distance(const TVec4D<T> &u, const TVec4D<T> &v)
{
    TVec4D<T> d = u - v;
	return dot(d, d);
}

// performs normalized balance vector2
template <typename T>
inline TVec2D<T> balance(const TVec2D<T> &v)
{
	TVec2D<T> tmp(v);

	T invLen = 1.0f / sqrt(v.x*v.x + v.y*v.y);

	tmp *= invLen;

	return TVec2D<T>(fabs(v.x)*tmp.x,
					fabs(v.y)*tmp.y);
}

// performs normalized balance vector3
template <typename T>
inline TVec3D<T> balance(const TVec3D<T> &v)
{
	TVec3D<T> tmp(v);

	T invLen = 1.0f / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

	tmp *= invLen;

	return TVec3D<T>(fabs(v.x)*tmp.x,
					fabs(v.y)*tmp.y,
					fabs(v.z)*tmp.z);
}

// performs normalized balance vector4
template <typename T>
inline TVec4D<T> balance(const TVec4D<T> &v)
{
	TVec4D<T> tmp(v);

	T invLen = 1.0f / sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);

	tmp *= invLen;

	return TVec4D<T>(fabs(v.x)*tmp.x,
					fabs(v.y)*tmp.y,
					fabs(v.z)*tmp.z,
					fabs(v.z)*tmp.w);
}

template <typename T>
inline TVec3D<T> remapRotate(TVec3D<T> &angles)
{
	if(angles.x < 0)
		angles += 360;
	if(angles.y < 0)
		angles += 360;
	if(angles.z < 0)
		angles += 360;

	if(angles.x > 379)
		angles -= 360;
	if(angles.y > 379)
		angles -= 360;
	if(angles.z > 379)
		angles -= 360;

	return TVec3D<T>(angles.y,-angles.z,angles.x);
}


// Vector multiply and add
template <typename T>
inline void VectorMA(const TVec3D<T> &veca, T scale, const TVec3D<T> &vecb, TVec3D<T> &vecc)
{
	vecc = veca + scale*vecb;
}

template <typename T>
inline T planeDistance(const TVec3D<T> &normal, const T offset, const TVec3D<T> &point)
{
    return point.x * normal.x + point.y * normal.y + point.z * normal.z + offset;
}

template <typename T>
inline T planeDistance(const TVec4D<T> &plane, const TVec3D<T> &point)
{
    return point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.w;
}

template <typename T>
inline T sCurve(const T t)
{
	return t * t * (3 - 2 * t);
}

template <typename T>
inline TVec3D<T> cross(const TVec3D<T> &u, const TVec3D<T> &v)
{
	return TVec3D<T>(u.y * v.z - v.y * u.z, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

template <typename T>
inline T lineProjection(const TVec2D<T> &start, const TVec2D<T> &end, const TVec2D<T> &point)
{
	TVec2D<T> v = end - start;
	return dot(v, point - start) / dot(v, v);
}

template <typename T>
inline T lineProjection(const TVec3D<T> &start, const TVec3D<T> &end, const TVec3D<T> &point)
{
	TVec3D<T> v = end - start;
	return dot(v, point - start) / dot(v, v);
}

template <typename T>
inline bool fixNormal(TVec3D<T> &vec)
{
	if ( vec.x == 0.0f )
	{
		if ( vec.y == 0.0f )
		{
			if ( vec.z > 0.0f )
			{
				if ( vec.z != 1.0f )
				{
					vec.z = 1.0f;
					return true;
				}
			}
			else
			{
				if ( vec.z != -1.0f )
				{
					vec.z = -1.0f;
					return true;
				}
			}
			return false;
		}
		else if ( vec.z == 0.0f )
		{
			if ( vec.y > 0.0f )
			{
				if ( vec.y != 1.0f )
				{
					vec.y = 1.0f;
					return true;
				}
			}
			else
			{
				if ( vec.y != -1.0f )
				{
					vec.y = -1.0f;
					return true;
				}
			}
			return false;
		}
	}
	else if ( vec.y == 0.0f )
	{
		if ( vec.z == 0.0f )
		{
			if ( vec.x > 0.0f )
			{
				if ( vec.x != 1.0f )
				{
					vec.x = 1.0f;
					return true;
				}
			}
			else
			{
				if ( vec.x != -1.0f )
				{
					vec.x = -1.0f;
					return true;
				}
			}
			return false;
		}
	}
	if ( fabs( vec.x ) == 1.0f )
	{
		if ( vec.y != 0.0f || vec.z != 0.0f )
		{
			vec.y = vec.z = 0.0f;
			return true;
		}
		return false;
	}
	else if ( fabs( vec.y ) == 1.0f )
	{
		if ( vec.x != 0.0f || vec.z != 0.0f )
		{
			vec.x = vec.z = 0.0f;
			return true;
		}
		return false;
	}
	else if ( fabs( vec.z ) == 1.0f )
	{
		if ( vec.x != 0.0f || vec.y != 0.0f )
		{
			vec.x = vec.y = 0.0f;
			return true;
		}
		return false;
	}

	return false;
}

const float _oneBy255 = 1.0f / 255.0f;

template <typename CHAR_T>
inline ColorRGB hexToColor3(CHAR_T* rgb)
{
	// parse color string
	char hexcolor[6];

	for (int i = 0; i < 6 && *rgb; i++)
		hexcolor[i] = *rgb++;

	// This looks weird
	char* pend;

	char r[3] = { hexcolor[0], hexcolor[1], 0 };
	char g[3] = { hexcolor[2], hexcolor[3], 0 };
	char b[3] = { hexcolor[4], hexcolor[5], 0 };

	return ColorRGB(
		strtol(r, &pend, 16) * _oneBy255,
		strtol(g, &pend, 16) * _oneBy255,
		strtol(b, &pend, 16) * _oneBy255);
}

template <typename CHAR_T>
inline ColorRGBA hexToColor4(CHAR_T* rgba)
{
	// parse color string
	char hexcolor[8];

	for (int i = 0; i < 8 && *rgba; i++)
		hexcolor[i] = *rgba++;

	// This looks weird
	char* pend;

	char r[3] = { hexcolor[0], hexcolor[1], 0 };
	char g[3] = { hexcolor[2], hexcolor[3], 0 };
	char b[3] = { hexcolor[4], hexcolor[5], 0 };
	char a[3] = { hexcolor[6], hexcolor[7], 0 };

	return ColorRGBA(
		strtol(r, &pend, 16) * _oneBy255,
		strtol(g, &pend, 16) * _oneBy255,
		strtol(b, &pend, 16) * _oneBy255,
		strtol(a, &pend, 16) * _oneBy255);
}

#endif // VECTOR_INLINE_H

#ifndef PSX_MATH_TYPES_H
#define PSX_MATH_TYPES_H

#include "math_common.h"

const int	ONE			= 4096;
const float ONE_F		= float(ONE);
const int	ONE_BITS	= 12;

const float TO_RADIAN		= (1.0f / ONE_F * PI_F * 2.0f);
const float TO_GTE_ANGLE	= (1.0f / (PI_F * 2.0f) * ONE_F);

#define FixDivHalfRound(x, bits)		(((x) + (1 << (bits-1))) >> bits)
#define	FIXEDH(a)					FixDivHalfRound(a, ONE_BITS)		// Fixed Half Round number
#define	FIXED(a)					((a) >> ONE_BITS)				// Fixed number (unsigned)

#define DIFF_ANGLES( _ang1, _ang2 ) \
	(((((_ang2) - (_ang1)) + 2048) & 4095) - 2048)

#define DIFF_ANGLES_F( _ang1, _ang2 ) \
	(fmodf((((_ang2) - (_ang1)) + 2048.0f), 4096.0f) - 2048.0f)

template <typename T>
struct TPSX_VECTOR
{
	T vx, vy, vz;

	TPSX_VECTOR<T>() {}

	template <class T2>
	TPSX_VECTOR(const TPSX_VECTOR<T2>& other)
		: vx((T)other.vx), vy((T)other.vy), vz((T)other.vz)
	{}

	TPSX_VECTOR<T>(const T ixyz)
		: vx(ixyz), vy(ixyz), vz(ixyz)
	{}

	TPSX_VECTOR<T>(const T ix, const T iy, const T iz)
		: vx(ix), vy(iy), vz(iz)
	{}

	operator T* () const { return (T*)&vx; }

	template <class T2>
	void operator += (const T2 s);

	template <class T2>
	void operator += (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator -= (const T2 s);

	template <class T2>
	void operator -= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator *= (const T2 s);

	template <class T2>
	void operator *= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator /= (const T2 s);

	template <class T2>
	void operator /= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator >>= (const T2 s);

	template <class T2>
	void operator >>= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator <<= (const T2 s);

	template <class T2>
	void operator <<= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator &= (const T2 s);

	template <class T2>
	void operator &= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator |= (const T2 s);

	template <class T2>
	void operator |= (const TPSX_VECTOR<T2>& v);

	template <class T2>
	void operator ^= (const T2 s);

	template <class T2>
	void operator ^= (const TPSX_VECTOR<T2>& v);
};


template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator += (const T2 s)
{
	vx += s;
	vy += s;
	vz += s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator += (const TPSX_VECTOR<T2>& v)
{
	vx += v.vx;
	vy += v.vy;
	vz += v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator -= (const T2 s)
{
	vx -= s;
	vy -= s;
	vz -= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator -= (const TPSX_VECTOR<T2>& v)
{
	vx -= v.vx;
	vy -= v.vy;
	vz -= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator *= (const T2 s)
{
	vx *= s;
	vy *= s;
	vz *= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator *= (const TPSX_VECTOR<T2>& v)
{
	vx *= v.vx;
	vy *= v.vy;
	vz *= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator /= (const T2 s)
{
	vx /= s;
	vy /= s;
	vz /= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator /= (const TPSX_VECTOR<T2>& v)
{
	vx /= v.vx;
	vy /= v.vy;
	vz /= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator >>= (const T2 s)
{
	vx >>= s;
	vy >>= s;
	vz >>= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator >>= (const TPSX_VECTOR<T2>& v)
{
	vx >>= v.vx;
	vy >>= v.vy;
	vz >>= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator <<= (const T2 s)
{
	vx <<= s;
	vy <<= s;
	vz <<= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator <<= (const TPSX_VECTOR<T2>& v)
{
	vx <<= v.vx;
	vy <<= v.vy;
	vz <<= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator &= (const T2 s)
{
	vx &= s;
	vy &= s;
	vz &= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator &= (const TPSX_VECTOR<T2>& v)
{
	vx &= v.vx;
	vy &= v.vy;
	vz &= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator |= (const T2 s)
{
	vx |= s;
	vy |= s;
	vz |= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator |= (const TPSX_VECTOR<T2>& v)
{
	vx |= v.vx;
	vy |= v.vy;
	vz |= v.vz;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator ^= (const T2 s)
{
	vx ^= s;
	vy ^= s;
	vz ^= s;
}

template <class T>
template <class T2>
inline void TPSX_VECTOR<T>::operator ^= (const TPSX_VECTOR<T2>& v)
{
	vx ^= v.vx;
	vy ^= v.vy;
	vz ^= v.vz;
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator + (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx + v.vx, u.vy + v.vy, u.vz + v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator + (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx + s, v.vy + s, v.vz + s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator + (const T s, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(v.vx + s, v.vy + s, v.vz + s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator - (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx - v.vx, u.vy - v.vy, u.vz - v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator - (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx - s, v.vy - s, v.vz - s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator - (const T s, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(v.vx - s, v.vy - s, v.vz - s);
}

template <typename T>
inline TPSX_VECTOR<T> operator - (const TPSX_VECTOR<T>& v)
{
	return TPSX_VECTOR<T>(-v.vx, -v.vy, -v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator * (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx * v.vx, u.vy * v.vy, u.vz * v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator * (const T s, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(v.vx * s, v.vy * s, v.vz * s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator * (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx * s, v.vy * s, v.vz * s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator / (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx / v.vx, u.vy / v.vy, u.vz / v.vz);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator / (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx / s, v.vy / s, v.vz / s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator >> (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx >> v.vx, u.vy >> v.vy, u.vz >> v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator << (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx << v.vx, u.vy << v.vy, u.vz << v.vz);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator >> (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx >> s, v.vy >> s, v.vz >> s);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator << (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx << s, v.vy << s, v.vz << s);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator & (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx & s, v.vy & s, v.vz & s);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator | (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx | s, v.vy | s, v.vz | s);
}

template <typename T, typename T2>
TPSX_VECTOR<T> operator ^ (const TPSX_VECTOR<T>& v, const T2 s)
{
	return TPSX_VECTOR<T>(v.vx ^ s, v.vy ^ s, v.vz ^ s);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator ^ (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(u.vx ^ v.vx, u.vy ^ v.vy, u.vz ^ v.vz);
}

template <typename T, typename T2>
inline TPSX_VECTOR<T> operator / (const T s, const TPSX_VECTOR<T2>& v)
{
	return TPSX_VECTOR<T>(s / v.vx, s / v.vy, s / v.vz);
}

template <typename T, typename T2>
inline bool operator == (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return (u.vx == v.vx && u.vy == v.vy && u.vz == v.vz);
}

template <typename T, typename T2>
inline T operator & (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return u.vx * v.vx + u.vy * v.vy + u.vz * v.vz;
}

template <typename T, typename T2>
inline bool operator <= (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return u.vx <= v.vx && u.vy <= v.vy && u.vz <= v.vz;
}

template <typename T, typename T2>
inline bool operator >= (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return u.vx >= v.vx && u.vy >= v.vy && u.vz >= v.vz;
}

template <typename T, typename T2>
inline bool operator < (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return u.vx < v.vx && u.vy < v.vy && u.vz < v.vz;
}

template <typename T, typename T2>
inline bool operator > (const TPSX_VECTOR<T>& u, const TPSX_VECTOR<T2>& v)
{
	return u.vx > v.vx && u.vy > v.vy && u.vz > v.vz;
}

typedef TPSX_VECTOR<int> VECTOR_NOPAD;
typedef TPSX_VECTOR<short> SVECTOR_NOPAD;
typedef TPSX_VECTOR<uint16> USVECTOR_NOPAD;

struct SVECTOR
{
	short vx, vy, vz, pad;

	SVECTOR_NOPAD& p() const { return *(SVECTOR_NOPAD*)&vx; }
	operator short* () const { return (short*)&vx; }
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
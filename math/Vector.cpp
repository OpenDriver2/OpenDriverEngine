#include "Vector.h"
#include "dktypes.h"

half::half(const float x)
{
	union 
	{
		float floatI;
		unsigned int i;
	};

	floatI = x;

//	unsigned int i = *((unsigned int *) &x);
	int e = ((i >> 23) & 0xFF) - 112;
	int m =  i & 0x007FFFFF;

	sh = (i >> 16) & 0x8000;
	if (e <= 0)
	{
		// Denorm
		m = ((m | 0x00800000) >> (1 - e)) + 0x1000;
		sh |= (m >> 13);
	} 
	else if (e == 143)
	{
		sh |= 0x7C00;
		if (m != 0)
		{
			// NAN
			m >>= 13;
			sh |= m | (m == 0);
		}
	} 
	else 
	{
		m += 0x1000;
		if (m & 0x00800000)
		{
			// Mantissa overflow
			m = 0;
			e++;
		}
		if (e >= 31)
		{
			// Exponent overflow
			sh |= 0x7C00;
		}
		else 
		{
			sh |= (e << 10) | (m >> 13);
		}
	}
}

half::operator float () const 
{
	union 
	{
		unsigned int s;
		float result;
	};

	s = (sh & 0x8000) << 16;
	unsigned int e = (sh >> 10) & 0x1F;
	unsigned int m = sh & 0x03FF;

	if (e == 0)
	{
		// +/- 0
		if (m == 0) return result;

		// Denorm
		while ((m & 0x0400) == 0)
		{
			m += m;
			e--;
		}
		e++;
		m &= ~0x0400;
	} 
	else if (e == 31)
	{
		// INF / NAN
		s |= 0x7F800000 | (m << 13);
		return result;
	}

	s |= ((e + 112) << 23) | (m << 13);

	return result;
}

/* --------------------------------------------------------------------------------- */

//Helper method to emulate GLSL
float fract(float value)
{
    return (float)fmod(value, 1.0f);
}

void AngleVectors(const Vector3D &angles, Vector3D *forward, Vector3D *right, Vector3D *up)
{
	float cp,cy,cr,sp,sy,sr;

	SinCos(DEG2RAD(-angles.x),&sp,&cp);
	SinCos(DEG2RAD(-angles.y),&sy,&cy);
	SinCos(DEG2RAD(-angles.z),&sr,&cr);

	if(forward)
	{
		forward->x = cp*sy;
		forward->y = sp;
		forward->z = cp*cy;
	}

	if (up)
	{
		up->x = cy*sr-sp*sy*cr;
		up->y = cp*cr;
		up->z = -sy*sr-sp*cy*cr;
	}

	if (right)
	{
		right->x = cy * cr + sp * sy * sr;
		right->y = -cp * sr;
		right->z = sp * cy * sr - sy * cr;
	}
}

Vector3D VectorAngles(const Vector3D &forward)
{
	Vector3D angle;

	angle.y = RAD2DEG(atan2f(forward.x, forward.z)*-1);

	if (angle.y < 0.0f)
		angle.y += 360.0f;

	if (angle.y >= 360.0f)
		angle.y -= 360.0f;

	float z1 = sqrtf(forward.x*forward.x + forward.z*forward.z);

	angle.x = RAD2DEG(atan2f(z1, forward.y)) - 90.0f;

	if (angle.x < 0.0f)
		angle.x += 360.0f;

	if (angle.x >= 360.0f)
		angle.x -= 360.0f;

	angle.z = 0.0f;

	return angle;
}

void VectorVectors( const Vector3D &forward, Vector3D &right, Vector3D &up )
{
	Vector3D tmp;

	if (forward[0] == 0 && forward[1] == 0)
	{
		// pitch 90 degrees up/down from identity
		right[0]	= -forward[2];	
		right[1]	= 0; 
		right[2]	= 0;

		up[0]		= 0; 
		up[1]		= 1; 
		up[2]		= 0;
	}
	else
	{
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 1.0;

		right = cross( forward, tmp );
		right = normalize( right );

		up = cross( right, forward );
		up = normalize( up );
	}
}

void VectorRotate( const Vector3D &in1, const Vector3D &angles, Vector3D *out, Vector3D *center)
{
	Vector3D psvdCenter = Vector3D(0);

	if(center != NULL)
	{
		psvdCenter = *center;
	}

	Vector3D	temp = RotateYZBy(in1,-angles.x,psvdCenter);
				temp = RotateXZBy(temp,-angles.y,psvdCenter);
				temp = RotateXYBy(temp,-angles.z,psvdCenter);

	out->y = temp.y;
	out->z = temp.z;
	out->x = temp.x;
}

Vector3D RotateXZBy(const Vector3D &vector,float degrees, Vector3D &center)
{
	degrees *= (float)(PI_F / 180.0f);
	float cs,sn;
	SinCos(degrees,&sn,&cs);
	Vector3D out = vector;
	out.x -= center.x;
	out.z -= center.z;
	out = Vector3D(vector.x * cs - vector.z * sn, vector.y, vector.x * sn + vector.z * cs);
	out.x += center.x;
	out.z += center.z;
	return out;
}

Vector3D RotateXYBy(const Vector3D &vector,float degrees, Vector3D &center)
{
	degrees *= (float)(PI_F / 180.0f);
	float cs,sn;
	SinCos(degrees,&sn,&cs);
	Vector3D out = vector;
	out.x -= center.x;
	out.y -= center.y;
	out = Vector3D(vector.x * cs - vector.y * sn, vector.x * sn + vector.y * cs, vector.z);
	out.x += center.x;
	out.y += center.x;
	return out;
}

Vector3D RotateYZBy(const Vector3D &vector,float degrees, Vector3D &center)
{
    degrees *= (float)(PI_F / 180.0f);
	float cs,sn;
	SinCos(degrees,&sn,&cs);
	Vector3D out = vector;
    out.z -= center.z;
    out.y -= center.y;
    out = Vector3D(vector.x, vector.y * cs - vector.z * sn, vector.y * sn + vector.z * cs);
    out.z += center.z;
    out.y += center.y;
	return out;
}

unsigned int toRGBA(const ColorRGBA &u)
{
	return (int(u.x * 255) | (int(u.y * 255) << 8) | (int(u.z * 255) << 16) | (int(u.w * 255) << 24));
}

unsigned int toBGRA(const ColorRGBA &u)
{
	return (int(u.z * 255) | (int(u.y * 255) << 8) | (int(u.x * 255) << 16) | (int(u.w * 255) << 24));
}

ColorRGB rgbeToRGB(unsigned char *rgbe)
{
	if (rgbe[3])
	{
		return ColorRGB(rgbe[0], rgbe[1], rgbe[2]) * ldexpf(1.0f, rgbe[3] - (int) (128 + 8));
	} 
	else
		return ColorRGB(0, 0, 0);
}

unsigned int rgbToRGBE8(const ColorRGB &rgb)
{
	float v = MAX(rgb.x, rgb.y);
	v = MAX(v, rgb.z);

	if (v < 1e-32f)
	{
		return 0;
	} 
	else 
	{
		int ex;
		float m = frexpf(v, &ex) * 256.0f / v;

		unsigned int r = (unsigned int) (m * rgb.x);
		unsigned int g = (unsigned int) (m * rgb.y);
		unsigned int b = (unsigned int) (m * rgb.z);
		unsigned int e = (unsigned int) (ex + 128);

		return r | (g << 8) | (b << 16) | (e << 24);
	}
}

unsigned int rgbToRGB9E5(const ColorRGB &rgb)
{
	float v = MAX(rgb.x, rgb.y);
	v = MAX(v, rgb.z);

	if (v < 1.52587890625e-5f)
	{
		return 0;
	} 
	else if (v < 65536)
	{
		int ex;
		float m = frexpf(v, &ex) * 512.0f / v;

		unsigned int r = (unsigned int) (m * rgb.x);
		unsigned int g = (unsigned int) (m * rgb.y);
		unsigned int b = (unsigned int) (m * rgb.z);
		unsigned int e = (unsigned int) (ex + 15);

		return r | (g << 9) | (b << 18) | (e << 27);
	} 
	else
	{
		unsigned int r = (rgb.x < 65536)? (unsigned int) (rgb.x * (1.0f / 128.0f)) : 0x1FF;
		unsigned int g = (rgb.y < 65536)? (unsigned int) (rgb.y * (1.0f / 128.0f)) : 0x1FF;
		unsigned int b = (rgb.z < 65536)? (unsigned int) (rgb.z * (1.0f / 128.0f)) : 0x1FF;
		unsigned int e = 31;

		return r | (g << 9) | (b << 18) | (e << 27);
	}
}
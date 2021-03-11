#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

template <class T>
struct TMat2 // Matrix2x2
{
	TVec2D<T> rows[2];

	TMat2<T>(){}

	template <class T2>
	TMat2<T>(const TMat2<T2>& other) : rows(other)
	{}

	TMat2<T>(const TVec2D<T>& row0, const TVec2D<T>& row1) 
		: rows{ row0, row1 }
	{}

	TMat2<T>(	const T m00, const T m01,
				const T m10, const T m11) 
		: rows{ {m00, m01 }, { m10, m11 }}
	{}

	operator const T *() const { return (const T *) rows; }
};


template <typename T>
TMat2<T> operator + (const TMat2<T> &m, const TMat2<T> &n);

template <typename T>
TMat2<T> operator - (const TMat2<T> &m, const TMat2<T> &n);

template <typename T>
TMat2<T> operator - (const TMat2<T> &m);

template <typename T>
TMat2<T> operator * (const TMat2<T> &m, const TMat2<T> &n);

template <typename T>
TVec2D<T> operator * (const TMat2<T> &m, const TVec2D<T> &v);

template <typename T>
TMat2<T> operator * (const TMat2<T> &m, const T x);

template <typename T>
TMat2<T> operator ! (const TMat2<T> &m);


/* --------------------------------------------------------------------------------- */

template <class T>
struct TMat3 // Matrix3x3
{
	TVec3D<T> rows[3];

	TMat3<T>() {}

	template <class T2>
	TMat3<T>(const TMat3<T2>& other) : rows(other)
	{}

	TMat3<T>(const TVec3D<T> &row0, const TVec3D<T> &row1, const TVec3D<T> &row2) :
		rows{ row0, row1,row2 }
	{}

	TMat3<T>(	const T m00, const T m01, const T m02,
				const T m10, const T m11, const T m12,
				const T m20, const T m21, const T m22)
		: rows{ {m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22} }
	{}

	T* toRaw() {return (T *) rows;}

	T& operator () (int row, int col) const { return rows[row][col]; }

	operator const T *() const { return (const T *) rows; }
};

template <typename T>
TMat3<T> operator + (const TMat3<T> &m, const TMat3<T> &n);

template <typename T>
TMat3<T> operator - (const TMat3<T> &m, const TMat3<T> &n);

template <typename T>
TMat3<T> operator - (const TMat3<T> &m);

template <typename T>
TMat3<T> operator * (const TMat3<T> &m, const TMat3<T> &n);

template <typename T>
TVec3D<T> operator * (const TMat3<T> &m, const TVec3D<T> &v);

template <typename T>
TMat3<T> operator * (const TMat3<T> &m, const T x);

template <typename T>
TMat3<T> operator ! (const TMat3<T> &m);

/* --------------------------------------------------------------------------------- */

template <class T>
struct TMat4 // Matrix4x4
{
	TVec4D<T> rows[4];

	TMat4<T>(){}

	template <class T2>
	TMat4<T>(const TMat4<T2>& other) : rows(other)
	{}

	TMat4<T>(const TVec4D<T> &row0, 
			 const TVec4D<T> &row1, 
			 const TVec4D<T> &row2, 
			 const TVec4D<T> &row3)
		: rows {row0, row1, row2, row3}
	{
	}

	TMat4<T>(	const T m00, const T m01, const T m02, const T m03,
				const T m10, const T m11, const T m12, const T m13,
				const T m20, const T m21, const T m22, const T m23,
				const T m30, const T m31, const T m32, const T m33)
		: rows{ {m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23}, {m30, m31, m32, m33} }
	{
	}

	TMat4<T>(const TVec3D<T> &axis, const float angle)
	{
		float sinA,cosA;
		SinCos(angle, &sinA, &cosA);

		rows[0] = TVec4D<T>(axis.x * axis.x * (1 - cosA) + cosA,
							axis.x * axis.y * (cosA - 1) + axis.z * sinA,
							axis.x * axis.z * (1 - cosA) + axis.y * sinA,
							0);

		rows[1] = TVec4D<T>(axis.x * axis.y * (cosA - 1) - axis.z * sinA,
							axis.y * axis.y * (1 - cosA) + cosA,
							axis.y * axis.z * (cosA - 1) + axis.x * sinA,
							0);
		rows[2] = TVec4D<T>(axis.x * axis.z * (1 - cosA) - axis.y * sinA,
							axis.y * axis.z * (cosA - 1) - axis.x * sinA,
							axis.z * axis.z * (1 - cosA) + cosA,
							0);

		rows[3] = TVec4D<T>(0, 0, 0, 1);
	}

	template <class T2>
	TMat4<T>(const TMat3<T2> &m)
	{
		rows[0] = TVec4D<T>(m.rows[0], 0);
		rows[1] = TVec4D<T>(m.rows[1], 0);
		rows[2] = TVec4D<T>(m.rows[2], 0);
		rows[3] = TVec4D<T>(0, 0, 0, 1);
	}

	T *toRaw() {return (T *) rows;}

	operator const T *() const { return (const T *) rows; }

	T& operator () (int row, int col) const { return rows[row][col]; }

	void				setRotation(const TVec3D<T> &v);
	void				setTranslation(const TVec3D<T> &v);
	void				translate(const TVec3D<T> &v);

	const TVec3D<T>&	getTranslationComponent() const;
	TMat3<T>			getRotationComponent() const;

	const TVec3D<T>		getTranslationComponentTransposed() const;
	TMat3<T>			getRotationComponentTransposed() const;
};

template <typename T>
TMat4<T> operator + (const TMat4<T> &m, const TMat4<T> &n);

template <typename T>
TMat4<T> operator - (const TMat4<T> &m, const TMat4<T> &n);

template <typename T>
TMat4<T> operator - (const TMat4<T> &m);

template <typename T>
TMat4<T> operator * (const TMat4<T> &m, const TMat4<T> &n);

template <typename T>
TVec4D<T> operator * (const TMat4<T> &m, const TVec4D<T> &v);

template <typename T>
TVec3D<T> operator * (const TMat4<T> &m, const TVec3D<T> &v);

template <typename T>
TMat4<T> operator * (const TMat4<T> &m, const T x);

template <typename T>
TMat4<T> operator ! (const TMat4<T> &m);

// Define common matrix types:

typedef TMat2<float> Matrix2x2;
typedef TMat3<float> Matrix3x3;
typedef TMat4<float> Matrix4x4;

typedef TMat2<int> IMatrix2x2;
typedef TMat3<int> IMatrix3x3;
typedef TMat4<int> IMatrix4x4;

/* --------------------------------------------------------------------------------- */

// finds matrix2 determinant
template <typename T>
T det(const TMat2<T> &m);

// finds matrix3 determinant
template <typename T>
T det(const TMat3<T> &m);

// does swap (rows-columns) of matrix4
template <typename T>
TMat2<T> transpose(const TMat2<T> &m);

// does swap (rows-columns) of matrix3
template <typename T>
TMat3<T> transpose(const TMat3<T> &m);

// does swap (rows-columns) of matrix4
template <typename T>
TMat4<T> transpose(const TMat4<T> &m);

// matrix shear - undone
template <typename T>
TMat4<T> shearX(const T shear);

template <typename T>
TMat4<T> shearY(const T shear);

template <typename T>
TMat4<T> shearZ(const T shear);

//----------------------------------------------------------------------------------

// matrix rotation
template <typename T>
TMat2<T> rotate2(const T angle);

//----------------------------------------------------------------------------------

// matrix3 x rotation
template <typename T>
TMat3<T> rotateX3(const T angle);

// matrix3 y rotation
template <typename T>
TMat3<T> rotateY3(const T angle);

// matrix3 z rotation
template <typename T>
TMat3<T> rotateZ3(const T angle);

// matrix3 x-y rotation
template <typename T>
TMat3<T> rotateXY3(const T angleX, const T angleY);

// matrix3 y-x rotation
template <typename T>
TMat3<T> rotateYX3(const T angleX, const T angleY);

// matrix3 x-y-z rotation
template <typename T>
TMat3<T> rotateXYZ3(const T angleX, const T angleY, const T angleZ);

// matrix3 z-x-y rotation
template <typename T>
TMat3<T> rotateZXY3(const T angleX, const T angleY, const T angleZ);

// matrix3 y-z-x rotation
template <typename T>
TMat3<T> rotateYZX3(const T angleX, const T angleY, const T angleZ);

// maxtrix3 axis angle rotation
template <typename T>
TMat3<T> rotateAxis3(const TVec3D<T>& axis, T angle);

//----------------------------------------------------------------------------------

// matrix4 x rotation
template <typename T>
TMat4<T> rotateX4(const T angle);

// matrix4 y rotation
template <typename T>
TMat4<T> rotateY4(const T angle);

// matrix4 z rotation
template <typename T>
TMat4<T> rotateZ4(const T angle);

// matrix4 x-y rotation
template <typename T>
TMat4<T> rotateXY4(const T angleX, const T angleY);

// matrix4 y-x rotation
template <typename T>
TMat4<T> rotateYX4(const T angleX, const T angleY);

// matrix4 x-y-z rotation
template <typename T>
TMat4<T> rotateXYZ4(const T angleX, const T angleY, const T angleZ);

// matrix4 z-x-y rotation
template <typename T>
TMat4<T> rotateZXY4(const T angleX, const T angleY, const T angleZ);

// matrix4 y-z-x rotation
template <typename T>
TMat4<T> rotateYZX4(const T angleX, const T angleY, const T angleZ);

//----------------------------------------------------------------------------------

// matrix4 vector translation
template <typename T>
TMat4<T> translate(const TVec3D<T> &v);

// matrix4 xyz translation
template <typename T>
TMat4<T> translate(const T x, const T y, const T z);

// matrix2 uv scaling
template <typename T>
TMat2<T> scale2(const T u, const T v);

// matrix3 xyz scaling
template <typename T>
TMat3<T> scale3(const T x, const T y, const T z);

// matrix4 xyz scaling
template <typename T>
TMat4<T> scale4(const T x, const T y, const T z);

// makes identity matrix2
template <typename T>
TMat2<T> _identity2();

// makes identity matrix3
template <typename T>
TMat3<T> _identity3();

// makes identity matrix4
template <typename T>
TMat4<T> _identity4();

// transforms point by matrix3
template <typename T>
TVec3D<T> transform3(const TVec3D<T> &in, const TMat3<T>& mat);

// transforms point by matrix4
template <typename T>
TVec3D<T> transform3(const TVec3D<T> &in, const TMat4<T>& mat);

// transforms point by matrix4 using all matrix features
template <typename T>
TVec3D<T> transform4(const TVec3D<T> &in, const TMat4<T>& mat);

// inverse rotation of vector
template <typename T>
TVec3D<T> inverseRotateVec(const TVec3D<T> &vector, const TMat4<T> &matrix);

// inverse translation of vector
template <typename T>
TVec3D<T> inverseTranslateVec(const TVec3D<T> &vector, const TMat4<T> &matrix);

// makes matrix inverse
template <typename T>
TMat4<T> inverse(const TMat4<T> &m);

// generates perspective projection matrix
Matrix4x4 perspectiveMatrix(const float fov, const float zNear, const float zFar);

// generates perspective projection matrix with view width/height x aspect
Matrix4x4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar);

// generates perspective projection matrix with view width/height y aspect
Matrix4x4 perspectiveMatrixY(const float fov, const int width, const int height, const float zNear, const float zFar);

// generates right-handed orthographic projection matrix correct for rendering
Matrix4x4 orthoMatrixR(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar);

// generates right-handed orthographic projection matrix correct for computations
Matrix4x4 orthoMatrix(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar);

// generates right-handed, screen-oriented 2D matrix
Matrix4x4 projection2DScreen(float wide, float tall);

// converts projection to Direct3D (FFP-Only)
Matrix4x4 toD3DProjection(const Matrix4x4 &m);

// converts projection to OpenGL (FFP-Only)
Matrix4x4 toGLProjection(const Matrix4x4 &m);

// generates cubic modelview matrix
Matrix4x4 cubeViewMatrix(const unsigned int side);

// generates cubeprojection matrix for OpenGL
Matrix4x4 cubeProjectionMatrixGL(const float zNear, const float zFar);

// generates cubeprojection matrix for Direct3D
Matrix4x4 cubeProjectionMatrixD3D(const float zNear, const float zFar);

// converts matrix3 to euler angles of X-Y-Z order
Vector3D EulerMatrixXYZ(const Matrix3x3 &matrix);

// converts matrix3 to euler angles of Z-X-Y order
Vector3D EulerMatrixZXY(const Matrix3x3 &matrix);

// Like D3D
typedef TMat2<float> float2x2;
typedef TMat3<float> float3x3;
typedef TMat4<float> float4x4;

// template specialization
#define identity2() _identity2<float>()
#define identity3() _identity3<float>()
#define identity4() _identity4<float>()

// include
#include "Matrix.inl"

#endif // MATRIX_H

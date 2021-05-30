//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Matrix (m2x2,m3x3,m4x4)
//////////////////////////////////////////////////////////////////////////////////

#include "Matrix.h"

/* --------------------------------------------------------------------------------- */



Matrix4x4 perspectiveMatrix(const float fov, const float zNear, const float zFar)
{
	float sinF, cosF;
	SinCos(0.5f * fov,&sinF,&cosF);

	float s = cosF / sinF;

	return Matrix4x4(
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0, 0, 1, 0);
}

Matrix4x4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar)
{
	float sinF, cosF;
	SinCos(0.5f * fov,&sinF,&cosF);

	float w = cosF / sinF;
	float h = (w * width) / height;

	return Matrix4x4(
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0, 0, 1, 0);
}

Matrix4x4 perspectiveMatrixY(const float fov, const int width, const int height, const float zNear, const float zFar)
{
	float sinF, cosF;
	SinCos(0.5f * fov,&sinF,&cosF);

	float h = cosF / sinF;
	float w = (h * height) / width;

	return Matrix4x4(
		w, 0, 0, 0,
		0, h, 0, 0,
		0, 0, (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0, 0, 1, 0);
}

Matrix4x4 orthoMatrixR(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar)
{
	Matrix4x4 mat = _identity4<float>();

	mat.rows[0][0] = 2.0f / (right - left);
	mat.rows[1][1] = 2.0f / (bottom - top);
	mat.rows[2][2] = 1.0f / (zFar - zNear);

	mat.rows[0][3] = (left + right) / (left - right);
	mat.rows[1][3] = (top + bottom) / (bottom - top);
	mat.rows[2][3] = zNear / (zNear - zFar);

	return mat;
}

Matrix4x4 orthoMatrix(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar)
{
	Matrix4x4 mat = _identity4<float>();

	mat.rows[0][0] = 2.0f / (right - left);
	mat.rows[1][1] = 2.0f / (bottom - top);
	mat.rows[2][2] = 2.0f / (zFar - zNear);

	mat.rows[0][3] = (right + left) / (left - right);
	mat.rows[1][3] = (top + bottom) / (bottom - top);
	mat.rows[2][3] = (zFar + zNear) / (zFar - zNear);

	return mat;
}

Matrix4x4 projection2DScreen(float wide, float tall)
{
	// create valid screen matrix
	Matrix4x4 ortho_transform = _identity4<float>();

	ortho_transform.rows[0].x = (2.0f / wide);
	ortho_transform.rows[1].y = -(2.0f / tall);

	ortho_transform.translate(-Vector3D(wide/2, tall/2,0));

	return ortho_transform;
}

Matrix4x4 toD3DProjection(const Matrix4x4 &m)
{
	Matrix4x4 mat;

	mat.rows[0] = m.rows[0];
	mat.rows[1] = m.rows[1];
	mat.rows[2] = 0.5f * (m.rows[2] + m.rows[3]);
	mat.rows[3] = m.rows[3];

	return mat;
}

Matrix4x4 toGLProjection(const Matrix4x4 &m)
{
	Matrix4x4 mat;

	mat.rows[0] = m.rows[0];
	mat.rows[1] = m.rows[1];
	mat.rows[2] = m.rows[2] * 2.0f - m.rows[3];
	mat.rows[3] = m.rows[3];

	return mat;
}

Matrix4x4 cubeViewMatrix(const unsigned int side)
{
	switch(side)
	{
	case POSITIVE_X:
		return Matrix4x4(
			0, 0, -1, 0,
			0, 1,  0, 0,
			1, 0,  0, 0,
			0, 0,  0, 1);
	case NEGATIVE_X:
		return Matrix4x4(
			 0, 0, 1, 0,
			 0, 1, 0, 0,
			-1, 0, 0, 0,
			 0, 0, 0, 1);
	case POSITIVE_Y:
		return Matrix4x4(
			1, 0,  0, 0,
			0, 0, -1, 0,
			0, 1,  0, 0,
			0, 0,  0, 1);
	case NEGATIVE_Y:
		return Matrix4x4(
			1,  0, 0, 0,
			0,  0, 1, 0,
			0, -1, 0, 0,
			0,  0, 0, 1);
	case POSITIVE_Z:
		return Matrix4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	//case NEGATIVE_Z:
	default:
		return Matrix4x4(
			-1, 0,  0, 0,
			 0, 1,  0, 0,
			 0, 0, -1, 0,
			 0, 0,  0, 1);
	}
}

Matrix4x4 cubeProjectionMatrixGL(const float zNear, const float zFar)
{
	return Matrix4x4(
		1,  0, 0, 0,
		0, -1, 0, 0,
		0,  0, (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0,  0, 1, 0);
}

Matrix4x4 cubeProjectionMatrixD3D(const float zNear, const float zFar)
{
	return Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, zFar / (zFar - zNear), (zFar * zNear) / (zNear - zFar),
		0, 0, 1, 0);
}

// converts matrix to euler angles of X-Y-Z order
Vector3D EulerMatrixXYZ(const Matrix3x3 &matrix)
{
	double angle_x,angle_y,angle_z;
	double ftrx,ftry;

	angle_y = -asin( (double)matrix.rows[0][2]);
	double C =  cos( (double)angle_y );

	if ( C*8192.0f > FLT_EPSILON )
	{
		ftrx      =  matrix.rows[2][2] / C;
		ftry      = -matrix.rows[1][2]  / C;

		angle_x  = atan2( ftry, ftrx );

		ftrx      =  matrix.rows[0][0] / C;
		ftry      = -matrix.rows[0][1] / C;

		angle_z  = atan2( ftry, ftrx );
	}
	else
	{
		angle_x  = 0;

		ftrx      = matrix.rows[1][1];
		ftry      = matrix.rows[1][0];

		angle_z  = atan2( ftry, ftrx );
	}

	return Vector3D(-angle_x,angle_y,-angle_z);
}

// converts matrix to euler angles of Z-X-Y order
Vector3D EulerMatrixZXY(const Matrix3x3 &matrix)
{
	float a = asinf(matrix.rows[1][2]);
	float ca = cosf(a);

	if (fabs(ca) > 0.0005f) // handle gimbal lock
	{
		return Vector3D(
			a,
			atan2f(-matrix.rows[0][2] / ca, matrix.rows[2][2] / ca),
			atan2f(-matrix.rows[1][0] / ca, matrix.rows[1][1] / ca)
		);
	}
	else // gimbal lock has occurred
	{
		return Vector3D(
			a,
			atan2f(matrix.rows[2][0], matrix.rows[0][0]),
			0
		);
	}
}

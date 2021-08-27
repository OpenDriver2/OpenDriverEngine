#ifndef CAMERA_H
#define CAMERA_H

#include "math/Vector.h"
#include <sol/forward.hpp>

//---------------------------------------------------------

class Volume;

struct CameraViewParams
{
	Matrix3x3 GetVectors()
	{
		return rotateZXY3(-DEG2RAD(angles.x), -DEG2RAD(angles.y), -DEG2RAD(angles.z));
	}

	Vector3D GetForward() const
	{
		// calculate view matrices
		Matrix3x3 view;
		view = rotateZXY3(-DEG2RAD(angles.x), -DEG2RAD(angles.y), -DEG2RAD(angles.z));
		return view.rows[2];
	}

	Vector3D GetRight() const
	{
		// calculate view matrices
		Matrix3x3 view;
		view = rotateZXY3(-DEG2RAD(angles.x), -DEG2RAD(angles.y), -DEG2RAD(angles.z));
		return view.rows[0];
	}

	Vector3D GetUp() const
	{
		// calculate view matrices
		Matrix3x3 view;
		view = rotateZXY3(-DEG2RAD(angles.x), -DEG2RAD(angles.y), -DEG2RAD(angles.z));
		return view.rows[0];
	}

	Vector3D	position{ 0 };
	Vector3D	angles{ 0 };
	float		fov{ 75.0f };
};

//---------------------------------------------------------

class CCamera
{
public:
	static CameraViewParams		MainView;
	static Vector3D				MainViewVelocity;

	static void SetupViewAndMatrices(const CameraViewParams& cameraParams, Volume& outFrustum);

	static void Lua_Init(sol::state& lua);
};

#endif // CAMERA_H
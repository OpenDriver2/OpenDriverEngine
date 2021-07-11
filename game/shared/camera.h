#ifndef CAMERA_H
#define CAMERA_H

#include "math/Vector.h"
#include <sol/forward.hpp>

//---------------------------------------------------------

class Volume;

struct CameraViewParams
{
	Vector3D	position{ 0 };
	Vector3D	angles{ 0 };
	float		fov{ 75.0f };
};

//---------------------------------------------------------

class CCamera
{
public:
	static CameraViewParams		MainView;

	static void SetupViewAndMatrices(const CameraViewParams& cameraParams, Volume& outFrustum);

	static void Lua_Init(sol::state& lua);
};

#endif // CAMERA_H
#ifndef CAMERA_H
#define CAMERA_H

#include "math/Vector.h"
#include <sol/forward.hpp>

// free camera
extern Vector3D g_cameraVelocity;
extern Vector3D g_cameraPosition;
extern Vector3D g_cameraAngles;

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

// free camera
void UpdateCameraMovement(float deltaTime, float speedModifier, const Vector3D& moveDir);

#endif // CAMERA_H
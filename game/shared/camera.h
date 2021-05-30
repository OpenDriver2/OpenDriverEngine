#ifndef CAMERA_H
#define CAMERA_H

#include "math/Vector.h"

class Volume;

extern Vector3D g_cameraVelocity;
extern Vector3D g_cameraPosition;
extern Vector3D g_cameraAngles;

extern Vector3D g_cameraMoveDir;

void UpdateCameraMovement(float deltaTime, float speedModifier);

void SetupCameraViewAndMatrices(const Vector3D& cameraPosition, const Vector3D& cameraAngles, Volume& outFrustum);

#endif // CAMERA_H
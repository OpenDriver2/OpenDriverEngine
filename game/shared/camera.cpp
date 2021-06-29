

#include "game/shared/main.h"
#include "renderer/gl_renderer.h"
#include "game/shared/render_heightmap.h"
#include "game/shared/render_model.h"

#include "math/Volume.h"
#include "math/convert.h"

extern bool g_displayHeightMap;

const float Z_NEAR = 0.01f;
const float Z_FAR = 100.0f;
const float CAMERA_FOV = 75.0f;

const float CAMERA_MOVEMENT_SPEED_FACTOR = 8.0f;	// in-game units per second (one unit is 4096)
const float CAMERA_MOVEMENT_ACCELERATION = 10;
const float CAMERA_MOVEMENT_DECELERATION = 40;

Vector3D g_cameraVelocity(0);
Vector3D g_cameraPosition(0);
Vector3D g_cameraAngles(25.0f, 45.0f, 0);

Vector3D g_cameraMoveDir(0);

//-------------------------------------------------------
// Updates camera movement for level viewer
//-------------------------------------------------------
void UpdateCameraMovement(float deltaTime, float speedModifier)
{
	Vector3D forward, right;
	AngleVectors(g_cameraAngles, &forward, &right);

	const float maxSpeed = CAMERA_MOVEMENT_SPEED_FACTOR * speedModifier;

	if (lengthSqr(g_cameraMoveDir) > 0.1f &&
		length(g_cameraVelocity) < maxSpeed)
	{
		g_cameraVelocity += g_cameraMoveDir.x * right * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier;
		g_cameraVelocity += g_cameraMoveDir.z * forward * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier;
	}
	else
	{
		float speed = length(g_cameraVelocity);
		if (speed < 1.0f)
			speed = 1.0f;

		g_cameraVelocity -= (g_cameraVelocity / speed) * CAMERA_MOVEMENT_DECELERATION * deltaTime;
	}

	g_cameraPosition += g_cameraVelocity * deltaTime;

	VECTOR_NOPAD cameraPosition = ToFixedVector(g_cameraPosition);

	int height = g_levMap->MapHeight(cameraPosition);

	// debug display
	if (g_displayHeightMap)
	{
		// draw the cell
		VECTOR_NOPAD cameraCell = cameraPosition;
		cameraCell.vy = height;
		DebugDrawDriver2HeightmapCell(cameraCell, ColorRGBA(1, 1, 0.25, 1.0f));
	}

	if (cameraPosition.vy < height)
	{
		cameraPosition.vy = height;
		g_cameraPosition.y = float(height) / ONE_F;
	}
}

extern int g_windowWidth;
extern int g_windowHeight;

//-------------------------------------------------------
// Sets up the camera matrices
//-------------------------------------------------------
void SetupCameraViewAndMatrices(const Vector3D& cameraPosition, const Vector3D& cameraAngles, Volume& outFrustum)
{
	// calculate view matrices
	Matrix4x4 view, proj;

	proj = perspectiveMatrixY(DEG2RAD(CAMERA_FOV), g_windowWidth, g_windowHeight, Z_NEAR, Z_FAR);
	view = rotateZXY4(-DEG2RAD(cameraAngles.x), -DEG2RAD(cameraAngles.y), -DEG2RAD(cameraAngles.z));
	view.translate(-cameraPosition);

	// calculate frustum volume
	outFrustum.LoadAsFrustum(proj * view);

	GR_SetMatrix(MATRIX_VIEW, view);
	GR_SetMatrix(MATRIX_PROJECTION, proj);

	// setup default world position
	GR_SetMatrix(MATRIX_WORLD, identity4());

	GR_UpdateMatrixUniforms();
}
#include "renderer/gl_renderer.h"

#include "routines/models.h"
#include "routines/textures.h"
#include "routines/regions.h"

#include "game/render/render_heightmap.h"
#include "game/render/render_model.h"
#include "game/render/render_level.h"

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include "world.h"
#include "camera.h"

#include "math/Volume.h"
#include "math/convert.h"


const float Z_NEAR = 0.01f;
const float Z_FAR = 100.0f;
const float CAMERA_FOV = 75.0f;

const float CAMERA_MOVEMENT_SPEED_FACTOR = 8.0f;	// in-game units per second (one unit is 4096)
const float CAMERA_MOVEMENT_ACCELERATION = 10;
const float CAMERA_MOVEMENT_DECELERATION = 40;

Vector3D g_cameraVelocity(0);
Vector3D g_cameraPosition(0);
Vector3D g_cameraAngles(0);

//-------------------------------------------------------
// Updates camera movement for level viewer
//-------------------------------------------------------
void UpdateCameraMovement(float deltaTime, float speedModifier, const Vector3D& moveDir)
{
	Vector3D forward, right;
	AngleVectors(g_cameraAngles, &forward, &right);

	const float maxSpeed = CAMERA_MOVEMENT_SPEED_FACTOR * speedModifier;

	if (lengthSqr(moveDir) > 0.1f &&
		length(g_cameraVelocity) < maxSpeed)
	{
		g_cameraVelocity += moveDir.x * right * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier;
		g_cameraVelocity += moveDir.z * forward * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier;
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

	int height = CWorld::MapHeight(cameraPosition);

	// debug display
	if (g_levRenderProps.displayHeightMap)
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

CameraViewParams CCamera::MainView;

void CCamera::Lua_Init(sol::state& lua)
{
	auto engine = lua["engine"].get_or_create<sol::table>();

	auto world = engine["Camera"].get_or_create<sol::table>();

	world["MainView"] = &MainView;

	// level properties
	engine.new_usertype<CameraViewParams>("CameraViewParams",
		"position", &CameraViewParams::position,
		"angles", &CameraViewParams::angles,
		"fov", &CameraViewParams::fov);
}

//-------------------------------------------------------
// Sets up the camera matrices
//-------------------------------------------------------
void CCamera::SetupViewAndMatrices(const CameraViewParams& cameraParams, Volume& outFrustum)
{
	// calculate view matrices
	Matrix4x4 view, proj;

	proj = perspectiveMatrixY(DEG2RAD(cameraParams.fov), g_windowWidth, g_windowHeight, Z_NEAR, Z_FAR);
	view = rotateZXY4(-DEG2RAD(cameraParams.angles.x), -DEG2RAD(cameraParams.angles.y), -DEG2RAD(cameraParams.angles.z));
	view.translate(-cameraParams.position);

	// calculate frustum volume
	outFrustum.LoadAsFrustum(proj * view);

	GR_SetMatrix(MATRIX_VIEW, view);
	GR_SetMatrix(MATRIX_PROJECTION, proj);

	// setup default world position
	GR_SetMatrix(MATRIX_WORLD, identity4());

	GR_UpdateMatrixUniforms();
}
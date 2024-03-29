#include "game/pch.h"
#include "camera.h"

const float Z_NEAR = 0.01f;
const float Z_FAR = 100.0f;

extern int g_windowWidth;
extern int g_windowHeight;

CameraViewParams CCamera::MainView;
Vector3D CCamera::MainViewVelocity;

void CCamera::Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();
	auto engine = lua["engine"].get_or_create<sol::table>();

	auto world = engine["Camera"].get_or_create<sol::table>();

	world["MainView"] = &MainView;
	world["MainViewVelocity"] = &MainViewVelocity;

	// level properties
	{
		LUADOC_TYPE();
		engine.new_usertype<CameraViewParams>(
			LUADOC_T("CameraViewParams"),

			LUADOC_P("position", "<vec.vec3>"), & CameraViewParams::position,
			LUADOC_P("angles", "<vec.vec3>"), &CameraViewParams::angles,
			LUADOC_P("fov", "<float>"), &CameraViewParams::fov,
			LUADOC_P("right", "<vec.vec3>"), sol::property(&CameraViewParams::GetRight),
			LUADOC_P("up", "<vec.vec3>"), sol::property(&CameraViewParams::GetUp),
			LUADOC_P("forward", "<vec.vec3>"), sol::property(&CameraViewParams::GetForward),
			LUADOC_P("vectors", "<mat3> (readonly)"), sol::property(&CameraViewParams::GetVectors)
		);
	}
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
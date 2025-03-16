#include "core/core_common.h"

#include "sys/scripting/sys_esl.h"

#include "camera.h"

const float Z_NEAR = 0.01f;
const float Z_FAR = 100.0f;

extern int g_windowWidth;
extern int g_windowHeight;

CViewParams CCamera::MainView;
Vector3D CCamera::MainViewVelocity;

void CCamera::Lua_Init(const esl::ScriptState& state)
{
	esl::LuaTable engine = eslSys::GetOrCreateGlobalTable(state, "engine");
	esl::LuaTable cameraTbl = state.CreateTable();
	engine.Set("Camera", cameraTbl);

	cameraTbl.Set("MainView", &MainView);
	cameraTbl.Set("MainViewVelocity", &MainViewVelocity);
}

//-------------------------------------------------------
// Sets up the camera matrices
//-------------------------------------------------------
void CCamera::SetupViewAndMatrices(const CViewParams& cameraParams, Volume& outFrustum)
{
	// calculate view matrices
	Matrix4x4 view, proj;
	cameraParams.GetMatrices(proj, view, g_windowWidth, g_windowHeight, /*cameraParams.GetZNear()*/Z_NEAR, Z_FAR);

	// calculate frustum volume
	outFrustum.LoadAsFrustum(proj * view);

	GR_SetMatrix(MATRIX_VIEW, view);
	GR_SetMatrix(MATRIX_PROJECTION, proj);

	// setup default world position
	GR_SetMatrix(MATRIX_WORLD, identity4());

	GR_UpdateMatrixUniforms();
}
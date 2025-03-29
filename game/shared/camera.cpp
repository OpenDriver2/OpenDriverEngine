#include "core/core_common.h"
#include "camera.h"

const float Z_NEAR = 0.01f;
const float Z_FAR = 100.0f;

extern int g_windowWidth;
extern int g_windowHeight;

CViewParams CCamera::MainView;
Vector3D CCamera::MainViewVelocity;

static Vector3D S_CViewParams_GetForwardVector(const CViewParams& viewParams)
{
	Vector3D vec;
	AngleVectors(viewParams.GetAngles(), &vec);
	return vec;
}

static Vector3D S_CViewParams_GetRightVector(const CViewParams& viewParams)
{
	Vector3D vec;
	AngleVectors(viewParams.GetAngles(), nullptr, &vec);
	return vec;
}

static Vector3D S_CViewParams_GetUpVector(const CViewParams& viewParams)
{
	Vector3D vec;
	AngleVectors(viewParams.GetAngles(), nullptr, nullptr, &vec);
	return vec;
}

EQSCRIPT_TYPE_BEGIN(CViewParams)
	EQSCRIPT_CLONE_FUNC()
	EQSCRIPT_BIND_CONSTRUCTOR()
	EQSCRIPT_BIND_CONSTRUCTOR(const Vector3D&, const Vector3D&, float )
	EQSCRIPT_BIND_FUNC( GetOrigin )
	EQSCRIPT_BIND_FUNC( GetAngles )
	EQSCRIPT_BIND_FUNC( GetFOV )

	EQSCRIPT_BIND_FUNC( SetOrigin )
	EQSCRIPT_BIND_FUNC( SetAngles )
	EQSCRIPT_BIND_FUNC( SetFOV )

	EQSCRIPT_BIND_STATIC_FUNC("Interpolate", CViewParams::Interpolate)
	EQSCRIPT_BIND_STATIC_FUNC("GetForwardVector", S_CViewParams_GetForwardVector)
	EQSCRIPT_BIND_STATIC_FUNC("GetRightVector", S_CViewParams_GetRightVector)
	EQSCRIPT_BIND_STATIC_FUNC("GetUpVector", S_CViewParams_GetUpVector)
EQSCRIPT_TYPE_END

void CCamera::Lua_Init(const esl::ScriptState& state)
{
	state.RegisterClass<CViewParams>();

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
#if 0
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
#endif
}
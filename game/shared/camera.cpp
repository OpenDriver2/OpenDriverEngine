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
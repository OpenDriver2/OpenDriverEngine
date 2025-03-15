#pragma once
#include "scripting/esl.h"
#include "math/Vector.h"
#include "render/ViewParams.h"

class Volume;
class CCamera
{
public:
	static CViewParams		MainView;
	static Vector3D			MainViewVelocity;

	static void SetupViewAndMatrices(const CViewParams& cameraParams, Volume& outFrustum);

	static void Lua_Init(const esl::ScriptState& state);
};

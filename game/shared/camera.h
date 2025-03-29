#pragma once
#include "sys/scripting/sys_esl.h"

#include "math/Vector.h"
#include "render/ViewParams.h"

class Volume;

EQSCRIPT_BIND_TYPE_NO_PARENT(CViewParams, "CViewParams", BY_REF)

class CCamera
{
public:
	static CViewParams		MainView;
	static Vector3D			MainViewVelocity;

	static void SetupViewAndMatrices(const CViewParams& cameraParams, Volume& outFrustum);

	static void Lua_Init(const esl::ScriptState& state);
};

#pragma once
#include "math/Vector.h"
class CMeshBuilder;

class CRender_Util
{
public:
	static void TesselatedShadowQuad(CMeshBuilder& meshBuilder, Vector3D* v, Vector2D* tc);
};

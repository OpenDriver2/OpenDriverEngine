#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include "math/Vector.h"
class CMeshBuilder;

class CRender_Util
{
public:
	static void TesselatedShadowQuad(CMeshBuilder& meshBuilder, Vector3D* v, Vector2D* tc);
};

#endif // RENDER_UTIL_H
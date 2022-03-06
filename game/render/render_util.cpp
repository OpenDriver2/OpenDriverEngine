#include "game/pch.h"

void CRender_Util::TesselatedShadowQuad(CMeshBuilder& meshBuilder, Vector3D* v, Vector2D* tc)
{
	const int xCount = 8;
	const int yCount = 10;
	const int base = 9;
	const float one_by_xcount = 1.0f / float(xCount);
	const float one_by_ycount = 1.0f / float(yCount);

	const Vector3D e0 = v[1] - v[0];
	const Vector3D e1 = v[2] - v[0];

	const Vector2D te0 = tc[1] - tc[0];
	const Vector2D te1 = tc[2] - tc[0];

	// transform verts
	Vector3D convVert[(xCount + 1) * (yCount + 1)];
	Vector2D convTc[(xCount + 1) * (yCount + 1)];
	{
		Vector3D* curVert = convVert;
		Vector2D* curTc = convTc;
		float yFactor = 0.0f;
		for (int iy = 0; iy <= yCount; ++iy)
		{
			float xFactor = 0.0f;
			for (int ix = 0; ix <= xCount; ++ix)
			{
				Vector3D pos = e0 * xFactor + v[0] + e1 * yFactor;
				pos.y = (CWorld::MapHeight(ToFixedVector(pos)) + 10) / ONE_F;

				*curVert++ = pos;
				*curTc++ = te0 * xFactor + tc[0] + te1 * yFactor;

				xFactor += one_by_xcount;
			}
			yFactor += one_by_ycount;
		}
	}

	// render quads
	{
		int idx = 0;
		Vector3D* curVert = convVert;
		Vector2D* curTc = convTc;
		float yFactor = 0.0f;
		for (int iy = 0; iy < yCount; ++iy)
		{
			float xFactor = 0.0f;
			for (int ix = 0; ix < xCount; ++ix)
			{
				meshBuilder.TexturedQuad3(curVert[0], curVert[1], curVert[base], curVert[base + 1],
					curTc[0], curTc[1], curTc[base], curTc[base + 1]);

				xFactor += one_by_xcount;
				curVert++;
				curTc++;
				idx++;
			}
			idx++;
			curVert++;
			curTc++;

			yFactor += one_by_ycount;
		}
	}
}
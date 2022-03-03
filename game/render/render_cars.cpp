#include "game/pch.h"
#include "render_cars.h"
#pragma optimize("", off)
/* TODO:
	- Add car model rendering and denting stuff
*/

GrVAO* CRender_Cars::carShadowVAO = nullptr;
TexDetailInfo_t* CRender_Cars::carShadowDetail = nullptr;
Vector4D CRender_Cars::carShadowUV(0.0f);


void CRender_Cars::InitCarRender()
{
	carShadowVAO = GR_CreateVAO(8192, 8192);
	carShadowDetail = CWorld::FindTextureDetail("CARSHAD");
	if (carShadowDetail) 
	{
		Vector2D shadowUV(carShadowDetail->info.x, carShadowDetail->info.y);
		Vector2D shadowWH(carShadowDetail->info.width, carShadowDetail->info.height);
		shadowUV += 1;
		shadowWH -= 2;
		carShadowUV = Vector4D(shadowUV / TEXPAGE_SIZE_Y, (shadowUV + shadowWH) / TEXPAGE_SIZE_Y);
	}
}

void CRender_Cars::TerminateCarRender()
{
	GR_DestroyVAO(carShadowVAO);
	carShadowVAO = nullptr;
}


void CRender_Cars::MangleWheelModel(MODEL* model)
{
	UV_INFO tmpUV2;
	uchar tmpUV;
	uint v0, v1, v2;

	// do some fuckery swaps
	POLYFT4* src = (POLYFT4*)model->pPolyAt(0);

	// wheel faces
	v0 = *(uint*)&src[2].v0;
	v1 = *(uint*)&src[2].uv0;
	v2 = *(uint*)&src[2].uv2;
	*(uint*)src = *(uint*)(src + 2);
	*(uint*)&src[0].v0 = v0;
	*(uint*)&src[0].uv0 = v1;
	*(uint*)&src[0].uv2 = v2;
	src[0].color = src[2].color;

	v0 = *(uint*)&src[3].v0;
	v1 = *(uint*)&src[3].uv0;
	v2 = *(uint*)&src[3].uv2;
	*(uint*)(src + 1) = *(uint*)(src + 3);
	*(uint*)&src[1].v0 = v0;
	*(uint*)&src[1].uv0 = v1;
	*(uint*)&src[1].uv2 = v2;
	src[1].color = src[3].color;

	v0 = *(uint*)&src[4].v0;
	v1 = *(uint*)&src[4].uv0;
	v2 = *(uint*)&src[4].uv2;
	*(uint*)(src + 2) = *(uint*)(src + 4);
	*(uint*)&src[2].v0 = v0;
	*(uint*)&src[2].uv0 = v1;
	*(uint*)&src[2].uv2 = v2;
	src[2].color = src[4].color;

	v0 = *(uint*)&src[6].v0;
	v1 = *(uint*)&src[6].uv0;
	v2 = *(uint*)&src[6].uv2;
	*(uint*)(src + 3) = *(uint*)(src + 6);
	*(uint*)&src[3].v0 = v0;
	*(uint*)&src[3].uv0 = v1;
	*(uint*)&src[3].uv2 = v2;
	src[3].color = src[6].color;

	src[2].v2 = 23;
	src[2].v3 = 22;

	src[2].v0 = 21;
	src[2].v1 = 20;

	src[3].v2 = 19;
	src[3].v3 = 18;

	src[3].v1 = 17;
	src[3].v0 = 16;

	src[4].v2 = 22;
	src[4].v3 = 23;

	src[4].v0 = 19;
	src[4].v1 = 18;

	src[5].v2 = 16;
	src[5].v3 = 17;

	src[5].v1 = 21;
	src[5].v0 = 20;

	// make untextured
	src[2].id = 21;
	src[3].id = 21;
	src[4].id = 21;
	src[5].id = 21;
	src[2].texture_set = 255;
	src[3].texture_set = 255;
	src[4].texture_set = 255;
	src[5].texture_set = 255;
	src[2].color = CVECTOR{0};
	src[3].color = CVECTOR{0};
	src[4].color = CVECTOR{0};
	src[5].color = CVECTOR{0};

	model->num_polys = 6;
}

void CRender_Cars::DrawCars(Array<CCar*>& cars)
{
	if (!carShadowDetail)
		return;
	CMeshBuilder carShadow(carShadowVAO);

	carShadow.Begin(PRIM_TRIANGLE_STRIP);

	for (usize i = 0; i < cars.size(); i++)
	{
		CCar* cp = cars[i];
		cp->DrawCar();

		AddCarShadow(carShadow, cp);
	}

	TextureID carShadowPage = CWorld::GetHWTexture(carShadowDetail->pageNum, 0);
	CRenderModel::SetupModelShader();
	GR_SetBlendMode(BM_SUBTRACT);
	GR_SetDepthMode(1, 0);
	GR_SetTexture(carShadowPage);
	GR_SetMatrix(MATRIX_WORLD, identity4());
	GR_UpdateMatrixUniforms();
	GR_SetCullMode(CULL_NONE);
	carShadow.End();

	// restore depth mode
	GR_SetDepthMode(1, 1);
}

void CRender_Cars::AddCarShadow(CMeshBuilder& meshBuilder, CCar* car)
{
	if (!carShadowDetail || !carShadowVAO)
		return;

	const CarCosmetics& car_cos = car->GetCosmetics();

	Matrix4x4 drawCarMat = car->GetInterpolatedDrawMatrix4();

	meshBuilder.Color4f(0.6f, 0.6f, 0.6f, 0.35f);

	Vector3D result[4];
	for (int i = 0; i < 4; i++)
	{
		Vector3D pointPos = (drawCarMat * Vector4D(FromFixedVector(car_cos.cPoints[i]), 1.0f)).xyz();

		sdPlane surfacePtr;
		VECTOR_NOPAD posFix, surfaceNormal;
		CWorld::FindSurface(ToFixedVector(pointPos), surfaceNormal, posFix, surfacePtr);

		posFix.vy += 8;
		result[i] = FromFixedVector(posFix);
	}

	const Vector2D tl = carShadowUV.xy();
	const Vector2D br = carShadowUV.zw();

	meshBuilder.TexturedQuad3(result[0], result[1], result[2], result[3], 
		tl, Vector2D(br.x, tl.y), Vector2D(tl.x, br.y), br);
}
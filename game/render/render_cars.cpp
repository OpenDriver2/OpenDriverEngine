#include "core/core_common.h"
#include "math/convert.h"

#include "routines/models.h"
#include "routines/textures.h"
#include "game/shared/world.h"
#include "game/shared/cars.h"

#include "render_cars.h"
#include "render_model.h"
#include "render_util.h"

/* TODO:
	- Add car model rendering and denting stuff
*/

GrVAO* CRender_Cars::ShadowVAO = nullptr;
TexDetailInfo_t* CRender_Cars::ShadowDetail = nullptr;
Vector4D CRender_Cars::ShadowUV(0.0f);


void CRender_Cars::Init()
{
	ShadowVAO = GR_CreateVAO(8192, 8192);
	ShadowDetail = CWorld::FindTextureDetail("CARSHAD");
	if (ShadowDetail) 
	{
		Vector2D shadowUV(ShadowDetail->info.x, ShadowDetail->info.y);
		Vector2D shadowWH(ShadowDetail->info.width, ShadowDetail->info.height);
		shadowUV += 1;
		shadowWH -= 2;
		ShadowUV = Vector4D(shadowUV / TEXPAGE_SIZE_Y, (shadowUV + shadowWH) / TEXPAGE_SIZE_Y);
	}
}

void CRender_Cars::Terminate()
{
	GR_DestroyVAO(ShadowVAO);
	ShadowVAO = nullptr;
}

void CRender_Cars::MangleWheelModel(MODEL* model)
{
	UV_INFO tmpUV2;
	uint8 tmpUV;
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

void CRender_Cars::DrawCars(ArrayCRef<CCar*> cars, const CViewParams& view)
{
	if (!ShadowDetail)
		return;

	CMeshBuilder carShadow(ShadowVAO);
	carShadow.Begin(PRIM_TRIANGLE_STRIP);

	for (int i = 0; i < cars.numElem(); i++)
	{
		CCar* cp = cars[i];
		const float distToView = distance(view.GetOrigin(), FromFixedVector(cp->GetPosition()));

		// TODO: LOD!!!
		cp->DrawCar();

		AddCarShadow(carShadow, cp, distToView);
	}

	TextureID carShadowPage = CWorld::GetHWTexture(ShadowDetail->pageNum, 0);
	CRenderModel::SetupModelShader();
	GR_SetPolygonOffset(0.0005f, 0.001f);
	GR_SetBlendMode(BM_SUBTRACT);
	GR_SetDepthMode(1, 0);
	GR_SetTexture(carShadowPage);
	GR_SetMatrix(MATRIX_WORLD, identity4());
	GR_UpdateMatrixUniforms();
	GR_SetCullMode(CULL_NONE);
	carShadow.End();

	// restore render states
	GR_SetDepthMode(1, 1);
	GR_SetPolygonOffset(0.0f, 0.0f);
}


void CRender_Cars::AddCarShadow(CMeshBuilder& meshBuilder, CCar* car, float distance)
{
	if (!ShadowDetail || !ShadowVAO)
		return;

	const CarCosmetics& car_cos = car->GetCosmetics();
	const Matrix4x4 drawCarMat = car->GetInterpolatedDrawMatrix4();

	meshBuilder.Color4f(0.6f, 0.6f, 0.6f, 0.35f);

	Vector3D verts[4];
	Vector2D uvs[4];
	for (int i = 0; i < 4; i++)
		verts[i] = (drawCarMat * Vector4D(FromFixedVector(car_cos.cPoints[i]), 1.0f)).xyz();;

	const Vector2D tl = ShadowUV.xy();
	const Vector2D br = ShadowUV.zw();

	uvs[0] = tl;
	uvs[1] = Vector2D(br.x, tl.y);
	uvs[2] = Vector2D(tl.x, br.y);
	uvs[3] = br;

	const bool highDetail = distance < 6.0f;

	if (highDetail)
	{
		CRender_Util::TesselatedShadowQuad(meshBuilder, verts, uvs);
	}
	else
	{
		for (int i = 0; i < 4; i++)
			verts[i].y = (CWorld::MapHeight(ToFixedVector(verts[i])) + 4) / ONE_F;

		meshBuilder.TexturedQuad3(verts[0], verts[1], verts[2], verts[3],
									uvs[0], uvs[1], uvs[2], uvs[3]);
	}	
}
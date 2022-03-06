#include "game/pch.h"
#include "render_level.h"
#pragma optimize("", off)

// extern some vars
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

LevelRenderProps CRender_Level::RenderProps;
GrVAO* CRender_Level::ShadowVAO = nullptr;

void CRender_Level::InitRender()
{
	ShadowVAO = GR_CreateVAO(128, 128);
}

void CRender_Level::TerminateRender()
{
	GR_DestroyVAO(ShadowVAO);
	ShadowVAO = nullptr;
}

int g_cellsDrawDistance = 441 * 10;
float g_maxShadowDistance = 6.0f;

//-----------------------------------------------------------------

const float MODEL_LOD_HIGH_MIN_DISTANCE = 1.0f;
const float MODEL_LOD_LOW_MIN_DISTANCE = 5.0f;

//-------------------------------------------------------
// returns specific model or LOD model
// based on the distance from camera
//-------------------------------------------------------
ModelRef_t* GetModelCheckLods(int index, float distSqr)
{
	ModelRef_t* baseRef = g_levModels.GetModelByIndex(index);

	if (CRender_Level::RenderProps.noLod)
		return baseRef;

	ModelRef_t* retRef = baseRef;
	if (baseRef->highDetailId != 0xFFFF)
	{
		if (distSqr < MODEL_LOD_HIGH_MIN_DISTANCE * MODEL_LOD_HIGH_MIN_DISTANCE)
			return g_levModels.GetModelByIndex(baseRef->highDetailId);
	}

	if (baseRef->lowDetailId != 0xFFFF)
	{
		if (distSqr > MODEL_LOD_LOW_MIN_DISTANCE * MODEL_LOD_LOW_MIN_DISTANCE)
			return g_levModels.GetModelByIndex(baseRef->lowDetailId);
	}

	return retRef;
}

// stats counters
int g_drawnCells;
int g_drawnModels;
int g_drawnPolygons;

void CRender_Level::DrawCellObject(
	const CELL_OBJECT& co,
	const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume,
	bool buildingLighting)
{
	Vector3D absCellPosition = FromFixedVector(co.pos);
	absCellPosition.y *= -1.0f;

	const float distanceFromCamera = lengthSqr(absCellPosition - cameraPos);

	ModelRef_t* ref = GetModelCheckLods(co.type, distanceFromCamera);
	if (!ref->model || !ref->enabled)
		return;

	const MODEL* model = ref->model;
	// check if it is in view
	const float boundSphere = model->bounding_sphere * RENDER_SCALING * 2.0f;
	if (!frustrumVolume.IsSphereInside(absCellPosition, boundSphere))
		return;

	DrawCellObject(co, absCellPosition, ref, cameraAngleY, buildingLighting);
}


void CRender_Level::DrawCellObject(
	const CELL_OBJECT& co, const Vector3D& position, const ModelRef_t* ref, 
	float cameraAngleY,
	bool buildingLighting)
{
	if (co.type >= MAX_MODELS)
	{
		// WHAT THE FUCK?
		return;
	}

	CRenderModel* renderModel = (CRenderModel*)ref->userData;

	if (!renderModel)
		return;

	const MODEL* model = ref->model;

	bool isGround = false;

	if ((model->shape_flags & (SHAPE_FLAG_WATER | SHAPE_FLAG_TILE)) ||
		(model->flags2 & (MODEL_FLAG_PATH | MODEL_FLAG_GRASS)))
	{
		isGround = true;
	}

	// compute world matrix
	{
		Matrix4x4 objectMatrix;

		if (model->shape_flags & SHAPE_FLAG_SPRITE)
			objectMatrix = rotateY4(DEG2RAD(cameraAngleY));
		else
			objectMatrix = g_objectMatrix[co.yang];

		objectMatrix.setTranslationTransposed(position);

		GR_SetMatrix(MATRIX_WORLD, objectMatrix);
		GR_UpdateMatrixUniforms();
	}

	// apply lighting
	if ((isGround || !buildingLighting) && RenderProps.nightMode)
		CRenderModel::SetupLightingProperties(RenderProps.nightAmbientScale, RenderProps.nightLightScale);
	else
		CRenderModel::SetupLightingProperties(RenderProps.ambientScale, RenderProps.lightScale);

	renderModel->Draw();

	g_drawnModels++;
	g_drawnPolygons += ref->model->num_polys;
}

void CRender_Level::DrawObjectShadow(CMeshBuilder& shadowMesh, const Matrix3x3& shadowMat, const ModelRef_t* ref, const Vector3D& position, float distance)
{
	const bool highDetail = distance < 2.0f;
	const float shadowAlpha = 1.0 - clamp(pow(distance / g_maxShadowDistance, 2.0f), 0.0f, 1.0f);

	MODEL* model = ref->model;
	if (ref->baseInstance)
		model = ref->baseInstance->model;

	dpoly_t dec_face;
	int face_ofs = 0;
	for (int i = 0; i < model->num_polys; ++i)
	{
		face_ofs += decode_poly(model->pPolyAt(face_ofs), &dec_face);

		assert(dec_face.flags & FACE_IS_QUAD);

		Vector3D verts[4];
		Vector2D uvs[4];
		for (int i = 0; i < 4; i++)
		{
			Vector3D vec = shadowMat * FromFixedVector(*model->pVertex(dec_face.vindices[i]));
			vec.y *= -1.0f;
			verts[i] = position + vec + vec3_up;

			UV_INFO uv = *(UV_INFO*)dec_face.uv[i];
			uvs[i].x = ((float)uv.u + 0.5f) / TEXPAGE_SIZE_Y;
			uvs[i].y = ((float)uv.v + 0.5f) / TEXPAGE_SIZE_Y;
		}

		// TODO: batching!!!
		shadowMesh.Begin(PRIM_TRIANGLE_STRIP);
		shadowMesh.Color4f(0.0f, 0.0f, 0.0f, shadowAlpha * 0.4f);

		if (highDetail)
		{
			swap(verts[3], verts[2]);
			swap(uvs[3], uvs[2]);
			CRender_Util::TesselatedShadowQuad(shadowMesh, verts, uvs);
		}
		else
		{
			for (int i = 0; i < 4; i++)
				verts[i].y = (CWorld::MapHeight(ToFixedVector(verts[i])) + 10) / ONE_F;

			shadowMesh.TexturedQuad3(verts[0], verts[1], verts[3], verts[2], uvs[0], uvs[1], uvs[3], uvs[2]);
		}

		TextureID shadowPage = CWorld::GetHWTexture(dec_face.page, 0);
		GR_SetTexture(shadowPage);
		shadowMesh.End();
	}
}

//-------------------------------------------------------
// Draws the map of Driver 1 or Driver 2
//-------------------------------------------------------
void CRender_Level::DrawMap(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume)
{
	g_drawnCells = 0;
	g_drawnModels = 0;
	g_drawnPolygons = 0;

	CBaseLevelMap* levMap = g_levMap;

	const bool driver2Map = levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);

	XZPAIR cell;
	levMap->WorldPositionToCellXZ(cell, cameraPosition);

	// drawing state
	static Array<CELL_OBJECT*> drawObjects;
	static Array<Vector3D> drawObjectPositions;
	static Array<ModelRef_t*> drawObjectModel;
	static Array<float> drawObjectDistance;
	static Array<int> shadowObjectIds;
	int numObjects = 0;
	int numObjectShadows = 0;
	{
		const int maxObjectsPerCell = 64;
		const int totalObjects = g_cellsDrawDistance * maxObjectsPerCell;
		drawObjects.reserve(totalObjects);
		drawObjectPositions.reserve(totalObjects);
		drawObjectModel.reserve(totalObjects);
		drawObjectDistance.reserve(totalObjects);
		shadowObjectIds.reserve(totalObjects);
	}

	int i = g_cellsDrawDistance;
	int vloop = 0;
	int hloop = 0;
	int dir = 0;

	XZPAIR icell;

	CELL_ITERATOR_CACHE iteratorCache;
	CBaseLevelRegion* currentRegion = nullptr;

	// walk through all cells
	while (i >= 0)
	{
		icell.x = cell.x + hloop;
		icell.z = cell.z + vloop;

		if (icell.x > -1 && icell.x < levMap->GetCellsAcross() &&
			icell.z > -1 && icell.z < levMap->GetCellsDown())
		{
			g_drawnCells++;
			CBaseLevelRegion* reg = g_levMap->GetRegion(icell);

			if (currentRegion != reg)
				memset(&iteratorCache, 0, sizeof(iteratorCache));
			currentRegion = reg;

			CWorld::ForEachCellObjectAt(icell, [&cameraPos, &frustrumVolume, &numObjects, &numObjectShadows](int listType, CELL_OBJECT* co) {
				if (listType != -1 && !RenderProps.displayAllCellLevels)
					return false;

				Vector3D absCellPosition = FromFixedVector(co->pos);
				absCellPosition.y *= -1.0f;
				
				const float distanceFromCamera = lengthSqr(absCellPosition - cameraPos);
				
				ModelRef_t* ref = GetModelCheckLods(co->type, distanceFromCamera);
				if (!ref->model || !ref->enabled)
					return true;
				
				const MODEL* model = ref->model;

				// check if it is in view
				const float boundSphere = model->bounding_sphere * RENDER_SCALING * 2.0f;
				if (!frustrumVolume.IsSphereInside(absCellPosition, boundSphere))
					return true;

				if (distanceFromCamera < g_maxShadowDistance && (model->shape_flags & SHAPE_FLAG_SPRITE))
					shadowObjectIds[numObjectShadows++] = numObjects;

				// add
				drawObjects[numObjects] = co;
				drawObjectPositions[numObjects] = absCellPosition;
				drawObjectDistance[numObjects] = distanceFromCamera;
				drawObjectModel[numObjects] = ref;
				numObjects++;

				return true;
			}, &iteratorCache);
		}

		if (dir == 0)
		{
			dir = (++hloop + vloop == 1) ? 1 : dir;
		}
		else if (dir == 1)
		{
			dir = (hloop == ++vloop) ? 2 : dir;
		}
		else if (dir == 2)
		{
			dir = (--hloop + vloop == 0) ? 3 : dir;
		}
		else
		{
			dir = (hloop == --vloop) ? 0 : dir;
		}

		i--;
	}

	// at least once we should do that
	CRenderModel::SetupModelShader();

	// draw object list
	for (int i = 0; i < numObjects; i++)
	{
		DrawCellObject(*drawObjects[i], drawObjectPositions[i], drawObjectModel[i], cameraAngleY, driver2Map);

		// debug
		if (RenderProps.displayCollisionBoxes)
			CRenderModel::DrawModelCollisionBox(drawObjectModel[i], drawObjects[i]->pos, drawObjects[i]->yang);
	}

	if (ShadowVAO)
	{
		// compulte shadow matrix
		const OUT_CELL_FILE_HEADER& cellHeader = g_levMap->GetMapInfo();
		Vector3D lightVector = normalize(FromFixedVector(cellHeader.light_source));

		const float shadowAngle = DEG2RAD(90.0f) + atan2f(lightVector.z, lightVector.x);
		Matrix3x3 shadowMat = rotateY3(shadowAngle) * rotateX3(DEG2RAD(90.0f));

		CRenderModel::SetupModelShader();
		GR_SetPolygonOffset(10.5f);
		GR_SetBlendMode(BM_SEMITRANS_ALPHA);
		GR_SetDepthMode(1, 0);

		GR_SetMatrix(MATRIX_WORLD, identity4());
		GR_UpdateMatrixUniforms();
		GR_SetCullMode(CULL_NONE);

		CMeshBuilder shadowMesh(ShadowVAO, 1024);
		for (int i = 0; i < numObjectShadows; i++)
		{
			const int objIdx = shadowObjectIds[i];
			DrawObjectShadow(shadowMesh, shadowMat, drawObjectModel[objIdx], drawObjectPositions[objIdx], drawObjectDistance[objIdx]);
		}

		// restore render states
		GR_SetDepthMode(1, 1);
		GR_SetPolygonOffset(0.0f);
		GR_SetBlendMode(BM_NONE);
	}
}

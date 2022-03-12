#include "game/pch.h"
#include "render_level.h"

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
float g_maxShadowDistance = 12.0f;

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
int g_drawnRegions;
int g_debugListCellsDrawn;

void CRender_Level::DrawObject(const DRAWABLE& drawable, const Vector3D& cameraPos, const Volume& frustrumVolume, bool buildingLighting)
{
	if (drawable.model >= MAX_MODELS)
		return;

	const float distanceFromCamera = lengthSqr(drawable.position - cameraPos);

	ModelRef_t* ref = GetModelCheckLods(drawable.model, distanceFromCamera);
	if (!ref->model || !ref->enabled)
		return;

	const MODEL* model = ref->model;
	// check if it is in view
	const float boundSphere = model->bounding_sphere * RENDER_SCALING * 2.0f;
	if (!frustrumVolume.IsSphereInside(drawable.position, boundSphere))
		return;

	CRenderModel* renderModel = (CRenderModel*)ref->userData;

	if (!renderModel)
		return;

	bool isGround = false;

	if ((model->shape_flags & (SHAPE_FLAG_WATER | SHAPE_FLAG_TILE)) ||
		(model->flags2 & (MODEL_FLAG_PATH | MODEL_FLAG_GRASS)))
	{
		isGround = true;
	}

	Matrix4x4 objectMatrix = translate(drawable.position) * rotateXYZ4(drawable.angles.x, drawable.angles.y, drawable.angles.z);

	GR_SetMatrix(MATRIX_WORLD, objectMatrix);
	GR_UpdateMatrixUniforms();

	const float modelLightLevel = ref->lightingLevel;

	// apply lighting
	if ((isGround || !buildingLighting) && RenderProps.nightMode)
		CRenderModel::SetupLightingProperties(RenderProps.nightAmbientScale * modelLightLevel, RenderProps.nightLightScale * modelLightLevel);
	else
		CRenderModel::SetupLightingProperties(RenderProps.ambientScale * modelLightLevel, RenderProps.lightScale * modelLightLevel);

	renderModel->Draw();

	g_drawnModels++;
	g_drawnPolygons += ref->model->num_polys;
}

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

	// compute world matrix
	Matrix4x4 objectMatrix;
	{
		if (model->shape_flags & SHAPE_FLAG_SPRITE)
			objectMatrix = rotateY4(DEG2RAD(cameraAngleY));
		else
			objectMatrix = g_objectMatrix[co.yang];
		objectMatrix.setTranslationTransposed(absCellPosition);
	}

	DrawCellObject(co, objectMatrix, ref, cameraAngleY, buildingLighting);
}


void CRender_Level::DrawCellObject(
	const CELL_OBJECT& co, const Matrix4x4& worldMat, const ModelRef_t* ref, 
	float cameraAngleY,
	bool buildingLighting)
{
	if (co.type >= MAX_MODELS)
	{
		// WHAT THE FUCK?
		return;
	}

	if (co.pos.vx == OBJECT_SMASHED_MARK)
	{
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

	GR_SetMatrix(MATRIX_WORLD, worldMat);
	GR_UpdateMatrixUniforms();
	
	const bool ignoreLightingLevel = (model->shape_flags & SHAPE_FLAG_SPRITE) && (co.yang & 63) == 63;
	const float modelLightLevel = ignoreLightingLevel ? 1.0f : ref->lightingLevel;

	// apply lighting
	if ((isGround || !buildingLighting) && RenderProps.nightMode)
		CRenderModel::SetupLightingProperties(RenderProps.nightAmbientScale * modelLightLevel, RenderProps.nightLightScale * modelLightLevel);
	else
		CRenderModel::SetupLightingProperties(RenderProps.ambientScale * modelLightLevel, RenderProps.lightScale * modelLightLevel);

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

		ASSERT(dec_face.flags & FACE_IS_QUAD);

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
	g_drawnRegions = 0;
	g_debugListCellsDrawn = 0;

	const CBaseLevelMap* levMap = g_levMap;

	const bool driver2Map = levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	const VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);
	static XZPAIR lastCameraPosCell{ 0,0 };
	static float lastCameraAngleY = 0.0f;

	XZPAIR cameraPosCell;
	levMap->WorldPositionToCellXZ(cameraPosCell, cameraPosition);

	bool needMapIteration = (RenderProps.displayCellObjectList != -1);
	if (cameraPosCell.x != lastCameraPosCell.x ||
		cameraPosCell.z != lastCameraPosCell.z ||
		fabs(AngleDiff(DEG2RAD(lastCameraAngleY), DEG2RAD(cameraAngleY))) > DEG2RAD(10.0f))
	{
		lastCameraAngleY = cameraAngleY;
		lastCameraPosCell = cameraPosCell;
		needMapIteration = true;
	}

	// drawing state
	static Array<CELL_OBJECT*> drawObjects;
	static Array<ModelRef_t*> drawObjectModel;
	static Array<float> drawObjectDistance;
	static Array<int> drawObjectListType;
	static Array<int> shadowObjectIds;
	static Array<Vector3D> shadowObjectPos;
	static int numObjects = 0;
	static int numObjectShadows = 0;

	if(needMapIteration)
	{
		numObjects = 0;
		const int maxObjectsPerCell = 64;
		const int totalObjects = g_cellsDrawDistance * maxObjectsPerCell;
		drawObjects.reserve(totalObjects);
		drawObjectModel.reserve(totalObjects);
		drawObjectDistance.reserve(totalObjects);
		drawObjectListType.reserve(totalObjects);
		shadowObjectIds.reserve(totalObjects);
		shadowObjectPos.reserve(totalObjects);
	}

	if (needMapIteration)
	{
		CELL_ITERATOR_CACHE iteratorCache;
		int prevRegion = -1;

		int i = g_cellsDrawDistance;
		int vloop = 0;
		int hloop = 0;
		int dir = 0;

		// walk through all cells
		while (i >= 0)
		{
			XZPAIR icell;
			icell.x = cameraPosCell.x + hloop;
			icell.z = cameraPosCell.z + vloop;

			if (icell.x > -1 && icell.x < levMap->GetCellsAcross() &&
				icell.z > -1 && icell.z < levMap->GetCellsDown())
			{
				g_drawnCells++;
				const int regionIdx = g_levMap->GetRegionIndex(icell);

				// BUG: cell iterator is prone to duplication
				// due to very high drawn cell count it gets overflown
				if (g_drawnCells > 441 && regionIdx != prevRegion)
					memset(&iteratorCache, 0, sizeof(iteratorCache));
				prevRegion = regionIdx;

				CWorld::ForEachCellObjectAt(icell, [&cameraPos, &cameraAngleY](int listType, CELL_OBJECT* co) {

					bool skipRotation = false;
					if (listType != -1 && RenderProps.displayAllCellLevels)
					{
						if (RenderProps.displayCellObjectList != -1 && listType != RenderProps.displayCellObjectList)
							return true;

						if (RenderProps.displayCellObjectList == listType)
						{
							g_debugListCellsDrawn++;
							skipRotation = true;
						}
					}

					auto& foundCellList = CWorld::CellLists.find(listType);
					if (foundCellList != CWorld::CellLists.end())
					{
						// TODO: apply transform to objectMatrix
						CELL_LIST_DESC& cellList = *foundCellList;
						if (!cellList.visible)
							return true;
					}
					else if(listType != -1 && !RenderProps.displayAllCellLevels)
					{
						return true;
					}

					Vector3D absCellPosition = FromFixedVector(co->pos);
					absCellPosition.y *= -1.0f;

					const float distanceFromCamera = lengthSqr(absCellPosition - cameraPos);

					ModelRef_t* ref = GetModelCheckLods(co->type, distanceFromCamera);
					if (!ref->model || !ref->enabled)
						return true;

					const MODEL* model = ref->model;

					// check if it is in view
					const float boundSphereUnits = model->bounding_sphere;
					const float boundSphere = boundSphereUnits * RENDER_SCALING * 2.0f;

					if (model->flags2 || (model->shape_flags & (SHAPE_FLAG_ROAD | SHAPE_FLAG_SPRITE))) // basically any flag is candidate
					{
						const float sphereMultiplier = (model->shape_flags & SHAPE_FLAG_ROAD) ? 3.0f : 1.0f;
						if ((boundSphereUnits * sphereMultiplier) / distanceFromCamera < 1.0f)
							return true;
					}

					// add
					drawObjects[numObjects] = co;
					drawObjectDistance[numObjects] = distanceFromCamera;
					drawObjectModel[numObjects] = ref;
					drawObjectListType[numObjects] = skipRotation ? -1 : listType;
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
	}

	// at least once we should do that
	CRenderModel::SetupModelShader();

	numObjectShadows = 0;

	// draw object list
	for (int i = 0; i < numObjects; i++)
	{
		const MODEL* model = drawObjectModel[i]->model;

		Vector3D absCellPosition = FromFixedVector(drawObjects[i]->pos);
		absCellPosition.y *= -1.0f;

		// compute world matrix
		Matrix4x4 objectMatrix;
		{
			if (model->shape_flags & SHAPE_FLAG_SPRITE)
				objectMatrix = rotateY4(DEG2RAD(cameraAngleY));
			else
				objectMatrix = g_objectMatrix[drawObjects[i]->yang];
			objectMatrix.setTranslationTransposed(absCellPosition);
		}

		auto& foundCellList = CWorld::CellLists.find(drawObjectListType[i]);
		if (foundCellList != CWorld::CellLists.end())
		{
			// apply transform to objectMatrix
			CELL_LIST_DESC& cellList = *foundCellList;
			if (cellList.dirty)
			{
				Matrix4x4 listTransform = rotateXYZ4(
					float(cellList.rotation.vx) * TO_RADIAN,
					float(cellList.rotation.vy) * TO_RADIAN,
					float(cellList.rotation.vz) * TO_RADIAN);
				listTransform.setTranslationTransposed(FromFixedVector(cellList.position));

				cellList.transform = listTransform * !cellList.pivotMatrix;
				cellList.dirty = false;
			}

			objectMatrix = cellList.transform * objectMatrix;
		}

		{
			// check if it is in view
			const float boundSphereUnits = model->bounding_sphere;
			const float boundSphere = boundSphereUnits * RENDER_SCALING * 2.0f;

			if (!frustrumVolume.IsSphereInside(Vector3D(absCellPosition.x, cameraPos.y, absCellPosition.z), boundSphere))
				continue;
		}

		if (drawObjectDistance[i] < g_maxShadowDistance && (model->shape_flags & SHAPE_FLAG_SPRITE))
		{
			shadowObjectPos[numObjectShadows] = absCellPosition;
			shadowObjectIds[numObjectShadows++] = i;
		}

		DrawCellObject(*drawObjects[i], objectMatrix, drawObjectModel[i], cameraAngleY, driver2Map);

		// debug
		if (RenderProps.displayCollisionBoxes)
			CRenderModel::DrawModelCollisionBox(drawObjectModel[i], drawObjects[i]->pos, drawObjects[i]->yang);
	}

	if (ShadowVAO && numObjectShadows > 0)
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
			DrawObjectShadow(shadowMesh, shadowMat, drawObjectModel[objIdx], shadowObjectPos[i], drawObjectDistance[objIdx]);
		}

		// restore render states
		GR_SetDepthMode(1, 1);
		GR_SetPolygonOffset(0.0f);
		GR_SetBlendMode(BM_NONE);
	}
}

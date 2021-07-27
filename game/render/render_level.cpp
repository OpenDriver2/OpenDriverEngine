#include "renderer/debug_overlay.h"
#include "renderer/gl_renderer.h"

#include "core/VirtualStream.h"
#include "routines/models.h"
#include "routines/regions_d1.h"
#include "routines/regions_d2.h"
#include "routines/textures.h"

#include "render_model.h"
#include "render_level.h"

#include "math/Volume.h"
#include "math/convert.h"
#include "game/shared/world.h"


// extern some vars
extern String					g_levname;

extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

extern FILE* g_levFile;

LevelRenderProps g_levRenderProps;

int g_cellsDrawDistance = 441 * 10;

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

	if (g_levRenderProps.noLod)
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

void DrawCellObject(const CELL_OBJECT& co, const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume, bool buildingLighting)
{
	if (co.type >= MAX_MODELS)
	{
		// WHAT THE FUCK?
		return;
	}

	Vector3D absCellPosition = FromFixedVector(co.pos);
	absCellPosition.y *= -1.0f;

	float distanceFromCamera = lengthSqr(absCellPosition - cameraPos);

	ModelRef_t* ref = GetModelCheckLods(co.type, distanceFromCamera);

	bool isGround = false;

	Matrix4x4 objectMatrix;

	if (ref->model)
	{
		MODEL* model = ref->model;

		if (model->shape_flags & SHAPE_FLAG_SPRITE)
		{
			objectMatrix = rotateY4(DEG2RAD(cameraAngleY));
		}
		else
		{
			objectMatrix = g_objectMatrix[co.yang];
		}

		if ((model->shape_flags & (SHAPE_FLAG_WATER | SHAPE_FLAG_TILE)) ||
			(model->flags2 & (MODEL_FLAG_PATH | MODEL_FLAG_GRASS)))
		{
			isGround = true;
		}
	}
	objectMatrix.setTranslationTransposed(absCellPosition);
	GR_SetMatrix(MATRIX_WORLD, objectMatrix);
	GR_UpdateMatrixUniforms();

	if (buildingLighting)
	{
		if (isGround && g_levRenderProps.nightMode)
			CRenderModel::SetupLightingProperties(g_levRenderProps.nightAmbientScale, g_levRenderProps.nightLightScale);
		else
			CRenderModel::SetupLightingProperties(g_levRenderProps.ambientScale, g_levRenderProps.lightScale);
	}
	else
	{
		if (g_levRenderProps.nightMode)
			CRenderModel::SetupLightingProperties(g_levRenderProps.nightAmbientScale, g_levRenderProps.nightLightScale);
		else
			CRenderModel::SetupLightingProperties(g_levRenderProps.ambientScale, g_levRenderProps.lightScale);
	}

	CRenderModel* renderModel = (CRenderModel*)ref->userData;

	if (renderModel)
	{
		const float boundSphere = ref->model->bounding_sphere * RENDER_SCALING * 2.0f;
		if (frustrumVolume.IsSphereInside(absCellPosition, boundSphere))
		{
			renderModel->Draw();
			g_drawnModels++;
			g_drawnPolygons += ref->model->num_polys;

			if (g_levRenderProps.displayCollisionBoxes)
				CRenderModel::DrawModelCollisionBox(ref, co.pos, co.yang);
		}
	}
}

//-------------------------------------------------------
// Draws the map of Driver 1 or Driver 2
//-------------------------------------------------------
void DrawMap(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume)
{
	g_drawnCells = 0;
	g_drawnModels = 0;
	g_drawnPolygons = 0;

	CBaseLevelMap* levMap = g_levMap;

	bool driver2Map = levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);

	XZPAIR cell;
	levMap->WorldPositionToCellXZ(cell, cameraPosition);

	static Array<CELL_OBJECT*> drawObjects;
	drawObjects.reserve(g_cellsDrawDistance * 2);
	drawObjects.clear();

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

			CWorld::ForEachCellObjectAt(icell, [](int listType, CELL_OBJECT* co) {
				if (listType != -1 && !g_levRenderProps.displayAllCellLevels)
					return false;

				drawObjects.append(co);
				return true;
			}, &iteratorCache);
		}

		if (dir == 0)
		{
			hloop++;

			if (hloop + vloop == 1)
				dir = 1;
		}
		else if (dir == 1)
		{
			vloop++;

			if (hloop == vloop)
				dir = 2;
		}
		else if (dir == 2)
		{
			hloop--;

			if (hloop + vloop == 0)
				dir = 3;
		}
		else
		{
			vloop--;

			if (hloop == vloop)
				dir = 0;
		}

		i--;
	}

	// at least once we should do that
	CRenderModel::SetupModelShader();

	// draw object list
	for (uint i = 0; i < drawObjects.size(); i++)
	{
		DrawCellObject(*drawObjects[i], cameraPos, cameraAngleY, frustrumVolume, driver2Map);
	}
}

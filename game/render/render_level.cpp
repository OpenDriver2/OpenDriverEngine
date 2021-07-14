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

// extern some vars
extern String					g_levname;

extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

extern FILE* g_levFile;

LevelRenderProps g_levRenderProps;

int g_cellsDrawDistance = 441;

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

	MODEL* model = ref->model;

	float cellRotationRad = -co.yang / 64.0f * PI_F * 2.0f;

	bool isGround = false;

	if (model)
	{
		if (model->shape_flags & SHAPE_FLAG_SPRITE)
		{
			cellRotationRad = DEG2RAD(cameraAngleY);
		}

		if ((model->shape_flags & (SHAPE_FLAG_WATER | SHAPE_FLAG_TILE)) ||
			(model->flags2 & (MODEL_FLAG_PATH | MODEL_FLAG_GRASS)))
		{
			isGround = true;
		}
	}

	Matrix4x4 objectMatrix = translate(absCellPosition) * rotateY4(cellRotationRad);
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

	bool driver2Map = g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16;

	VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);

	CBaseLevelMap* levMap = g_levMap;

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

	// walk through all cells
	while (i >= 0)
	{
		icell.x = cell.x + hloop;
		icell.z = cell.z + vloop;

		if (icell.x > -1 && icell.x < levMap->GetCellsAcross() &&
			icell.z > -1 && icell.z < levMap->GetCellsDown())
		{
			if (driver2Map)
			{
				CDriver2LevelMap* levMapDriver2 = (CDriver2LevelMap*)levMap;

				// Driver 2 map iteration
				CELL_ITERATOR_D2 ci;

				PACKED_CELL_OBJECT* ppco = levMapDriver2->GetFirstPackedCop(&ci, icell);

				if (ppco)
					g_drawnCells++;

				// walk each cell object in cell
				while (ppco)
				{
					if (ci.listType != -1 && !g_levRenderProps.displayAllCellLevels)
						return;

					drawObjects.append(ci.co);

					ppco = levMapDriver2->GetNextPackedCop(&ci);
				}
			}
			else
			{
				CELL_ITERATOR_D1 ci;
				CDriver1LevelMap* levMapDriver1 = (CDriver1LevelMap*)g_levMap;

				// Driver 1 map iteration
				CELL_OBJECT* pco = levMapDriver1->GetFirstCop(&ci, icell.x, icell.z);

				if (pco)
					g_drawnCells++;

				// walk each cell object in cell
				while (pco)
				{
					drawObjects.append(pco);

					pco = levMapDriver1->GetNextCop(&ci);
				}
			}
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
		DrawCellObject(*drawObjects[i], cameraPos, cameraAngleY, frustrumVolume, true);
	}
}

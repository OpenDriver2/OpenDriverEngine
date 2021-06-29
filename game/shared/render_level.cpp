#include "renderer/debug_overlay.h"
#include "renderer/gl_renderer.h"

#include "core/VirtualStream.h"
#include "routines/models.h"
#include "routines/regions_d1.h"
#include "routines/regions_d2.h"
#include "routines/textures.h"

#include "render_model.h"

#include "math/Volume.h"
#include "math/convert.h"

// extern some vars
extern String					g_levname;
extern String					g_levname_moddir;
extern String					g_levname_texdir;
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

extern FILE* g_levFile;

extern bool g_nightMode;
extern bool g_displayCollisionBoxes;
extern bool g_displayHeightMap;
extern bool g_displayAllCellLevels;
extern bool g_noLod;
extern int g_cellsDrawDistance;

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

	if (g_noLod)
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

struct PCO_PAIR_D2
{
	PACKED_CELL_OBJECT* pco;
	XZPAIR nearCell;
};

//-------------------------------------------------------
// Draws Driver 2 level region cells
// and spools the world if needed
//-------------------------------------------------------
void DrawLevelDriver2(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume)
{
	int i = g_cellsDrawDistance;
	int vloop = 0;
	int hloop = 0;
	int dir = 0;
	XZPAIR cell;
	XZPAIR icell;

	g_drawnCells = 0;
	g_drawnModels = 0;
	g_drawnPolygons = 0;

	VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);

	CDriver2LevelMap* levMapDriver2 = (CDriver2LevelMap*)g_levMap;
	CFileStream spoolStream(g_levFile);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &spoolStream;
	spoolContext.lumpInfo = &g_levInfo;
	spoolContext.models = &g_levModels;
	spoolContext.textures = &g_levTextures;

	levMapDriver2->WorldPositionToCellXZ(cell, cameraPosition);

	/*
	int cameraAngleY = g_cameraAngles.y * (4096.0f / 360.0f);
	int FrAng = 512;

	// setup planes
	int backPlane = 6144;
	int rightPlane = -6144;
	int leftPlane = 6144;

	int farClipLimit = 280000;

	int rightAng = cameraAngleY - FrAng & 0xfff;
	int leftAng = cameraAngleY + FrAng & 0xfff;
	int backAng = cameraAngleY + 1024 & 0xfff;

	int rightcos = icos(rightAng);
	int rightsin = isin(rightAng);

	int leftcos = icos(leftAng);
	int leftsin = isin(leftAng);
	int backcos = icos(backAng);
	int backsin = isin(backAng);
	*/

	static Array<PCO_PAIR_D2> drawObjects;
	drawObjects.reserve(g_cellsDrawDistance * 2);
	drawObjects.clear();

	// walk through all cells
	while (i >= 0)
	{
		if (abs(hloop) + abs(vloop) < 256)
		{
			// clamped vis values
			int vis_h = MIN(MAX(hloop, -9), 10);
			int vis_v = MIN(MAX(vloop, -9), 10);

			icell.x = cell.x + hloop;
			icell.z = cell.z + vloop;

			if ( //rightPlane < 0 &&
				//leftPlane > 0 &&
				//backPlane < farClipLimit &&  // check planes
				icell.x > -1 && icell.x < levMapDriver2->GetCellsAcross() &&
				icell.z > -1 && icell.z < levMapDriver2->GetCellsDown())
			{
				CELL_ITERATOR_D2 ci;
				PACKED_CELL_OBJECT* ppco;

				levMapDriver2->SpoolRegion(spoolContext, icell);

				ppco = levMapDriver2->GetFirstPackedCop(&ci, icell);

				if (ppco)
					g_drawnCells++;

				// walk each cell object in cell
				while (ppco)
				{
					if (ci.listType != 0 && !g_displayAllCellLevels)
						break;
					
					PCO_PAIR_D2 pair;
					pair.nearCell = ci.nearCell;
					pair.pco = ppco;

					drawObjects.append(pair);

					ppco = levMapDriver2->GetNextPackedCop(&ci);
				}
			}
		}

		if (dir == 0)
		{
			//leftPlane += leftcos;
			//backPlane += backcos;
			//rightPlane += rightcos;

			hloop++;

			if (hloop + vloop == 1)
				dir = 1;
		}
		else if (dir == 1)
		{
			//leftPlane += leftsin;
			//backPlane += backsin;
			//rightPlane += rightsin;

			vloop++;

			if (hloop == vloop)
				dir = 2;
		}
		else if (dir == 2)
		{
			//leftPlane -= leftcos;
			//backPlane -= backcos;
			//rightPlane -= rightcos;

			hloop--;

			if (hloop + vloop == 0)
				dir = 3;
		}
		else
		{
			//leftPlane -= leftsin;
			//backPlane -= backsin;
			//rightPlane -= rightsin;

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
		CELL_OBJECT co;
		CDriver2LevelMap::UnpackCellObject(co, drawObjects[i].pco, drawObjects[i].nearCell);

		if (co.type >= MAX_MODELS)
		{
			// WHAT THE FUCK?
			continue;
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

		if (isGround && g_nightMode)
			CRenderModel::SetupLightingProperties(0.35f, 0.0f);
		else
			CRenderModel::SetupLightingProperties(1.0f, 1.0f);

		CRenderModel* renderModel = (CRenderModel*)ref->userData;

		if (renderModel)
		{
			const float boundSphere = ref->model->bounding_sphere * RENDER_SCALING * 2.0f;
			if (frustrumVolume.IsSphereInside(absCellPosition, boundSphere))
			{
				renderModel->Draw();
				g_drawnModels++;
				g_drawnPolygons += ref->model->num_polys;

				if (g_displayCollisionBoxes)
					CRenderModel::DrawModelCollisionBox(ref, co.pos, co.yang);
			}
		}
	}
}

//-------------------------------------------------------
// Draws Driver 2 level region cells
// and spools the world if needed
//-------------------------------------------------------
void DrawLevelDriver1(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume)
{
	CELL_ITERATOR_D1 ci;
	CELL_OBJECT* pco;

	int i = g_cellsDrawDistance;
	int vloop = 0;
	int hloop = 0;
	int dir = 0;
	XZPAIR cell;
	XZPAIR icell;

	g_drawnCells = 0;
	g_drawnModels = 0;
	g_drawnPolygons = 0;

	VECTOR_NOPAD cameraPosition = ToFixedVector(cameraPos);

	CDriver1LevelMap* levMapDriver1 = (CDriver1LevelMap*)g_levMap;
	CFileStream spoolStream(g_levFile);

	SPOOL_CONTEXT spoolContext;
	spoolContext.dataStream = &spoolStream;
	spoolContext.lumpInfo = &g_levInfo;
	spoolContext.models = &g_levModels;
	spoolContext.textures = &g_levTextures;

	levMapDriver1->WorldPositionToCellXZ(cell, cameraPosition);

	static Array<CELL_OBJECT*> drawObjects;
	drawObjects.reserve(g_cellsDrawDistance * 2);
	drawObjects.clear();

	// walk through all cells
	while (i >= 0)
	{
		if (abs(hloop) + abs(vloop) < 256)
		{
			// clamped vis values
			int vis_h = MIN(MAX(hloop, -9), 10);
			int vis_v = MIN(MAX(vloop, -9), 10);

			icell.x = cell.x + hloop;
			icell.z = cell.z + vloop;


			if (icell.x > -1 && icell.x < levMapDriver1->GetCellsAcross() &&
				icell.z > -1 && icell.z < levMapDriver1->GetCellsDown())
			{
				levMapDriver1->SpoolRegion(spoolContext, icell);

				pco = levMapDriver1->GetFirstCop(&ci, icell.x, icell.z);

				if(pco)
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

	for (uint i = 0; i < drawObjects.size(); i++)
	{
		pco = drawObjects[i];

		if (pco->type >= MAX_MODELS)
		{
			// WHAT THE FUCK?
			continue;
		}

		Vector3D absCellPosition = FromFixedVector(pco->pos);
		absCellPosition.y *= -1.0f;
		
		float distanceFromCamera = lengthSqr(absCellPosition - cameraPos);

		ModelRef_t* ref = GetModelCheckLods(pco->type, distanceFromCamera);
		MODEL* model = ref->model;

		float cellRotationRad = -pco->yang / 64.0f * PI_F * 2.0f;

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

		if (g_nightMode)
			CRenderModel::SetupLightingProperties(0.25f, 0.0f);
		else
			CRenderModel::SetupLightingProperties(0.55f, 0.55f);

		CRenderModel* renderModel = (CRenderModel*)ref->userData;

		if (renderModel)
		{
			const float boundSphere = ref->model->bounding_sphere * RENDER_SCALING * 2.0f;
			if (frustrumVolume.IsSphereInside(absCellPosition, boundSphere))
			{
				renderModel->Draw();
				g_drawnModels++;
				g_drawnPolygons += ref->model->num_polys;

				if (g_displayCollisionBoxes)
					CRenderModel::DrawModelCollisionBox(ref, pco->pos, pco->yang);
			}
		}
	}
}
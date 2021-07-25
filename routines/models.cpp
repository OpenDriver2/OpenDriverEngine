#include "core/cmdlib.h"
#include "core/IVirtualStream.h"

#include <malloc.h>
#include <nstd/HashSet.hpp>

#include "models.h"

#include <string.h>

//--------------------------------------------------------------------------------

CDriverLevelModels::CDriverLevelModels()
{

}

CDriverLevelModels::~CDriverLevelModels()
{
}

void CDriverLevelModels::FreeAll()
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		ModelRef_t& ref = m_levelModels[i];

		OnModelFreed(&ref);
		
		if (ref.model)
			free(ref.model);
	}

	for (int i = 0; i < MAX_CAR_MODELS; i++)
	{
		CarModelData_t& carModelData = m_carModels[i];

		OnCarModelFreed(&carModelData);
		
		if (carModelData.cleanmodel)
			free(carModelData.cleanmodel);

		if (carModelData.dammodel)
			free(carModelData.dammodel);

		if (carModelData.lowmodel)
			free(carModelData.lowmodel);
	}

	m_model_names.clear();
}

ModelRef_t* CDriverLevelModels::GetModelByIndex(int nIndex) const
{
	if (nIndex >= 0 && nIndex < MAX_MODELS)
		return (ModelRef_t*)&m_levelModels[nIndex];

	return nullptr;
}

int CDriverLevelModels::FindModelIndexByName(const char* name) const
{
	for (usize i = 0; i < MAX_MODELS && i < m_model_names.size(); i++)
	{
		if (!stricmp(m_model_names[i], name))
			return i;
	}

	return -1;
}

const char* CDriverLevelModels::GetModelNameByIndex(int nIndex) const
{
	if (nIndex >= m_model_names.size())
		return nullptr;
	
	return m_model_names[nIndex];
}

CarModelData_t* CDriverLevelModels::GetCarModel(int index) const
{
	return (CarModelData_t*)&m_carModels[index];
}

//-------------------------------------------------------------
// parses model lumps and exports models to OBJ
//-------------------------------------------------------------
void CDriverLevelModels::LoadCarModelsLump(IVirtualStream* pFile, int size)
{
	int modelCount;
	pFile->Read(&modelCount, sizeof(int), 1);

	DevMsg(SPEW_NORM, "	all car models count: %d\n", modelCount);

	// read entries
	carmodelentry_t model_entries[MAX_CAR_MODELS];
	pFile->Read(&model_entries, sizeof(carmodelentry_t), MAX_CAR_MODELS);

	// position
	int r_ofs = pFile->Tell();

	int pad; // really padding?
	pFile->Read(&pad, sizeof(int), 1);

	// load car models
	for (int i = 0; i < MAX_CAR_MODELS; i++)
	{
		DevMsg(SPEW_NORM, "car model: %d %d %d\n", model_entries[i].cleanOffset != -1, model_entries[i].damOffset != -1, model_entries[i].lowOffset != -1);

		CarModelData_t& carModelData = m_carModels[i];
		
		if (model_entries[i].cleanOffset != -1)
		{
			pFile->Seek(r_ofs + model_entries[i].cleanOffset, VS_SEEK_SET);

			pFile->Read(&carModelData.cleanSize, 1, sizeof(int));

			carModelData.cleanmodel = (MODEL*)malloc(carModelData.cleanSize);
			pFile->Read(carModelData.cleanmodel, 1, carModelData.cleanSize);
		}
		else
			carModelData.cleanmodel = nullptr;

		if (model_entries[i].damOffset != -1)
		{
			pFile->Seek(r_ofs + model_entries[i].damOffset, VS_SEEK_SET);

			pFile->Read(&carModelData.damSize, 1, sizeof(int));

			carModelData.dammodel = (MODEL*)malloc(carModelData.damSize);
			pFile->Read(carModelData.dammodel, 1, carModelData.damSize);
		}
		else
			carModelData.dammodel = nullptr;

		if (model_entries[i].lowOffset != -1)
		{
			pFile->Seek(r_ofs + model_entries[i].lowOffset, VS_SEEK_SET);

			pFile->Read(&carModelData.lowSize, 1, sizeof(int));

			carModelData.lowmodel = (MODEL*)malloc(carModelData.lowSize);
			pFile->Read(carModelData.lowmodel, 1, carModelData.lowSize);
		}
		else
			carModelData.lowmodel = nullptr;

		OnCarModelLoaded(&carModelData);
	}
}

//-------------------------------------------------------------
// load model names
//-------------------------------------------------------------
void CDriverLevelModels::LoadModelNamesLump(IVirtualStream* pFile, int size)
{
	char* modelnames = new char[size];
	pFile->Read(modelnames, size, 1);

	int len = strlen(modelnames);
	int sz = 0;

	do
	{
		char* str = modelnames + sz;

		len = strlen(str);

		m_model_names.append(String::fromCString(str));

		sz += len + 1;
	} while (sz < size);

	delete[] modelnames;
}

//-------------------------------------------------------------
// loads low detail tables
//-------------------------------------------------------------
void CDriverLevelModels::LoadLowDetailTableLump(IVirtualStream* pFile, int size)
{
	// read high detail table
	for (int i = 0; i < m_numModelsInPack; i++)
	{
		ushort mappedId;
		pFile->Read(&mappedId, 1, sizeof(ushort));

		ModelRef_t& ref = m_levelModels[i];
		ref.highDetailId = mappedId;
	}

	// read low detail table
	for (int i = 0; i < m_numModelsInPack; i++)
	{
		ushort mappedId;
		pFile->Read(&mappedId, 1, sizeof(ushort));

		ModelRef_t& ref = m_levelModels[i];
		ref.lowDetailId = mappedId;
	}
}

//-------------------------------------------------------------
// loads level models lump
//-------------------------------------------------------------
void CDriverLevelModels::LoadLevelModelsLump(IVirtualStream* pFile)
{
	// initialize to dummies
	for (int i = 0; i < MAX_MODELS; i++)
	{
		memset(&m_levelModels[i], 0, sizeof(ModelRef_t));
		m_levelModels[i].index = i;
		m_levelModels[i].highDetailId = 0xFFFF;
		m_levelModels[i].lowDetailId = 0xFFFF;
	}

	// read from file
	int modelCount;
	pFile->Read(&modelCount, sizeof(int), 1);

	DevMsg(SPEW_INFO, "	models in pack: %d\n", modelCount);

	m_numModelsInPack = modelCount;

	// load from pack
	for (int i = 0; i < modelCount; i++)
	{
		int modelSize;

		pFile->Read(&modelSize, sizeof(int), 1);

		if (modelSize > 0)
		{
			ModelRef_t& ref = m_levelModels[i];
			ref.index = i;
			ref.model = (MODEL*)malloc(modelSize);
			ref.size = modelSize;

			pFile->Read(ref.model, modelSize, 1);
		}
		else // leave empty as swap
		{
			ModelRef_t& ref = m_levelModels[i];
			ref.index = i;
			ref.model = nullptr;
			ref.size = modelSize;
		}
	}

	for (int i = 0; i < modelCount; i++)
	{
		// init name
		ModelRef_t& ref = m_levelModels[i];
		ref.name = GetModelNameByIndex(i);
		
		OnModelLoaded(&m_levelModels[i]);
	}
}

void CDriverLevelModels::SetModelLoadingCallbacks(OnModelLoaded_t onLoaded, OnModelFreed_t onFreed)
{
	m_onModelLoaded = onLoaded;
	m_onModelFreed = onFreed;
}

void CDriverLevelModels::SetCarModelLoadingCallbacks(OnCarModelLoaded_t onLoaded, OnCarModelFreed_t onFreed)
{
	m_onCarModelLoaded = onLoaded;
	m_onCarModelFreed = onFreed;
}

void CDriverLevelModels::OnModelLoaded(ModelRef_t* ref)
{
	// also process here instances
	if (ref->model)
	{
		// resolve base instance ref
		if (ref->model->instance_number != -1)
		{
			ref->baseInstance = GetModelByIndex(ref->model->instance_number);

			if (!ref->baseInstance)
			{
				MsgWarning("Vertex ref %d not found for %d!\n", ref->model->instance_number, ref->index);
				return;
			}
		}

		if (m_onModelLoaded)
			m_onModelLoaded(ref);
	}
}

void CDriverLevelModels::OnModelFreed(ModelRef_t* ref)
{
	if (m_onModelFreed)
		m_onModelFreed(ref);
}

void CDriverLevelModels::OnCarModelLoaded(CarModelData_t* data)
{
	if (m_onCarModelLoaded)
		m_onCarModelLoaded(data);
}

void CDriverLevelModels::OnCarModelFreed(CarModelData_t* data)
{
	if (m_onCarModelFreed)
		m_onCarModelFreed(data);
}

//--------------------------------------------------------------------------------

// From Car Poly code of D2
int PolySizes[56] = {
	8,12,16,24,20,20,28,32,8,12,16,16,
	0,0,0,0,12,12,12,16,20,20,24,24,
	0,0,0,0,0,0,0,0,8,12,16,24,
	20,20,28,32,0,0,0,0,0,0,0,0,
	12,12,12,16,20,20,24,24
};

HashSet<int> g_UnknownPolyTypes;

void PrintUnknownPolys()
{
	if (!g_UnknownPolyTypes.size())
		return;

	MsgError("Unknown polygons: \n");
	
	HashSet<int>::Iterator itr;
	for (itr = g_UnknownPolyTypes.begin(); itr != g_UnknownPolyTypes.end(); ++itr)
	{
		MsgWarning("%d (sz=%d), ", *itr, PolySizes[*itr]);
	}
	MsgError("\n\n");
	g_UnknownPolyTypes.clear();
}

// 5 (sz=20), 11 (sz=16), 10 (sz=16), 17 (sz=12), 9 (sz=12), 16 (sz=12), 8 (sz=8),

template <typename T>
void SwapValues(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

// returns size of face and fills dface_t struct
// TODO: rework, few variants of faces still looks bad
int decode_poly(const char* polyList, dpoly_t* out, int forceType /*= -1*/)
{
	int ptype = forceType == -1 ? (*polyList & 0x1f) : forceType;

	out->page = 0xFF;
	out->detail = 0xFF;
	out->flags = 0;

	*(uint*)&out->color = 0;

	switch (ptype)
	{
		case 1:
			// what a strange face type. Hardcoded?
			*(uint*)out->vindices = *(uint*)&polyList[3];
			break;
		case 0:
		case 8:
		case 16:
		case 18:
		{
			// F3
			*(uint*)out->vindices = *(uint*)&polyList[1];
			*(uint*)&out->color = *(uint*)&polyList[8];
			// FIXME: read colours

			out->flags = FACE_RGB; // RGB?
			break;
		}
		case 19:
		{
			// F4
			*(uint*)out->vindices = *(uint*)&polyList[1];
			*(uint*)&out->color = *(uint*)&polyList[8];
			
			// FIXME: read colours

			out->flags = FACE_RGB; // RGB?

			break;
		}
		case 4:
		case 10:
		case 20:
		{
			POLYFT3* pft3 = (POLYFT3*)polyList;

			*(uint*)out->vindices = *(uint*)&pft3->v0;
			*(ushort*)out->uv[0] = *(ushort*)&pft3->uv0;
			*(ushort*)out->uv[1] = *(ushort*)&pft3->uv1;
			*(ushort*)out->uv[2] = *(ushort*)&pft3->uv2;

			if(ptype != 10)
				*(uint*)&out->color = *(uint*)&pft3->color;

			out->page = pft3->texture_set;
			out->detail = pft3->texture_id;

			out->flags = FACE_TEXTURED;

			break;
		}
		case 5:
		case 9:
		case 11:
		case 17:
		case 21:
		{
			POLYFT4* pft4 = (POLYFT4*)polyList;

			*(uint*)out->vindices = *(uint*)&pft4->v0;
			*(ushort*)out->uv[0] = *(ushort*)&pft4->uv0;
			*(ushort*)out->uv[1] = *(ushort*)&pft4->uv1;
			*(ushort*)out->uv[2] = *(ushort*)&pft4->uv2;
			*(ushort*)out->uv[3] = *(ushort*)&pft4->uv3;
			
			if(ptype != 11)
				*(uint*)&out->color = *(uint*)&pft4->color;
			
			out->page = pft4->texture_set;
			out->detail = pft4->texture_id;

			//SwapValues(out->uv[2], out->uv[3]);

			out->flags = FACE_IS_QUAD | FACE_TEXTURED;

			break;
		}
		case 22:
		{
			POLYGT3* pgt3 = (POLYGT3*)polyList;

			*(uint*)out->vindices = *(uint*)&pgt3->v0;
			*(uint*)out->nindices = *(uint*)&pgt3->n0;
			*(ushort*)out->uv[0] = *(ushort*)&pgt3->uv0;
			*(ushort*)out->uv[1] = *(ushort*)&pgt3->uv1;
			*(ushort*)out->uv[2] = *(ushort*)&pgt3->uv2;

			*(uint*)&out->color = *(uint*)&pgt3->color;
			
			out->page = pgt3->texture_set;
			out->detail = pgt3->texture_id;

			out->flags = FACE_VERT_NORMAL | FACE_TEXTURED;
			break;
		}
		case 7:
		case 23:
		{
			POLYGT4* pgt4 = (POLYGT4*)polyList;

			*(uint*)out->vindices = *(uint*)&pgt4->v0;
			*(uint*)out->nindices = *(uint*)&pgt4->n0;
			*(ushort*)out->uv[0] = *(ushort*)&pgt4->uv0;
			*(ushort*)out->uv[1] = *(ushort*)&pgt4->uv1;
			*(ushort*)out->uv[2] = *(ushort*)&pgt4->uv2;
			*(ushort*)out->uv[3] = *(ushort*)&pgt4->uv3;

			*(uint*)&out->color = *(uint*)&pgt4->color;
			
			out->page = pgt4->texture_set;
			out->detail = pgt4->texture_id;
			out->flags = FACE_IS_QUAD | FACE_VERT_NORMAL | FACE_TEXTURED;
			break;
		}
		default:
		{
			g_UnknownPolyTypes.append(ptype);
		}
	}
	
	return PolySizes[*polyList & 0x1f];
}
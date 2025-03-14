#include "core/core_common.h"
#include "models.h"

//--------------------------------------------------------------------------------

CDriverLevelModels::CDriverLevelModels()
{

}

CDriverLevelModels::~CDriverLevelModels()
{
}

void CDriverLevelModels::FreeAll()
{
	for (ModelRef_t& ref : m_levelModels)
	{
		OnModelFreed(&ref);
		if (ref.model)
			PPFree(ref.model);
	}

	for (CarModelData_t& carModelData : m_carModels)
	{
		OnCarModelFreed(&carModelData);
		
		if (carModelData.cleanmodel)
			PPFree(carModelData.cleanmodel);

		if (carModelData.dammodel)
			PPFree(carModelData.dammodel);

		if (carModelData.lowmodel)
			PPFree(carModelData.lowmodel);
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
	for (int i = 0; i < MAX_MODELS && i < m_model_names.numElem(); i++)
	{
		if (!stricmp(m_model_names[i], name))
			return i;
	}

	return -1;
}

const char* CDriverLevelModels::GetModelNameByIndex(int nIndex) const
{
	if (nIndex >= m_model_names.numElem())
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

	if (!modelCount)
		return;

	DevMsg(SPEW_NORM, "	all car models count: %d\n", modelCount);

	// read entries
	carmodelentry_t model_entries[MAX_CAR_MODELS];
	pFile->Read(&model_entries, sizeof(carmodelentry_t), MAX_CAR_MODELS);

	// a kind of validation for older formats since they have duplicate car model lumps
	if ((uint)model_entries[0].cleanOffset > 100000)
		return;

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

			carModelData.cleanmodel = (MODEL*)PPAlloc(carModelData.cleanSize);
			pFile->Read(carModelData.cleanmodel, 1, carModelData.cleanSize);
		}
		else
			carModelData.cleanmodel = nullptr;

		if (model_entries[i].damOffset != -1)
		{
			pFile->Seek(r_ofs + model_entries[i].damOffset, VS_SEEK_SET);

			pFile->Read(&carModelData.damSize, 1, sizeof(int));

			carModelData.dammodel = (MODEL*)PPAlloc(carModelData.damSize);
			pFile->Read(carModelData.dammodel, 1, carModelData.damSize);
		}
		else
			carModelData.dammodel = nullptr;

		if (model_entries[i].lowOffset != -1)
		{
			pFile->Seek(r_ofs + model_entries[i].lowOffset, VS_SEEK_SET);

			pFile->Read(&carModelData.lowSize, 1, sizeof(int));

			carModelData.lowmodel = (MODEL*)PPAlloc(carModelData.lowSize);
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

	int len = CString::Length(modelnames);
	int sz = 0;

	do
	{
		char* str = modelnames + sz;
		len = CString::Length(str);
		m_model_names.append(str);

		sz += len + 1;
	} while (sz < size);

	// assign model names to existing models if they were loaded
	// (for older D1 LEV versions)

	for (int i = 0; i < m_numModelsInPack; i++)
	{
		ushort mappedId;
		pFile->Read(&mappedId, 1, sizeof(ushort));

		ModelRef_t& ref = m_levelModels[i];
		ref.name = GetModelNameByIndex(i);
	}

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
		m_levelModels[i] = ModelRef_t();
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
			ref.model = (MODEL*)PPAlloc(modelSize);
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
static const int PolySizes[56] = {
	8,12,16,24,20,20,28,32,8,12,16,16,
	0,0,0,0,12,12,12,16,20,20,24,24,
	0,0,0,0,0,0,0,0,8,12,16,24,
	20,20,28,32,0,0,0,0,0,0,0,0,
	12,12,12,16,20,20,24,24
};

static Set<int> g_UnknownPolyTypes(PP_SL);

void PrintUnknownPolys()
{
	if (!g_UnknownPolyTypes.size())
		return;

	MsgError("Unknown polygons: \n");
	
	for (auto itr = g_UnknownPolyTypes.begin(); !itr.atEnd(); ++itr)
	{
		MsgWarning("%d (sz=%d), ", itr.key(), PolySizes[itr.key()]);
	}
	MsgError("\n\n");
	g_UnknownPolyTypes.clear();
}

// returns size of face and fills dface_t struct
// TODO: rework, few variants of faces still looks bad
int decode_poly(const char* polyList, dpoly_t* out, int forceType /*= -1*/)
{
	memset(out, 0, sizeof(dpoly_t));
	int polyType = forceType == -1 ? *polyList : forceType;
	int ptype = polyType & 31;
	int extFlags = (polyType >> 5) & 7;

	out->type = ptype;
	out->page = 0xFF;
	out->detail = 0xFF;
	out->flags = 0;
	out->color = CVECTOR{ 255, 0, 255, 0 };

	// TODO: D1 and D2 to have different decoding routines

	switch (ptype)
	{
		case 1:
			// what a strange face type. Hardcoded?
			*(uint*)out->vindices = *(uint*)&polyList[4];
			*(uint*)&out->color = *(uint*)&polyList[8];
			out->flags = FACE_IS_QUAD | FACE_RGB;
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
			*(uint*)out->vindices = *(uint*)&polyList[4];
			*(uint*)out->uv = *(uint*)&polyList[8];
			*(uint*)&out->color = *(uint*)&polyList[12];
			
			// FIXME: read colours

			out->flags = FACE_RGB | FACE_IS_QUAD; // RGB?

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
		case 7:
		case 9:
		case 11:
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

			out->flags = FACE_IS_QUAD | FACE_TEXTURED;

			break;
		}
		case 17:
		{
			// F4
			out->page = polyList[1];
			out->detail = polyList[2];

			*(uint*)out->vindices = *(uint*)&polyList[4];
			*(uint*)&out->color = *(uint*)&polyList[8];

			out->flags = FACE_IS_QUAD | FACE_RGB; // RGB?

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
			g_UnknownPolyTypes.insert(ptype);
		}
	}

	// triangles are hacked to be quads for PSX. We don't need that
	if (out->flags && FACE_IS_QUAD && out->vindices[2] == out->vindices[3])
	{
		out->flags &= ~FACE_IS_QUAD;
	}

	if (out->page == 255 && (out->flags & FACE_TEXTURED))
	{
		out->flags &= ~FACE_TEXTURED;
		out->flags |= FACE_RGB;
		out->color.pad = 128;
	}
	
	return PolySizes[*polyList & 31];
}
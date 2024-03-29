#include "game/pch.h"
#include "render_model.h"
#include "render_cars.h"

#define MODEL_VERTEX_SHADER \
	"	attribute vec4 a_position_tu;\n"\
	"	attribute vec4 a_normal_tv;\n"\
	"	attribute vec4 a_color;\n"\
	"	uniform mat4 u_View;\n"\
	"	uniform mat4 u_Projection;\n"\
	"	uniform mat4 u_World;\n"\
	"	uniform mat4 u_WorldViewProj;\n"\
	"	uniform vec4 u_fogParams;\n"\
	"	void main() {\n"\
	"		v_texcoord.xy = vec2(a_position_tu.w, 1.0-a_normal_tv.w);\n"\
	"		v_normal = mat3(u_World) * a_normal_tv.xyz;\n"\
	"		v_color = a_color;\n"\
	"		gl_Position = u_WorldViewProj * vec4(a_position_tu.xyz, 1.0);\n"\
	"		float fogHeight = 1.0 - saturate((u_World * vec4(a_position_tu.xyz, 1.0)).y * u_fogParams.z);\n"\
	"		float fogDistance = saturate((u_fogParams.x - gl_Position.z) * u_fogParams.y);\n"\
	"		v_texcoord.z = (1.0 - fogDistance) * fogHeight;\n"\
	"	}\n"

#define MODEL_FRAGMENT_SHADER \
	"	uniform sampler2D s_texture;\n"\
	"	uniform vec3 u_lightDir;\n"\
	"	uniform vec4 u_ambientColor;\n"\
	"	uniform vec4 u_lightColor;\n"\
	"	uniform vec4 u_fogColor;\n"\
	"	void main() {\n"\
	"		vec4 lighting;\n"\
	"		vec4 color = texture2D(s_texture, v_texcoord.xy);\n"\
	"		if(color.a <= 0.5f) discard;\n"\
	"		color.rgb *= v_color.rgb;\n"\
	"		lighting = vec4(color.rgb * u_ambientColor.rgb * u_ambientColor.a, color.a * v_color.a);\n"\
	"		lighting.rgb += u_lightColor.rgb * u_lightColor.a * color.rgb * max(1.0 - dot(v_normal, u_lightDir), 0);\n"\
	"		fragColor = mix(lighting, u_fogColor, v_texcoord.z);\n"\
	"	}\n"

const char* model_shader =
	"varying vec3 v_texcoord;\n"
	"varying vec3 v_normal;\n"
	"varying vec4 v_color;\n"
	"#ifdef VERTEX\n"
	MODEL_VERTEX_SHADER
	"#else\n"
	MODEL_FRAGMENT_SHADER
	"#endif\n";

//-------------------------------------------------------------------

void AddExtentVertex(Vector3D& minPoint, Vector3D& maxPoint, const Vector3D& v)
{
	if (v.x < minPoint.x)
		minPoint.x = v.x;

	if (v.x > maxPoint.x)
		maxPoint.x = v.x;

	if (v.y < minPoint.y)
		minPoint.y = v.y;

	if (v.y > maxPoint.y)
		maxPoint.y = v.y;

	if (v.z < minPoint.z)
		minPoint.z = v.z;

	if (v.z > maxPoint.z)
		maxPoint.z = v.z;
}

CRenderModel::CRenderModel()
{
}

CRenderModel::~CRenderModel()
{
	Destroy();
}

// FIXME: remove this or don't use when using custom models
void VertexTileWaterOrGrassCb(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert)
{
	// HACK: move vertices slightly to get rid of Z-fighting
	vert.vy -= 0.0005f;
}

bool CRenderModel::Initialize(ModelRef_t* model)
{
	if (!model)
		return false;

	if (!model->model)
		return false;

	m_extMin = Vector3D(V_MAX_COORD);
	m_extMax = Vector3D(-V_MAX_COORD);
	
	m_sourceModel = model;

	ModelVertexCb cb = nullptr;

	// grass should be under pavements and other things
	if ((model->model->shape_flags & SHAPE_FLAG_WATER) || (model->model->flags2 & MODEL_FLAG_GRASS))
		cb = VertexTileWaterOrGrassCb;

	GenerateBuffers(nullptr, cb);

	return true;
}

void CRenderModel::Destroy()
{
	GR_DestroyVAO(m_vao);
	m_vao = nullptr;
	m_sourceModel = nullptr;
	m_batches.clear();
}

int CRenderModel::FindGrVertexIndex(const Array<vertexTuple_t>& whereFind, int flags, int vertexIndex, int normalIndex, ushort uvs)
{
	for(usize i = 0; i < whereFind.size(); i++)
	{
		if (whereFind[i].flags != flags)
			continue;
		
		if (flags & FACE_VERT_NORMAL)
		{
			if (whereFind[i].normalIndex != normalIndex)
				continue;
		}
		else
		{
			return -1;
		}

		if (flags & FACE_TEXTURED)
		{
			if (whereFind[i].uvs != uvs)
				continue;
		}

		if (whereFind[i].vertexIndex == vertexIndex)
			return whereFind[i].grVertexIndex;
	}

	return -1;
}

struct genBatch_t
{
	Array<uint16> indices;
	int tpage;
};

genBatch_t* FindBatch(Array<genBatch_t*>& batches, int tpageId)
{
	for (usize i = 0; i < batches.size(); i++)
	{
		if (batches[i]->tpage == tpageId)
			return batches[i];
	}
	return nullptr;
}

void CRenderModel::GenerateBuffers(FindVertexFn lookupFn /*= FindGrVertexIndex*/, ModelVertexCb vertexModCb /*= nullptr*/)
{
	if (!lookupFn)
		lookupFn = FindGrVertexIndex;

	Array<genBatch_t*>		batches;
	Array<GrVertex>			vertices;
	Array<vertexTuple_t>	verticesMap;
	
	MODEL* model = m_sourceModel->model;
	MODEL* vertex_ref = model;

	if (m_sourceModel->baseInstance) // car models have vertex_ref=0
	{
		vertex_ref = m_sourceModel->baseInstance->model;
	}

	genBatch_t* batch = nullptr;
	
	int modelSize = m_sourceModel->size;
	int face_ofs = 0;
	dpoly_t dec_face;

	vertices.reserve(model->num_vertices);

	// go through all polygons
	for (int i = 0; i < model->num_polys; i++)
	{
		char* facedata = model->pPolyAt(face_ofs);

		// check offset
		if ((ubyte*)facedata >= (ubyte*)model + modelSize)
		{
			MsgError("poly id=%d type=%d ofs=%d bad offset!\n", i, *facedata & 31, model->poly_block + face_ofs);
			break;
		}

		int forcePolyType = -1;

		// [A] HACK: is sky? force POLYFT4. This fixes VEGAS skies
		if (vertexModCb && m_sourceModel->index < 4)
			forcePolyType = 21;

		int poly_size = decode_poly(facedata, &dec_face, forcePolyType);

		// check poly size
		if (poly_size == 0)
		{
			MsgError("poly id=%d type=%d ofs=%d zero size!\n", i, *facedata & 31, model->poly_block + face_ofs);
			break;
		}

		face_ofs += poly_size;

		int numPolyVerts = (dec_face.flags & FACE_IS_QUAD) ? 4 : 3;
		bool bad_face = false;
		bool bad_normals = false;

		// perform vertex checks
		for (int v = 0; v < numPolyVerts; v++)
		{
			if (dec_face.vindices[v] >= vertex_ref->num_vertices)
			{
				bad_face = true;
				break;
			}

			// also check normals
			if (dec_face.flags & FACE_VERT_NORMAL)
			{
				if (dec_face.nindices[v] >= vertex_ref->num_point_normals)
				{
					bad_normals = true;
					break;
				}
			}
		}

		if (bad_face)
		{
			MsgError("poly id=%d type=%d ofs=%d has invalid indices (or format is unknown)\n", i, *facedata & 31, model->poly_block + face_ofs);
			
			continue;
		}

		// find or create new batch
		int tpageId = (dec_face.flags & FACE_TEXTURED) ? dec_face.page : -1;

		if (tpageId == 255)
			tpageId = -1;
		
		if(batch && batch->tpage != tpageId)
			batch = FindBatch(batches, tpageId);
		
		if (!batch)
		{
			batch = new genBatch_t;
			batch->tpage = tpageId;
	
			batches.append(batch);
		}
		
		// Gouraud-shaded poly smoothing
		bool smooth = (dec_face.flags & FACE_VERT_NORMAL);

		int faceIndices[4];

		// add vertices and generate faces
		for (int v = 0; v < numPolyVerts; v++)
		{
			// NOTE: Vertex indexes is reversed here
#define VERT_IDX v//numPolyVerts - 1 - v

			int vflags = dec_face.flags & ~(FACE_IS_QUAD | FACE_RGB);
			
			// try searching for vertex
			int index = (lookupFn)(verticesMap,
				vflags,
				dec_face.vindices[VERT_IDX], 
				dec_face.nindices[VERT_IDX], 
				*(ushort*)dec_face.uv[VERT_IDX]);

			//if (vertexModCb)
			//	index = -1;

			// add new vertex
			if (index == -1)
			{
				GrVertex newVert;
				vertexTuple_t vertMap;

				vertMap.flags = vflags;
				vertMap.normalIndex = -1;
				vertMap.vertexIndex = dec_face.vindices[VERT_IDX];
				vertMap.uvs = *(ushort*)dec_face.uv[VERT_IDX];

				// get the vertex
				SVECTOR* vert = vertex_ref->pVertex(dec_face.vindices[VERT_IDX]);
				Vector3D fVert = Vector3D(vert->vx * RENDER_SCALING, vert->vy * -RENDER_SCALING, vert->vz * RENDER_SCALING);

				(*(Vector3D*)&newVert.vx) = fVert;

				// set color
				newVert.cr = newVert.cg = newVert.cb = newVert.ca = 1.0f;

				// add bounding box stuff
				AddExtentVertex(m_extMin, m_extMax, fVert);

				if (smooth && !bad_normals)
				{
					vertMap.normalIndex = dec_face.nindices[VERT_IDX];

					SVECTOR* norm = vertex_ref->pPointNormal(vertMap.normalIndex);
					*(Vector3D*)&newVert.nx = -Vector3D(norm->vx * RENDER_SCALING, norm->vy * -RENDER_SCALING, norm->vz * RENDER_SCALING);
				}

				if (dec_face.flags & FACE_TEXTURED)
				{
					UV_INFO uv = *(UV_INFO*)dec_face.uv[VERT_IDX];

					// map to 0..1
					newVert.tc_u = ((float)uv.u + 0.5f) / TEXPAGE_SIZE_Y;
					newVert.tc_v = ((float)uv.v + 0.5f) / TEXPAGE_SIZE_Y;
				}

				if (dec_face.flags & FACE_RGB)
				{
					newVert.cr = dec_face.color.r / 255;
					newVert.cg = dec_face.color.g / 255;
					newVert.cb = dec_face.color.b / 255;
				}

				index = vertMap.grVertexIndex = vertices.size();

				// TODO: pass smooth flag modifier
				if (vertexModCb)
					(vertexModCb)(i, dec_face, v, newVert);

				vertices.append(newVert);

				// add vertex and a map
				verticesMap.append(vertMap);

				// vertices and verticesMap should be equal
				ASSERT(verticesMap.size() == vertices.size());
			}
			
			// add index
			faceIndices[v] = index;
		}

		// if not gouraud shaded we just compute face normal
		// FIXME: make it like game does?
		if(!smooth || bad_normals)
		{
			// it takes only triangle
			Vector3D v0 = *(Vector3D*)&vertices[faceIndices[0]].vx;
			Vector3D v1 = *(Vector3D*)&vertices[faceIndices[1]].vx;
			Vector3D v2 = *(Vector3D*)&vertices[faceIndices[2]].vx;
			
			Vector3D normal = normalize(cross(v2 - v1, v0 - v1));

			// set to each vertex
			for (int v = 0; v < numPolyVerts; v++)
				*(Vector3D*)&vertices[faceIndices[v]].nx = normal;
		}

		// triangulate quads
		if(numPolyVerts == 4)
		{
			batch->indices.append(faceIndices[0]);
			batch->indices.append(faceIndices[1]);
			batch->indices.append(faceIndices[2]);

			batch->indices.append(faceIndices[2]);
			batch->indices.append(faceIndices[3]);
			batch->indices.append(faceIndices[0]);
		}
		else
		{
			batch->indices.append(faceIndices[0]);
			batch->indices.append(faceIndices[1]);
			batch->indices.append(faceIndices[2]);
		}
	}

	Array<uint16> indices;

	// merge batches
	for(usize i = 0; i < batches.size(); i++)
	{	
		uint16 startIndex = indices.size();
		
		for(usize j = 0; j < batches[i]->indices.size(); j++)
			indices.append(batches[i]->indices[j]);

		modelBatch_t batch;
		batch.startIndex = startIndex;
		batch.numIndices = batches[i]->indices.size();
		batch.tpage = batches[i]->tpage;

		m_batches.append(batch);
		
		delete batches[i];
	}
	
	// if has existing one - regenerate
	if (m_vao)
		GR_DestroyVAO(m_vao);

	m_vao = GR_CreateVAO(vertices.size(), indices.size(), (GrVertex*)vertices, (uint16*)indices, 0);

	if(!m_vao)
	{
		MsgError("Cannot create Model VAO!\n");
	}
}

extern TextureID g_whiteTexture;

int CRenderModel::GetNumBatches() const
{
	return m_batches.size();
}

void CRenderModel::SetupRendering(bool setupShader, bool setupVAO)
{
	if(setupShader)
		SetupModelShader();

	if(setupVAO)
		GR_SetVAO(m_vao);
}

void CRenderModel::DrawBatch(int batchNum, bool setupTexture /*= true*/, int paletteSet /*= 0*/)
{
	modelBatch_t& batch = m_batches[batchNum];

	if (setupTexture)
	{
		// check if palette tex is not empty
		TextureID desiredTex = CWorld::GetHWTexture(batch.tpage, paletteSet);

		if (paletteSet > 0 && desiredTex == g_whiteTexture)
			desiredTex = CWorld::GetHWTexture(batch.tpage, 0);

		GR_SetTexture(desiredTex);
	}

	GR_DrawIndexed(PRIM_TRIANGLES, batch.startIndex, batch.numIndices);
}

void CRenderModel::Draw(bool fullSetup /*= true*/, int paletteSet /*= 0*/)
{
	if(fullSetup)
		SetupModelShader();

	GR_SetVAO(m_vao);
	
	for(usize i = 0; i < m_batches.size(); i++)
	{
		modelBatch_t& batch = m_batches[i];
		
		if (fullSetup)
		{
			// check if palette tex is not empty
			TextureID desiredTex = CWorld::GetHWTexture(batch.tpage, paletteSet);

			if(paletteSet > 0 && desiredTex == g_whiteTexture)
				desiredTex = CWorld::GetHWTexture(batch.tpage, 0);

			GR_SetTexture(desiredTex);
		}

		GR_DrawIndexed(PRIM_TRIANGLES, batch.startIndex, batch.numIndices);
	}
}

void CRenderModel::GetExtents(Vector3D& outMin, Vector3D& outMax) const
{
	outMin = m_extMin;
	outMax = m_extMax;
}

//--------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------
// Render model viewer
//-------------------------------------------------------
void CRenderModel::DrawModelCollisionBox(ModelRef_t* ref, const VECTOR_NOPAD& position, int rotation)
{
	if (ref->baseInstance)
		ref = ref->baseInstance;

	const float to_rad = 1.0 / 64.0f * PI_F * 2.0f;

	const float objRotationRad = -rotation * to_rad;

	Vector3D offset = FromFixedVector(position);
	offset.y *= -1.0f;
	Matrix4x4 world = translate(offset) * rotateY4(objRotationRad);

	MODEL* model = ref->model;

	// add collision box drawing
	int numcb = model->GetCollisionBoxCount();
	COLLISION_PACKET* box = model->pCollisionBox(0);

	for (int i = 0; i < numcb; i++)
	{
		float boxRotationRad = -box->yang * to_rad;

		//if(box->type == 0)
		{
			Vector3D pos(box->xpos, -box->ypos, box->zpos);
			Vector3D size(box->xsize / 2, box->ysize / 2, box->zsize / 2);

			CDebugOverlay::SetTransform(world * translate(pos / ONE_F) * rotateY4(boxRotationRad));
			CDebugOverlay::Box(-size / ONE_F, size / ONE_F, ColorRGBA(1, 1, 0, 0.5f));
		}

		box++;
	}

	CDebugOverlay::SetTransform(identity4());
}

struct ModelShaderInfo
{
	ShaderID	shader{ 0 };

	int			cid_ambientColor{ -1 };
	int			cid_lightColor{ -1 };
	int			cid_lightDir{ -1 };
	int			cid_fogParams{ -1 };
	int			cid_fogColor{ -1 };
} g_modelShader;

struct WorldRenderProperties
{
	Vector4D ambientColor;
	Vector4D lightColor;
	Vector3D lightDir;

	Vector4D fogParams;
	Vector4D fogColor;

} g_worldRenderProperties;

// compiles model shader
void CRenderModel::InitModelShader()
{
	// create shader
	g_modelShader.shader = GR_CompileShader(model_shader);

	g_modelShader.cid_ambientColor = GR_GetShaderConstantIndex(g_modelShader.shader, "u_ambientColor");
	g_modelShader.cid_lightColor = GR_GetShaderConstantIndex(g_modelShader.shader, "u_lightColor");
	g_modelShader.cid_lightDir = GR_GetShaderConstantIndex(g_modelShader.shader, "u_lightDir");

	g_modelShader.cid_fogParams = GR_GetShaderConstantIndex(g_modelShader.shader, "u_fogParams");
	g_modelShader.cid_fogColor = GR_GetShaderConstantIndex(g_modelShader.shader, "u_fogColor");
}

// prepares shader for rendering
// used for Models
void CRenderModel::SetupModelShader()
{
	GR_SetShader(g_modelShader.shader);
	GR_SetShaderConstantVector4D(g_modelShader.cid_ambientColor, g_worldRenderProperties.ambientColor);
	GR_SetShaderConstantVector4D(g_modelShader.cid_lightColor, g_worldRenderProperties.lightColor);
	GR_SetShaderConstantVector3D(g_modelShader.cid_lightDir, g_worldRenderProperties.lightDir);

	GR_SetShaderConstantVector4D(g_modelShader.cid_fogParams, g_worldRenderProperties.fogParams);
	GR_SetShaderConstantVector4D(g_modelShader.cid_fogColor, g_worldRenderProperties.fogColor);
}

extern CBaseLevelMap* g_levMap;

// sets up lighting properties
void CRenderModel::SetupLightingProperties(float ambientScale /*= 1.0f*/, float lightScale /*= 1.0f*/)
{
	const OUT_CELL_FILE_HEADER& cellHeader = g_levMap->GetMapInfo();
	
	Vector3D lightVector = normalize(FromFixedVector(cellHeader.light_source));
	float lightLevel = cellHeader.ambient_light_level / ONE_F;
	
	g_worldRenderProperties.ambientColor = ColorRGBA(CRender_Level::RenderProps.ambientColor, 0.8f * ambientScale * lightLevel);
	g_worldRenderProperties.lightColor = ColorRGBA(CRender_Level::RenderProps.lightColor, 0.8f * lightScale * lightLevel);
	g_worldRenderProperties.lightDir = lightVector * Vector3D(1,-1,1);

	const float fogNear = CRender_Level::RenderProps.fogParams.x;
	const float fogFar = CRender_Level::RenderProps.fogParams.y;
	const float fogTop = CRender_Level::RenderProps.fogParams.z;

	g_worldRenderProperties.fogParams = Vector4D(fogFar, 1.0f / (fogFar - fogNear), 1.0f / fogTop, 1.0f);
	g_worldRenderProperties.fogColor = ColorRGBA(CRender_Level::RenderProps.fogColor, 0.0f);
}


//----------------------------------------
// callbacks for model lump loader

// called when model loaded in CDriverLevelModels
void CRenderModel::OnModelLoaded(ModelRef_t* ref)
{
	if (!ref->model)
		return;

	// mangle wheel models
	if (!strcmp(ref->name, "CLEANWHEEL") ||
		!strcmp(ref->name, "FASTWHEEL") ||
		!strcmp(ref->name, "DAMWHEEL"))
	{
		CRender_Cars::MangleWheelModel(ref->model);
	}

	CRenderModel* renderModel = new CRenderModel();

	if (renderModel->Initialize(ref))
		ref->userData = renderModel;
	else
		delete renderModel;
}

// called when model freed in CDriverLevelModels
void CRenderModel::OnModelFreed(ModelRef_t* ref)
{
	CRenderModel* model = (CRenderModel*)ref->userData;

	if (model)
		model->Destroy();

	delete model;
	ref->userData = nullptr;
}
#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

#include "gl_renderer.h"

class CMeshBuilder
{
public:
	CMeshBuilder(GrVAO* mesh, int maxVerts = 8192);
	~CMeshBuilder();

	// begins the mesh
	void		Begin(GR_PrimitiveType type);

	// ends building and renders the mesh
	void		End(bool render = true);

	// position setup
	void		Position3f(float x, float y, float z);
	void		Position3fv(const Vector3D& v);
	void		Position2f(float x, float y);
	void		Position2fv(const Vector2D& v);

	// normal setup
	void		Normal3f(float nx, float ny, float nz);
	void		Normal3fv(const Vector3D& v);

	// texcoord setup
	void		TexCoord2f(float s, float t);
	void		TexCoord2fv(const Vector2D& v);
	void		TexCoord3f(float s, float t, float r);
	void		TexCoord3fv(const Vector3D& v);

	// color setup
	void		Color3f(float r, float g, float b);
	void		Color3fv(const ColorRGB& rgb);
	void		Color4f(float r, float g, float b, float a);
	void		Color4fv(const ColorRGBA& rgba);

	//
	// complex primitives (also using index buffer)
	//

	// lines
	void		Line2fv(const Vector2D& v1, const Vector2D& v2);
	void		Line3fv(const Vector3D& v1, const Vector3D& v2);

	// Makes 2D triangle
	// to set quad color use Color3*/Color4* operators
	void		Triangle2(const Vector2D& v1, const Vector2D& v2, const Vector2D& v3);

	// Makes 2D quad
	// to set quad color use Color3*/Color4* operators
	void		Quad2(const Vector2D& v_tl, const Vector2D& v_tr, const Vector2D& v_bl, const Vector2D& v_br);

	// Makes 3D quad
	// to set quad color use Color3*/Color4* operators
	void		Quad3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, const Vector3D& v4);

	// Makes 2D textured quad
	// to set quad color use Color3*/Color4* operators
	void		TexturedQuad2(const Vector2D& v_tl, const Vector2D& v_tr, const Vector2D& v_bl, const Vector2D& v_br,
		const Vector2D& t_tl, const Vector2D& t_tr, const Vector2D& t_bl, const Vector2D& t_br);

	// Makes 3D textured quad
	// to set quad color use Color3*/Color4* operators
	void		TexturedQuad3(const Vector3D& v_tl, const Vector3D& v_tr, const Vector3D& v_bl, const Vector3D& v_br,
		const Vector2D& t_tl, const Vector2D& t_tr, const Vector2D& t_bl, const Vector2D& t_br);

	// advances vertex, no indexing issued
	void		AdvanceVertex();

	// advances index with current index
	int			AdvanceVertexIndex();

	// advances index with custom index
	int			AdvanceVertexIndex(uint16 index);

protected:
	struct vertdata_t
	{
		Vector4D			value;
	};

	void		AdvanceVertexPtr();
	void		CopyVertex();

	vertdata_t			m_position;
	vertdata_t			m_normal;
	vertdata_t			m_texcoord;
	vertdata_t			m_color;

	GrVAO*				m_mesh{ nullptr };
	GrVertex*			m_verts{ nullptr };

	GrVertex*			m_curVertex{ nullptr };
	int					m_stride{ 0 };

	GR_PrimitiveType	m_primType{ PRIM_TRIANGLES };
	bool				m_pushedVert{ false };
	bool				m_begun{ false };
};

//----------------------------------------------------------

inline CMeshBuilder::CMeshBuilder(GrVAO* mesh, int maxVerts /*= 8192*/) :
	m_mesh(mesh),
	m_stride(sizeof(GrVertex))
{
	m_verts = (GrVertex*)Memory::alloc(sizeof(GrVertex)*maxVerts);
}

inline CMeshBuilder::~CMeshBuilder()
{
	Memory::free(m_verts);
}

// begins the mesh
inline void CMeshBuilder::Begin(GR_PrimitiveType type)
{
	m_primType = type;

	m_position.value = Vector4D(0, 0, 0, 1.0f);
	m_texcoord.value = vec4_zero;
	m_normal.value = Vector4D(0, 1, 0, 0);
	m_color.value = color4_white;

	m_begun = true;
	m_pushedVert = false;
}

// ends building and renders the mesh
inline void CMeshBuilder::End(bool render/* = true*/)
{
	if (m_begun && render)
	{
		int numVerts = m_curVertex - m_verts;
		GR_UpdateVAO(m_mesh, numVerts, m_verts);
		GR_SetVAO(m_mesh);
		GR_DrawNonIndexed(m_primType, 0, numVerts);
	}

	m_begun = false;
}

// position setup
inline void CMeshBuilder::Position3f(float x, float y, float z)
{
	m_position.value.x = x;
	m_position.value.y = y;
	m_position.value.z = z;
	m_position.value.w = 1.0f;
	m_pushedVert = true;
}

inline void CMeshBuilder::Position3fv(const Vector3D& v)
{
	Position3f(v[0], v[1], v[2]);
}

inline void CMeshBuilder::Position2f(float x, float y)
{
	m_position.value.x = x;
	m_position.value.y = y;
	m_position.value.z = 0.0f;
	m_position.value.w = 1.0f;
	m_pushedVert = true;
}

inline void CMeshBuilder::Position2fv(const Vector2D& v)
{
	Position2f(v[0], v[1]);
}

// normal setup
inline void CMeshBuilder::Normal3f(float nx, float ny, float nz)
{
	m_normal.value.x = nx;
	m_normal.value.y = ny;
	m_normal.value.z = nz;
	m_normal.value.w = 0.0f;
}

inline void CMeshBuilder::Normal3fv(const Vector3D& v)
{
	Normal3f(v[0], v[1], v[2]);
}

inline void CMeshBuilder::TexCoord2f(float s, float t)
{
	m_texcoord.value.x = s;
	m_texcoord.value.y = t;
	m_texcoord.value.z = 0.0f;
	m_texcoord.value.w = 0.0f;
}

inline void CMeshBuilder::TexCoord2fv(const Vector2D& v)
{
	TexCoord2f(v[0], v[1]);
}

inline void CMeshBuilder::TexCoord3f(float s, float t, float r)
{
	m_texcoord.value.x = s;
	m_texcoord.value.y = t;
	m_texcoord.value.z = r;
	m_texcoord.value.w = 0.0f;
}

inline void CMeshBuilder::TexCoord3fv(const Vector3D& v)
{
	TexCoord3f(v[0], v[1], v[2]);
}

// color setup
inline void CMeshBuilder::Color3f(float r, float g, float b)
{
	m_color.value.x = r;
	m_color.value.y = g;
	m_color.value.z = b;
	m_color.value.w = 1.0f;
}

inline void CMeshBuilder::Color3fv(const ColorRGB& rgb)
{
	Color3f(rgb[0], rgb[1], rgb[2]);
}

inline void CMeshBuilder::Color4f(float r, float g, float b, float a)
{
	m_color.value.x = r;
	m_color.value.y = g;
	m_color.value.z = b;
	m_color.value.w = a;
}

inline void CMeshBuilder::Color4fv(const ColorRGBA& rgba)
{
	Color4f(rgba[0], rgba[1], rgba[2], rgba[3]);
}


// advances vertex
inline void CMeshBuilder::AdvanceVertex()
{
	if (!m_begun)
		return;

	if (!m_pushedVert)
		return;

	m_pushedVert = false;
	
	if (!m_curVertex)
		m_curVertex = m_verts;

	CopyVertex();

	m_curVertex++;
}

// advances index with current index
inline int CMeshBuilder::AdvanceVertexIndex()
{
#if 0
	if (!m_begun)
		return -1;

	if (!m_pushedVert)
		return -1;

	m_pushedVert = false;

	uint16* inputIdx;

	if (!m_curVertex)
		m_curVertex = m_verts;

	CopyVertex();

	m_curVertex++;

	*inputIdx = curVertex;
	return curVertex;
#else
	return 0;
#endif
}

// advances index with custom index
inline int CMeshBuilder::AdvanceVertexIndex(uint16 index)
{
#if 0
	if (!m_begun)
		return -1;

	if (index == 0xFFFF)
	{
		m_mesh->AddStripBreak();
		return -1;
	}

	uint16* inputIdx;

	int curVertex = m_mesh->AllocateGeom(m_pushedVert ? 1 : 0, 1, &m_curVertex, &inputIdx, false);
	if (curVertex == -1)
	{
		m_pushedVert = false;
		return -1;
	}

	if (m_pushedVert)
	{
		CopyVertex();

		m_pushedVert = false;
	}

	*inputIdx = index;
	return curVertex;
#else
	return 0;
#endif
}

inline void CMeshBuilder::AdvanceVertexPtr()
{
	if (!m_curVertex) // first it needs to be allocated
		return;

	if (!m_pushedVert)
		return;

	m_pushedVert = false;

	CopyVertex();

	m_curVertex++;
}

//-------------------------------------------------------------------------------
// Complex primitives
//-------------------------------------------------------------------------------

// lines
inline void CMeshBuilder::Line2fv(const Vector2D& v1, const Vector2D& v2)
{
	Position2fv(v1); AdvanceVertex();
	Position2fv(v2); AdvanceVertex();
}

inline void CMeshBuilder::Line3fv(const Vector3D& v1, const Vector3D& v2)
{
	Position3fv(v1); AdvanceVertex();
	Position3fv(v2); AdvanceVertex();
}

// Makes 2D triangle
// to set quad color use Color3*/Color4* operators
inline void CMeshBuilder::Triangle2(const Vector2D& v1, const Vector2D& v2, const Vector2D& v3)
{
	// TODO: alloc indices in size of 3
	if (!m_curVertex)
		m_curVertex = m_verts;

	Position2fv(v1);
	AdvanceVertexPtr();

	Position2fv(v2);
	AdvanceVertexPtr();

	Position2fv(v3);
	AdvanceVertexPtr();
#if 0
	indices[0] = startIndex;
	indices[1] = startIndex + 1;
	indices[2] = startIndex + 2;
#endif
}

// Makes 2D quad
// to set quad color use Color3*/Color4* operators
inline void CMeshBuilder::Quad2(const Vector2D& v_tl, const Vector2D& v_tr, const Vector2D& v_bl, const Vector2D& v_br)
{
	uint16* indices = nullptr;

	int quadIndices = (m_primType == PRIM_TRIANGLES) ? 6 : 4;

	// TODO: alloc indices in size of quadIndices
	if (!m_curVertex)
		m_curVertex = m_verts;

	// top left 0
	Position2fv(v_tl);
	AdvanceVertexPtr();

	// top right 1
	Position2fv(v_tr);
	AdvanceVertexPtr();

	// bottom left 2
	Position2fv(v_bl);
	AdvanceVertexPtr();

	// bottom right 3
	Position2fv(v_br);
	AdvanceVertexPtr();

#if 0
	// make indices working
	if (m_primType == PRIM_TRIANGLES)
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;

		indices[3] = startIndex + 2;
		indices[4] = startIndex + 1;
		indices[5] = startIndex + 3;
	}
	else // more linear
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;
		indices[3] = startIndex + 3;
	}
#endif
}

// Makes textured quad
// to set quad color use Color3*/Color4* operators
inline void CMeshBuilder::TexturedQuad2(const Vector2D& v_tl, const Vector2D& v_tr, const Vector2D& v_bl, const Vector2D& v_br,
	const Vector2D& t_tl, const Vector2D& t_tr, const Vector2D& t_bl, const Vector2D& t_br)
{
	uint16* indices = nullptr;

	int quadIndices = (m_primType == PRIM_TRIANGLES) ? 6 : 4;

	// TODO: alloc indices in size of quadIndices
	if (!m_curVertex)
		m_curVertex = m_verts;

	// top left 0
	Position2fv(v_tl);
	TexCoord2fv(t_tl);
	AdvanceVertexPtr();

	// top right 1
	Position2fv(v_tr);
	TexCoord2fv(t_tr);
	AdvanceVertexPtr();

	// bottom left 2
	Position2fv(v_bl);
	TexCoord2fv(t_bl);
	AdvanceVertexPtr();

	// bottom right 3
	Position2fv(v_br);
	TexCoord2fv(t_br);
	AdvanceVertexPtr();

#if 0
	// make indices working
	if (primType == PRIM_TRIANGLES)
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;

		indices[3] = startIndex + 2;
		indices[4] = startIndex + 1;
		indices[5] = startIndex + 3;
	}
	else // more linear
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;
		indices[3] = startIndex + 3;
	}
#endif
}

// Makes textured quad
// to set quad color use Color3*/Color4* operators
inline void CMeshBuilder::Quad3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, const Vector3D& v4)
{
	int quadIndices = (m_primType == PRIM_TRIANGLES) ? 6 : 4;

	// TODO: alloc indices in size of quadIndices
	if (!m_curVertex)
		m_curVertex = m_verts;

	// top left 0
	Position3fv(v1);
	AdvanceVertexPtr();

	// top right 1
	Position3fv(v2);
	AdvanceVertexPtr();

	// bottom left 2
	Position3fv(v3);
	AdvanceVertexPtr();

	// bottom right 3
	Position3fv(v4);
	AdvanceVertexPtr();

#if 0
	// make indices working
	if (m_primType == PRIM_TRIANGLES)
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;

		indices[3] = startIndex + 2;
		indices[4] = startIndex + 1;
		indices[5] = startIndex + 3;
	}
	else // more linear
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;
		indices[3] = startIndex + 3;
	}
#endif
}

// Makes textured quad
// to set quad color use Color3*/Color4* operators
inline void CMeshBuilder::TexturedQuad3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, const Vector3D& v4,
	const Vector2D& t1, const Vector2D& t2, const Vector2D& t3, const Vector2D& t4)
{
	int quadIndices = (m_primType == PRIM_TRIANGLES) ? 6 : 4;

	// TODO: alloc indices in size of quadIndices
	if (!m_curVertex)
		m_curVertex = m_verts;

	// top left 0
	Position3fv(v1);
	TexCoord2fv(t1);
	AdvanceVertexPtr();

	// top right 1
	Position3fv(v2);
	TexCoord2fv(t2);
	AdvanceVertexPtr();

	// bottom left 2
	Position3fv(v3);
	TexCoord2fv(t3);
	AdvanceVertexPtr();

	// bottom right 3
	Position3fv(v4);
	TexCoord2fv(t4);
	AdvanceVertexPtr();

#if 0
	// make indices working
	if (m_primType == PRIM_TRIANGLES)
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;

		indices[3] = startIndex + 2;
		indices[4] = startIndex + 1;
		indices[5] = startIndex + 3;
	}
	else // more linear
	{
		indices[0] = startIndex;
		indices[1] = startIndex + 1;
		indices[2] = startIndex + 2;
		indices[3] = startIndex + 3;
	}
#endif
}

inline void CMeshBuilder::CopyVertex()
{
	memcpy(&m_curVertex->vx, &m_position.value.x, sizeof(Vector3D));
	memcpy(&m_curVertex->nx, &m_normal.value.x, sizeof(Vector3D));
	memcpy(&m_curVertex->cr, &m_color.value.x, sizeof(Vector4D));
	m_curVertex->tc_u = m_texcoord.value.x;
	m_curVertex->tc_v = m_texcoord.value.y;
}

#endif // MESH_BUILDER_H
#ifndef DRAWMODEL_H
#define DRAWMODEL_H

#include "math/Vector.h"

#define RENDER_SCALING			(1.0f / ONE_F)

struct ModelRef_t;
struct GrVAO;

struct modelBatch_t
{
	int tpage;
	uint16 startIndex;
	uint16 numIndices;
};

struct vertexTuple_t
{
	int		flags;				// store few face flags here

	short	grVertexIndex;

	short	vertexIndex;
	short	normalIndex;		// point normal index
	ushort	uvs;
};

class CRenderModel
{
public:

	typedef void		(*ModelVertexCb)(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert);
	typedef int			(*FindVertexFn)(const Array<vertexTuple_t>& whereFind, int flags, int vertexIndex, int normalIndex, ushort uvs);

						CRenderModel();
	virtual				~CRenderModel();

	bool				Initialize(ModelRef_t* model);
	void				Destroy();

	void				SetupRendering(bool setupShader, bool setupVAO);
	void				Draw(bool fullSetup = true, int paletteSet = 0);
	void				DrawBatch(int batchNum, bool setupTexture = true, int paletteSet = 0);

	int					GetNumBatches() const;

	void				GetExtents(Vector3D& outMin, Vector3D& outMax) const;

	static void			DrawModelCollisionBox(ModelRef_t* ref, const VECTOR_NOPAD& position, int rotation);
	static void			SetupModelShader();

	static void			SetupLightingProperties(float ambientScale = 1.0f, float lightScale = 1.0f);

	static void			InitModelShader();

	// calling it again will regenerate VAO.
	// Initialize calls it automatically so you don't have to (first time)
	void				GenerateBuffers(FindVertexFn lookupFn = nullptr, ModelVertexCb vertexModCb = nullptr);

	// callbacks for creating/destroying renderer objects
	static void			OnModelLoaded(ModelRef_t* ref);
	static void			OnModelFreed(ModelRef_t* ref);

protected:

	static int			FindGrVertexIndex(const Array<vertexTuple_t>& whereFind, int flags, int vertexIndex, int normalIndex, ushort uvs);

	Vector3D			m_extMin;
	Vector3D			m_extMax;

	ModelRef_t*			m_sourceModel { nullptr };
	GrVAO*				m_vao { nullptr };
	Array<modelBatch_t>	m_batches;
};

#endif
#ifndef DRAWMODEL_H
#define DRAWMODEL_H

#include "math/Vector.h"

#define RENDER_SCALING			(1.0f / ONE_F)

struct ModelRef_t;
struct GrVAO;

struct modelBatch_t
{
	int tpage;
	int startIndex;
	int numIndices;
};

typedef void (*ModelVertexCb)(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert);

class CRenderModel
{
public:
						CRenderModel();
	virtual				~CRenderModel();

	bool				Initialize(ModelRef_t* model);
	void				Destroy();

	void				Draw(bool fullSetup = true);

	void				GetExtents(Vector3D& outMin, Vector3D& outMax) const;

	static void			DrawModelCollisionBox(ModelRef_t* ref, const VECTOR_NOPAD& position, int rotation);
	static void			SetupModelShader();
	static void			SetupLightingProperties(float ambientScale = 1.0f, float lightScale = 1.0f);
	static void			InitModelShader();

	// calling it again will regenerate VAO.
	// Initialize calls it automatically so you don't have to (first time)
	void				GenerateBuffers(ModelVertexCb vertexModCb = nullptr);

	// callbacks for creating/destroying renderer objects
	static void			OnModelLoaded(ModelRef_t* ref);
	static void			OnModelFreed(ModelRef_t* ref);
	
protected:

	Vector3D			m_extMin;
	Vector3D			m_extMax;

	ModelRef_t*			m_sourceModel { nullptr };
	GrVAO*				m_vao { nullptr };
	Array<modelBatch_t>	m_batches;
	int					m_numVerts;
};

#endif
#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <glad/glad.h>
#include "core/dktypes.h"
#include "math/Matrix.h"

#define RO_DOUBLE_BUFFERED

typedef GLuint TextureID;
typedef GLuint ShaderID;
typedef GLuint RenderTargetID;

struct GrVertex
{
	float vx, vy, vz, tc_u;
	float nx, ny, nz, tc_v;
	float cr, cg, cb, ca;
};

struct GrVAO;

enum GR_ShaderAttrib
{
	a_position_tu,
	a_normal_tv,
	a_color,
};

enum GR_BlendMode
{
	BM_NONE = 0,
	BM_AVERAGE,
	BM_ADD,
	BM_SUBTRACT,
	BM_ADD_QUATER_SOURCE,
	BM_SEMITRANS_ALPHA,
};

enum GR_CullMode
{
	CULL_NONE = 0,
	CULL_FRONT,
	CULL_BACK,
};

enum GR_MatrixMode
{
	MATRIX_VIEW = 0,
	MATRIX_PROJECTION,
	MATRIX_WORLD,

	MATRIX_WORLDVIEWPROJECTION,

	MATRIX_MODES,
};

enum GR_PrimitiveType
{
	PRIM_TRIANGLES = 0,
	PRIM_TRIANGLE_FAN,
	PRIM_TRIANGLE_STRIP,
	PRIM_QUADS,
	PRIM_LINES,
	PRIM_LINE_STRIP,
	PRIM_LINE_LOOP,
	PRIM_POINTS,
};

// Reserved, used for shaders only
enum GR_ConstantType
{
	CONSTANT_FLOAT,
	CONSTANT_VECTOR2D,
	CONSTANT_VECTOR3D,
	CONSTANT_VECTOR4D,
	CONSTANT_INT,
	CONSTANT_IVECTOR2D,
	CONSTANT_IVECTOR3D,
	CONSTANT_IVECTOR4D,
	CONSTANT_BOOL,
	CONSTANT_BVECTOR2D,
	CONSTANT_BVECTOR3D,
	CONSTANT_BVECTOR4D,
	CONSTANT_MATRIX2x2,
	CONSTANT_MATRIX3x3,
	CONSTANT_MATRIX4x4,

	CONSTANT_TYPE_COUNT
};

int			GR_Init(char* windowName, int width, int height, int fullscreen);
void		GR_Shutdown();

void		GR_UpdateWindowSize(int width, int height);
void		GR_SwapWindow();

ShaderID	GR_CompileShader(const char* source);

//--------------------------------------------------

void		GR_BeginScene();
void		GR_EndScene();

void		GR_ClearColor(float r, float g, float b);
void		GR_ClearDepth(float depth);

void		GR_SetViewPort(int x, int y, int width, int height);

//--------------------------------------------------

TextureID	GR_CreateRGBATexture(int width, int height, ubyte* data = nullptr);
void		GR_DestroyTexture(TextureID texture);
void		GR_UpdateRGBATexture(TextureID texture, int width, int height, ubyte* data);

//--------------------------------------------------

GrVAO*		GR_CreateVAO(int numVertices, GrVertex* verts = nullptr, int dynamic = 0);
GrVAO*		GR_CreateVAO(int numVertices, int numIndices, GrVertex* verts = nullptr, uint16* indices = nullptr, int dynamic = 0);

void		GR_UpdateVAO(GrVAO* vaoPtr, int numVertices, GrVertex* verts);
void		GR_UpdateVAO(GrVAO* vaoPtr, int numVertices, GrVertex* verts, int numIndices, uint16* indices);

void		GR_DestroyVAO(GrVAO* vaoPtr);

//--------------------------------------------------

void		GR_SetShader(const ShaderID& shader);
int			GR_GetShaderConstantIndex(ShaderID shaderId, char* name);

void		GR_SetShaderConstantvi(int index, GR_ConstantType constantType, int count, float* value);

void		GR_SetShaderConstantFloat(int index, float value);
void		GR_SetShaderConstantVector3D(int index, const Vector3D& value);
void		GR_SetShaderConstantVector4D(int index, const Vector4D& value);
void		GR_SetShaderConstantMatrix3x3(int index, const Matrix3x3& value);
void		GR_SetShaderConstantMatrix4x4(int index, const Matrix4x4& value);

void		GR_SetVAO(GrVAO* vaoPtr);
void		GR_SetTexture(TextureID texture);
void		GR_SetMatrix(GR_MatrixMode mode, const Matrix4x4& matrix);

void		GR_SetPolygonOffset(float ofs);
void		GR_SetDepthMode(int test, int write);
void		GR_SetCullMode(GR_CullMode cullMode);
void		GR_SetBlendMode(GR_BlendMode blendMode);

void		GR_UpdateMatrixUniforms();

//--------------------------------------------------

void		GR_DrawNonIndexed(GR_PrimitiveType primitivesType, int firstVertex, int numVertices);
void		GR_DrawIndexed(GR_PrimitiveType primitivesType, int firstIndex, int numIndices);

#endif
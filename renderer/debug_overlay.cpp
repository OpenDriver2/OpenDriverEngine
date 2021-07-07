#include "gl_renderer.h"

#define LINE_VERTEX_SHADER \
	"	attribute vec4 a_position_tu;\n"\
	"	attribute vec4 a_normal_tv;\n"\
	"	attribute vec4 a_color;\n"\
	"	uniform mat4 u_View;\n"\
	"	uniform mat4 u_Projection;\n"\
	"	uniform mat4 u_World;\n"\
	"	uniform mat4 u_WorldViewProj;\n"\
	"	void main() {\n"\
	"		v_color = a_color;\n"\
	"		gl_Position = u_WorldViewProj * vec4(a_position_tu.xyz, 1.0);\n"\
	"	}\n"

#define LINE_FRAGMENT_SHADER \
	"	void main() {\n"\
	"		fragColor = v_color;\n"\
	"	}\n"

const char* line_shader =
"varying vec2 v_texcoord;\n"
"varying vec3 v_normal;\n"
"varying vec4 v_color;\n"
"#ifdef VERTEX\n"
LINE_VERTEX_SHADER
"#else\n"
LINE_FRAGMENT_SHADER
"#endif\n";

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include "debug_overlay.h"

GrVertex	g_lineBuffer[MAX_LINE_BUFFER_SIZE];
int			g_numLineVerts;

GrVAO*		g_linesVAO { nullptr };
ShaderID	g_linesShader { 0 };
Matrix4x4	g_lineTransform { identity3() };
bool		g_enableDebugOverlay{ false };

void CDebugOverlay::Lua_Init(sol::state& lua)
{
	auto engine = lua["engine"].get_or_create<sol::table>();

	auto debugOverlay = engine["DebugOverlay"].get_or_create<sol::table>();

	debugOverlay["SetTransform"] = &SetTransform;
	debugOverlay["Line"] = &Line;
	debugOverlay["Box"] = &Box;
	debugOverlay["Enable"] = &Enable;
	debugOverlay["Enable"] = &IsEnabled;
}

void CDebugOverlay::Init()
{
	g_linesVAO = GR_CreateVAO(1024, nullptr, 1);
	g_linesShader = GR_CompileShader(line_shader);
}

void CDebugOverlay::Enable(bool enable)
{
	g_enableDebugOverlay = enable;
}

bool CDebugOverlay::IsEnabled()
{
	return g_enableDebugOverlay;
}

void CDebugOverlay::Destroy()
{
	GR_DestroyVAO(g_linesVAO);
	g_linesVAO = nullptr;

	//GR_DestroyShader(g_linesShader);
}

void CDebugOverlay::SetTransform(const Matrix4x4& transform)
{
	g_lineTransform = transform;
}

void CDebugOverlay::Line(const Vector3D& posA, const Vector3D& posB, const ColorRGBA& color)
{
	if(g_numLineVerts + 2 < MAX_LINE_BUFFER_SIZE)
	{
		Vector3D rPosA = (g_lineTransform*Vector4D(posA, 1.0f)).xyz();
		Vector3D rPosB = (g_lineTransform*Vector4D(posB, 1.0f)).xyz();
		
		g_lineBuffer[g_numLineVerts++] = GrVertex{ rPosA.x, rPosA.y,rPosA.z, 0,0,0, 0,0, color.x, color.y, color.z, color.w };
		g_lineBuffer[g_numLineVerts++] = GrVertex{ rPosB.x, rPosB.y,rPosB.z, 0,0,0, 0,0, color.x, color.y, color.z, color.w };
	}
}

void CDebugOverlay::Box(const Vector3D& mins, const Vector3D& maxs, const ColorRGBA& color)
{
	Line(Vector3D(mins.x, maxs.y, mins.z),
		Vector3D(mins.x, maxs.y, maxs.z), color);

	Line(Vector3D(maxs.x, maxs.y, maxs.z),
		Vector3D(maxs.x, maxs.y, mins.z), color);

	Line(Vector3D(maxs.x, mins.y, mins.z),
		Vector3D(maxs.x, mins.y, maxs.z), color);

	Line(Vector3D(mins.x, mins.y, maxs.z),
		Vector3D(mins.x, mins.y, mins.z), color);

	Line(Vector3D(mins.x, mins.y, maxs.z),
		Vector3D(mins.x, maxs.y, maxs.z), color);

	Line(Vector3D(maxs.x, mins.y, maxs.z),
		Vector3D(maxs.x, maxs.y, maxs.z), color);

	Line(Vector3D(mins.x, mins.y, mins.z),
		Vector3D(mins.x, maxs.y, mins.z), color);

	Line(Vector3D(maxs.x, mins.y, mins.z),
		Vector3D(maxs.x, maxs.y, mins.z), color);

	Line(Vector3D(mins.x, maxs.y, mins.z),
		Vector3D(maxs.x, maxs.y, mins.z), color);

	Line(Vector3D(mins.x, maxs.y, maxs.z),
		Vector3D(maxs.x, maxs.y, maxs.z), color);

	Line(Vector3D(mins.x, mins.y, mins.z), 
		Vector3D(maxs.x, mins.y, mins.z), color);

	Line(Vector3D(mins.x, mins.y, maxs.z),
		Vector3D(maxs.x, mins.y, maxs.z), color);
}

void CDebugOverlay::Draw()
{
	if (g_numLineVerts == 0)
		return;

	GR_UpdateVAO(g_linesVAO, g_numLineVerts, g_lineBuffer);
	
	GR_SetShader(g_linesShader);
	GR_SetVAO(g_linesVAO);

	GR_SetMatrix(MATRIX_WORLD, identity4());
	GR_UpdateMatrixUniforms();
	
	GR_SetCullMode(CULL_NONE);
	GR_SetBlendMode(BM_SEMITRANS_ALPHA);
	GR_SetDepth(0);

	GR_DrawNonIndexed(PRIM_LINES, 0, g_numLineVerts);

	g_numLineVerts = 0;
	g_lineTransform = identity3();
}
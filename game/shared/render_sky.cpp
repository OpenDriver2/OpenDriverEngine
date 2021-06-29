#include <nstd/String.hpp>
#include <nstd/File.hpp>

#include "math/Vector.h"
#include "math/Volume.h"

#include "core/dktypes.h"
#include "core/cmdlib.h"

#include "renderer/gl_renderer.h"

#include "routines/textures.h"

#include "main.h"
#include "render_model.h"
#include "camera.h"

#include "render_sky.h"

#include <string.h>

#define SKY_VERTEX_SHADER \
	"	attribute vec4 a_position_tu;\n"\
	"	attribute vec4 a_normal_tv;\n"\
	"	attribute vec4 a_color;\n"\
	"	uniform mat4 u_View;\n"\
	"	uniform mat4 u_Projection;\n"\
	"	uniform mat4 u_World;\n"\
	"	uniform mat4 u_WorldViewProj;\n"\
	"	void main() {\n"\
	"		v_texcoord = vec2(a_position_tu.w, 1.0-a_normal_tv.w);\n"\
	"		v_normal = mat3(u_World) * a_normal_tv.xyz;\n"\
	"		v_color = a_color;\n"\
	"		gl_Position = u_WorldViewProj * vec4(a_position_tu.xyz, 1.0);\n"\
	"	}\n"

#define SKY_FRAGMENT_SHADER \
	"	uniform sampler2D s_texture;\n"\
	"	void main() {\n"\
	"		vec4 lighting;\n"\
	"		vec4 color = texture2D(s_texture, v_texcoord.xy);\n"\
	"		fragColor = color * v_color;\n"\
	"	}\n"

const char* sky_shader =
	"varying vec2 v_texcoord;\n"
	"varying vec3 v_normal;\n"
	"varying vec4 v_color;\n"
	"#ifdef VERTEX\n"
	SKY_VERTEX_SHADER
	"#else\n"
	SKY_FRAGMENT_SHADER
	"#endif\n";

const int SKY_CLUT_START_Y = 252;
const int SKY_SIZE_W = 256 / 4;
const int SKY_SIZE_H = 84;

const int SKY_OFFSET_STEP = 0x10000;

const int SKY_TEX_CHANNELS = 4;
const int SKY_TEXPAGE_SIZE = 512 * 256;

// TODO: CSkyRenderer

TextureID g_skyTexture = 0;
ShaderID g_skyShader = 0;

UV g_skytexuv[28] = { 0 };
short g_skytpage[28][2] = { 0 };

uint8 g_HorizonLookup[4][4] = {
	{0, 0, 20, 20},
	{20, 20, 0, 0},
	{0, 0, 20, 20},
	{0, 0, 20, 20},
};

uint8 g_HorizonTextures[40] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	0, 1, 2, 3, 4, 5, 6, 7,
	20, 21, 22, 23, 24, 25, 26, 27,
	16, 17, 18, 19
};


void CopyTpageImage(ushort* tp_src, ushort* dst, int x, int y, int dst_w, int dst_h)
{
	ushort* src = tp_src + x + 128 * y;

	for (int i = 0; i < dst_h; i++)
	{
		memcpy(dst, src, dst_w * sizeof(short));
		dst += dst_w;
		src += 128;
	}
}

void ConvertIndexedSkyImage(uint* color_data, ubyte* src_indexed, int x_idx, int y_idx, bool outputBGR, bool originalTransparencyKey)
{
	ushort imageClut[16];

	int clut_x = x_idx * 16;
	int clut_y = SKY_CLUT_START_Y + y_idx;

	CopyTpageImage((ushort*)src_indexed, (ushort*)imageClut, clut_x, clut_y, 16, 1);

	int ox = x_idx * SKY_SIZE_W * 2;
	int oy = y_idx * SKY_SIZE_H;
	int w = SKY_SIZE_W * 2;
	int h = SKY_SIZE_H;

	int tp_wx = ox + w;
	int tp_hy = oy + h;

	for (int y = oy; y < tp_hy; y++)
	{
		for (int x = ox; x < tp_wx; x++)
		{
			ubyte clindex = src_indexed[y * 256 + x / 2];

			if (0 != (x & 1))
				clindex >>= 4;

			clindex &= 0xF;

			// flip texture by Y
			int ypos = (256 - y - 1) * 512;

			if (outputBGR)
			{
				TVec4D<ubyte> color = rgb5a1_ToBGRA8(imageClut[clindex], originalTransparencyKey);
				color_data[ypos + x] = *(uint*)(&color);
			}
			else
			{
				TVec4D<ubyte> color = rgb5a1_ToRGBA8(imageClut[clindex], originalTransparencyKey);
				color_data[ypos + x] = *(uint*)(&color);
			}
		}
	}
}

static int vertexSky_horizontaboffset;
void VertexSkyCb(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert)
{
	int skytexnum = g_HorizonTextures[vertexSky_horizontaboffset + polyNum];
	
	if (polyNum >= 12)
	{
		vert.cr *= 0.5f;
		vert.cg *= 0.5f;
		vert.cb *= 0.5f;
	}

	UV& uvs = g_skytexuv[skytexnum];
	short* tp = g_skytpage[skytexnum];

	vert.tc_u = (tp[0] * 4.0) / 512.0f;
	vert.tc_v = tp[1] / 256.0f;

	// map to 0..1
	if (polyVertNum == 0)
	{
		vert.tc_u += ((float)uvs.u2 + 0.5f) / 512.0f;
		vert.tc_v += ((float)uvs.v2 + 0.5f) / 256.0f;
	}
	else if (polyVertNum == 1)
	{
		vert.tc_u += ((float)uvs.u3 + 0.5f) / 512.0f;
		vert.tc_v += ((float)uvs.v3 + 0.5f) / 256.0f;
	}
	else if (polyVertNum == 2)
	{
		vert.tc_u += ((float)uvs.u1 + 0.5f) / 512.0f;
		vert.tc_v += ((float)uvs.v1 + 0.5f) / 256.0f;
	}
	else if (polyVertNum == 3)
	{
		vert.tc_u += ((float)uvs.u0 + 0.5f) / 512.0f;
		vert.tc_v += ((float)uvs.v0 + 0.5f) / 256.0f;
	}
}

// Generate UVs just like REDRIVER2 does
void GenerateSkyUVs()
{
	int flipped, single;

	int u, v, x, y, ry;
	int tp_x, clut_x, i;

	i = 0;
	y = 0;

	flipped = 1;

	do {
		if (flipped)
		{
			switch (y)
			{
			default:
				ry = 0;
				flipped = 0;
				single = 1;
				break;
			case 1:
				ry = 0;
				flipped = 0;
				single = 0;
				break;
			case 2:
				ry = 1;
				flipped = 0;
				single = 0;
				break;
			case 3:
				ry = 1;
				flipped = 1;
				single = 0;
				break;
			case 4:
				ry = 0;
				flipped = 1;
				single = 0;
				break;
			case 5:
				ry = 2;
				flipped = 0;
				single = 0;
				break;
			case 6:
				ry = 2;
				flipped = 1;
				single = 0;
				break;
			}
		}
		else
		{
			ry = 0;
			flipped = 0;
			single = 1;
		}

		x = 0;
		y++;

		//clut_x = 320;
		tp_x = 0;

		v = ry * 84;

		do {
			u = x * 128;

			if (single)
			{
				g_skytexuv[i].u0 = u;
				g_skytexuv[i].v0 = v;
				g_skytexuv[i].u1 = u;
				g_skytexuv[i].v1 = v;
				g_skytexuv[i].u2 = u;
				g_skytexuv[i].v2 = v;
				g_skytexuv[i].u3 = u;
				g_skytexuv[i].v3 = v;
			}
			else if (flipped)
			{
				g_skytexuv[i].u0 = u;
				g_skytexuv[i].v0 = v + 83;
				g_skytexuv[i].u1 = u + 127;
				g_skytexuv[i].v1 = v + 83;
				g_skytexuv[i].u2 = u;
				g_skytexuv[i].v2 = v;
				g_skytexuv[i].u3 = u + 127;
				g_skytexuv[i].v3 = v;
			}
			else
			{
				g_skytexuv[i].u0 = u;
				g_skytexuv[i].v0 = v;
				g_skytexuv[i].u1 = u + 127;
				g_skytexuv[i].v1 = v;
				g_skytexuv[i].u2 = u;
				g_skytexuv[i].v2 = v + 83;
				g_skytexuv[i].u3 = u + 127;
				g_skytexuv[i].v3 = v + 83;
			}

			g_skytpage[i][0] = tp_x & 0xffffffc0;
			g_skytpage[i][1] = ry * 84 & 768;
			
			tp_x += 32;

			x++;
			i++;
		} while (x < 4);

		flipped = (y < 7);

	} while (y < 7);

	// pre-process models
	for (int i = 0; i < 4; i++)
	{
		ModelRef_t* ref = g_levModels.GetModelByIndex(i);
		vertexSky_horizontaboffset = g_HorizonLookup[/*GameLevel*/0][i];

		if (ref && ref->userData)
		{
			CRenderModel* renderModel = (CRenderModel*)ref->userData;
			renderModel->GenerateBuffers(VertexSkyCb);
		}
	}
}

void InitSkyShader()
{
	g_skyShader = GR_CompileShader(sky_shader);
}

// Initialize sky texture and UVs
bool InitSky(int skyNumber)
{
	File file;
	if (!file.open("DRIVER2/DATA/SKY0.RAW"))
	{
		MsgError("Unable to open 'SKY0.RAW'\n");
		return false;
	}

	int64 fileSize = file.size();

	ubyte* data = new ubyte[fileSize];

	file.read(data, fileSize);
	file.close();

	ubyte* skyImage = data + skyNumber * SKY_OFFSET_STEP;

	uint* color_data;
	int imgSize = SKY_TEXPAGE_SIZE * SKY_TEX_CHANNELS;

	color_data = new uint[imgSize / sizeof(uint)];

	// 3x4 images (128x84 makes 256x252 tpage)
	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			ConvertIndexedSkyImage(color_data, skyImage, x, y, false, false);
		}
	}

	g_skyTexture = GR_CreateRGBATexture(512, 256, (ubyte*)color_data);

	delete[] data;
	delete[] color_data;

	InitSkyShader();
	GenerateSkyUVs();

	return true;
}

// Destroys sky texture and UVs
void DestroySky()
{
	GR_DestroyTexture(g_skyTexture);
	g_skyTexture = -1;
}

// Renders sky
void RenderSky()
{
	Volume dummy;
	GR_SetShader(g_skyShader);

	SetupCameraViewAndMatrices(vec3_up * -0.005f, g_cameraAngles, dummy);

	GR_SetDepth(0);
	GR_SetCullMode(CULL_FRONT);

	GR_SetTexture(g_skyTexture);

	GR_UpdateMatrixUniforms();

	// first 4 models are sky models
	for (int i = 0; i < 4; i++)
	{
		ModelRef_t* ref = g_levModels.GetModelByIndex(i);

		if (ref && ref->userData)
		{
			CRenderModel* renderModel = (CRenderModel*)ref->userData;
			renderModel->Draw(false);
		}
	}
}
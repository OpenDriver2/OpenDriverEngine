#include "core/core_common.h"

#include "render/ViewParams.h"
#include "render_sky.h"
#include "routines/textures.h"
#include "routines/models.h"
#include "render_model.h"
#include "game/shared/world.h"

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
	"	uniform vec4 u_skyColor;"\
	"	void main() {\n"\
	"		vec4 lighting;\n"\
	"		vec4 color = texture2D(s_texture, v_texcoord.xy);\n"\
	"		fragColor = color * v_color * u_skyColor;\n"\
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

const int D2_SKY_CLUT_START_Y = 252;
const int D2_SKY_SIZE_W = 512 / 4;
const int D2_SKY_SIZE_H = 252 / 3;
const int D2_SKY_TEXPAGE_SIZE = 512 * 256; // it's treated as 2:1
const int D2_SKY_OFFSET_STEP = 0x10000;

const int D1_SKY_TEXPAGE_SIZE = 256 * 128; // it's treated as 2:1
const int D1_SKY_SIZE_W = 256 / 4;
const int D1_SKY_SIZE_H = 126 / 3;

const int SKY_TEX_CHANNELS = 4;

// TODO: CSkyRenderer

TextureID g_skyTexture = 0;
ShaderID g_skyShader = 0;
int g_skyColorConstantId = -1;

UV g_skytexuv[28] = { 0 };
short g_skytpage[28][2] = { 0 };

uint8 g_HorizonLookupD2[4][4] = {
	{0, 0, 20, 20},
	{20, 20, 0, 0},
	{0, 0, 20, 20},
	{0, 0, 20, 20},
};

uint8 g_HorizonTexturesD2[40] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	0, 1, 2, 3, 4, 5, 6, 7,
	20, 21, 22, 23, 24, 25, 26, 27,
	16, 17, 18, 19
};

uint8 g_HorizonLookupD1[4] = {
	0, 12, 24, 36
};

uint8 g_HorizonTexturesD1[48] = {
    0x0,  0x2,  0x6,  0xA,
    0x1,  0x5,  0x9,  0x4,
    0x8,  0xB,  0x7,  0x3,
    0x2,  0xB,  0x7,  0x3,
    0x6,  0xA,  0x8,  0x4,
    0x0,  0x1,  0x5,  0x9,
    0x9,  0x0,  0x4,  0x8,
    0x5,  0x1,  0xA,  0x6,
    0x2,  0x3,  0x7,  0xB,
    0x0,  0x4,  0x8,  0x3,
    0x9,  0x5,  0x1,  0x7,
    0xB,  0x2,  0x6,  0xA,
};

#pragma pack(push, 1)
struct BMP_INFO_HEADER
{
	int size;
	int width;
	int height;
	short planes;
	short bitcount;
	int compression;
	int sizeimage;
	int xpelsPerMeter;
	int ypelsPerMeter;
	int clrused;
	int clrimportant;
};

struct BMP_FILE_HEADER
{
	char type[2];
	int size;
	short _reserved[2];
	int offbits;
	BMP_INFO_HEADER info;
};
#pragma pack(pop)

static void CopyTpageImage(ushort* tp_src, ushort* dst, int x, int y, int dst_w, int dst_h)
{
	ushort* src = tp_src + x + 128 * y;

	for (int i = 0; i < dst_h; i++)
	{
		memcpy(dst, src, dst_w * sizeof(short));
		dst += dst_w;
		src += 128;
	}
}

static void ConvertIndexedSkyImageD1(uint* color_data, ushort* imageClut, ubyte* src_indexed, int x_idx, int y_idx, bool outputBGR, bool originalTransparencyKey)
{
	int ox = x_idx * D1_SKY_SIZE_W;
	int oy = y_idx * D1_SKY_SIZE_H;
	int w = D1_SKY_SIZE_W;
	int h = D1_SKY_SIZE_H;

	int tp_wx = ox + w;
	int tp_hy = oy + h;

	for (int y = oy; y < tp_hy; y++)
	{
		for (int x = ox; x < tp_wx; x++)
		{
			ubyte clindex = src_indexed[(y - oy) * 32 + (x - ox) / 2];

			if (0 != (x & 1))
				clindex >>= 4;

			clindex &= 0xF;

			// flip texture by Y
			int ypos = (128 - y - 1) * 256;

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

static void ConvertIndexedSkyImageD2(uint* color_data, ubyte* src_indexed, int x_idx, int y_idx, bool outputBGR, bool originalTransparencyKey)
{
	ushort imageClut[16];

	int clut_x = x_idx * 16;
	int clut_y = D2_SKY_CLUT_START_Y + y_idx;

	CopyTpageImage((ushort*)src_indexed, (ushort*)imageClut, clut_x, clut_y, 16, 1);

	int ox = x_idx * D2_SKY_SIZE_W;
	int oy = y_idx * D2_SKY_SIZE_H;
	int w = D2_SKY_SIZE_W;
	int h = D2_SKY_SIZE_H;

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
static void VertexSkyCbD1(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert)
{
	const int skytexnum = g_HorizonTexturesD1[vertexSky_horizontaboffset + polyNum];
	
	UV& uvs = g_skytexuv[skytexnum];
	short* tp = g_skytpage[skytexnum];

	vert.tc_u = (tp[0] * 4.0) / 256.0f;
	vert.tc_v = tp[1] / 128.0f;

	// map to 0..1
	if (polyVertNum == 0)
	{
		vert.tc_u += ((float)uvs.u2 + 0.5f) / 256.0f;
		vert.tc_v += ((float)uvs.v2 + 0.5f) / 128.0f;
	}
	else if (polyVertNum == 1)
	{
		vert.tc_u += ((float)uvs.u3 + 0.5f) / 256.0f;
		vert.tc_v += ((float)uvs.v3 + 0.5f) / 128.0f;
	}
	else if (polyVertNum == 2)
	{
		vert.tc_u += ((float)uvs.u1 + 0.5f) / 256.0f;
		vert.tc_v += ((float)uvs.v1 + 0.5f) / 128.0f;
	}
	else if (polyVertNum == 3)
	{
		vert.tc_u += ((float)uvs.u0 + 0.5f) / 256.0f;
		vert.tc_v += ((float)uvs.v0 + 0.5f) / 128.0f;
	}
}

static void VertexSkyCbD2(int polyNum, const dpoly_t& poly, int polyVertNum, GrVertex& vert)
{
	const int skytexnum = g_HorizonTexturesD2[vertexSky_horizontaboffset + polyNum];

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

static void GenerateSkyUVs_Driver2()
{
	int flipped, single;

	int u, v, x, y, ry;
	int tp_x, clut_x, i;

	i = 0;
	flipped = 1;

	for (y = 0; y < 7; y++)
	{
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

		tp_x = 0;
		v = ry * D2_SKY_SIZE_H;

		for (x = 0; x < 4; x++)
		{
			u = x * D2_SKY_SIZE_W;
			UV& uvs = g_skytexuv[i];

			if (single)
			{
				uvs.u0 = u;
				uvs.v0 = v;
				uvs.u1 = u;
				uvs.v1 = v;
				uvs.u2 = u;
				uvs.v2 = v;
				uvs.u3 = u;
				uvs.v3 = v;
			}
			else if (flipped)
			{
				uvs.u0 = u;
				uvs.v0 = v + D2_SKY_SIZE_H - 1;
				uvs.u1 = u + D2_SKY_SIZE_W - 1;
				uvs.v1 = v + D2_SKY_SIZE_H - 1;
				uvs.u2 = u;
				uvs.v2 = v;
				uvs.u3 = u + D2_SKY_SIZE_W - 1;
				uvs.v3 = v;
			}
			else
			{
				uvs.u0 = u;
				uvs.v0 = v;
				uvs.u1 = u + D2_SKY_SIZE_W - 1;
				uvs.v1 = v;
				uvs.u2 = u;
				uvs.v2 = v + D2_SKY_SIZE_H - 1;
				uvs.u3 = u + D2_SKY_SIZE_W - 1;
				uvs.v3 = v + D2_SKY_SIZE_H - 1;
			}

			g_skytpage[i][0] = tp_x & 0xffffffc0;
			g_skytpage[i][1] = ry * D2_SKY_SIZE_H & 768;

			tp_x += 32;
			i++;
		}

		flipped = y < 7;
	}
}

static void GenerateSkyUVs_Driver1()
{
	const int bmpWidth = D1_SKY_SIZE_W >> 2; // 64 >> 2;
	const int bmpHeight = D1_SKY_SIZE_H;

	int rect_x = 0; // 512;
	int rect_y = 0;

	for (int i = 0; i < 12; i++)
	{
		UV& uvs = g_skytexuv[i];

		uvs.u0 = rect_x << 2;
		uvs.u1 = rect_x * 4 + (bmpWidth << 2) - 1;
		uvs.u2 = rect_x << 2;
		uvs.u3 = rect_x * 4 + (bmpWidth << 2) - 1;
		uvs.v0 = rect_y;
		uvs.v1 = rect_y;
		uvs.v2 = bmpHeight + rect_y - 1;
		uvs.v3 = bmpHeight + rect_y - 1;

		rect_x += bmpWidth;
		if (rect_x == D1_SKY_SIZE_W) // 576 - 512
		{
			rect_x = 0;
			rect_y += bmpHeight;
		}

		g_skytpage[i][0] = 0;
		g_skytpage[i][1] = 0;
	}
}

// Generate UVs just like REDRIVER2 does
static void GenerateSkyUVs()
{
	ModelRef_t* skyModel = CWorld::GetModelByIndex(0);
	if (skyModel->model->num_polys == 20)
		GenerateSkyUVs_Driver2();
	else
		GenerateSkyUVs_Driver1();

	// pre-process models
	for (int i = 0; i < 4; i++)
	{
		ModelRef_t* ref = CWorld::GetModelByIndex(i);
		ref->model->num_polys;

		if (ref && ref->userData)
		{
			CRenderModel* renderModel = (CRenderModel*)ref->userData;

			if (skyModel->model->num_polys == 20)
			{
				vertexSky_horizontaboffset = g_HorizonLookupD2[/*GameLevel*/0][i];
				renderModel->GenerateBuffers(nullptr, VertexSkyCbD2);
			}
			else
			{
				vertexSky_horizontaboffset = g_HorizonLookupD1[i];
				renderModel->GenerateBuffers(nullptr, VertexSkyCbD1);
			}
		}
	}
}

//----------------------------------------------------

ColorRGB CSky::Color(1.0f);

void CSky::Init()
{
	g_skyShader = GR_CompileShader(sky_shader);
	g_skyColorConstantId = GR_GetShaderConstantIndex(g_skyShader, "u_skyColor");
}

void CSky::Lua_Init(sol::state& lua)
{
	LUADOC_GLOBAL();

	auto engine = lua["engine"].get_or_create<sol::table>();

	{
		LUADOC_TYPE("Sky");
		auto sky = engine["Sky"].get_or_create<sol::table>();

		sky[LUADOC_M("Load", "(filename: string, num: int) : boolean - Loads sky from file with specified index")]
			= &Load;

		sky[LUADOC_M("Unload", "(void)")]
			= &Unload;

		sky[LUADOC_P("color", "(vec3)")]
			= &Color;
	}
}

// Initialize sky texture and UVs
bool CSky::Load(const char* filename, int skyNumber)
{
	if (filename == nullptr)
		return false;

	String skyName = String::fromCString(filename);
	tchar* subStr = const_cast<tchar*>(skyName.find('#', 0));

	if (subStr)
	{
		if (skyNumber > 2)
			skyNumber = 2;
		skyNumber += atoi(subStr + 1);
		skyName.trim(subStr);
	}

	File file;
	if (!file.open(skyName, File::readFlag))
	{
		MsgError("Unable to open '%s'\n", filename);
		return false;
	}

	Unload();

	GenerateSkyUVs();

	// TODO: handle D1 sky texture files
	int64 fileSize = file.size();
	ubyte* data = new ubyte[fileSize];

	file.read(data, fileSize);
	file.close();

	// Driver 1 skies?
	String ext = File::extension(skyName);
	if (ext.startsWith("BIN"))
	{
		// should have 256x126 image
		const int imgSize = D1_SKY_TEXPAGE_SIZE * SKY_TEX_CHANNELS;
		uint* color_data = new uint[imgSize / sizeof(uint)];

		// total 15 sky textures
		int* offsetInfo = (int*)(data + skyNumber * 48);

		for (int i = 0; i < 12; i++)
		{
			// get the BMP data
			int bmpSize = offsetInfo[i + 1] - offsetInfo[i];
			ubyte* bmpData = data + offsetInfo[i];

			BMP_FILE_HEADER* hdr = (BMP_FILE_HEADER*)bmpData;
			ubyte* imageData = (ubyte*)(hdr + 1);

			ushort skyclutData[16];
			for (int j = 0; j < 16; ++j)
			{
				ubyte b = imageData[0];
				ubyte g = imageData[1];
				ubyte r = imageData[2];

				imageData += 4;
				skyclutData[j] = (ushort)(r >> 3) | (ushort)(g >> 3) << 5 | (ushort)(b >> 3) << 10 | 0x8000;
			}
			
			ushort pixMapData[2048];
			ushort* pixData = (ushort*)(imageData + (hdr->info.height - 1) * 32);
			for (int yy = 0; yy < hdr->info.height; ++yy)
			{
				for (int xx = 0; xx < 16; ++xx) 
				{
					ushort pix = pixData[xx];
					pixMapData[yy * 16 + xx] = (ushort)((pix & 0xf) << 4) | (ushort)((pix & 0xf0) >> 4) | (ushort)((pix & 0xf00) << 4) | (ushort)((pix & 0xf000) >> 4);
				}
				pixData -= 16;
			}

			ConvertIndexedSkyImageD1(color_data, skyclutData, (ubyte*)pixMapData, i % 4, i / 4, false, false);
		}

		// Notice that it should be treated as 256x128 texture or UVs will be wrong!
		g_skyTexture = GR_CreateRGBATexture(256, 128, (ubyte*)color_data);
		delete[] color_data;
	}
	else
	{
		ubyte* skyImage = data + skyNumber * D2_SKY_OFFSET_STEP;

		const int imgSize = D2_SKY_TEXPAGE_SIZE * SKY_TEX_CHANNELS;
		uint* color_data = new uint[imgSize / sizeof(uint)];

		// 3x4 images (128x84 makes 512x252 tpage)
		for (int y = 0; y < 3; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				ConvertIndexedSkyImageD2(color_data, skyImage, x, y, false, false);
			}
		}

		// Notice that it should be treated as 512x256 texture or UVs will be wrong!
		g_skyTexture = GR_CreateRGBATexture(512, 256, (ubyte*)color_data);
		delete[] color_data;
	}

	delete[] data;

	return true;
}

// Destroys sky texture and UVs
void CSky::Unload()
{
	GR_DestroyTexture(g_skyTexture);
	g_skyTexture = -1;
}

// Renders sky
void CSky::Draw(const CViewParams& view)
{
	CViewParams _view = view;
	Volume dummy;
	GR_SetShader(g_skyShader);

	const float sky_y_offset = 16.0f / 4096.0f;
	_view.SetOrigin(vec3_up * -sky_y_offset);

	CCamera::SetupViewAndMatrices(_view, dummy);
	GR_SetShaderConstantVector4D(g_skyColorConstantId, Vector4D(Color, 1.0f));

	GR_SetDepthMode(0, 0);
	GR_SetCullMode(CULL_FRONT);

	GR_SetTexture(g_skyTexture);

	// first 4 models are sky models
	for (int i = 0; i < 4; i++)
	{
		ModelRef_t* ref = CWorld::GetModelByIndex(i);

		if (ref && ref->userData)
		{
			CRenderModel* renderModel = (CRenderModel*)ref->userData;
			renderModel->Draw(false);
		}
	}
}
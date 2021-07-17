#ifndef D2_TYPES
#define D2_TYPES

#include "core/dktypes.h"
#include "math/psx_math_types.h"

#define TEXPAGE_SIZE_X	(128)	// don't ask why. It's 4 bit.
#define TEXPAGE_SIZE_Y	(256)

#define TEXPAGE_4BIT_SIZE	(TEXPAGE_SIZE_X*TEXPAGE_SIZE_Y)
#define TEXPAGE_SIZE		(TEXPAGE_SIZE_Y*TEXPAGE_SIZE_Y)

enum ETextureSetFlags
{
	TEX_PERMANENT		= 0x1,		// permanently loaded into VRAM
	TEX_SWAPABLE		= 0x2,		// spooled and uncompressed
	TEX_SPECIAL			= 0x4,		// special car texture page in D2
	TEX_DAMAGED			= 0x8,
	TEX_8BIT			= 0x10,		// two 8bit TSets make up a single bitmap
	TEX_PALNUM			= 0x20,		// mask out for palette number (0-1)
	TEX_PARENT			= 0x40,
};

struct TEXINF
{
	uint16		id;
	uint16		nameoffset;
	uint8		x, y, width, height;
};

struct TEXPAGE_POS	// og name: TP
{
	uint flags;
	uint offset;
};

struct TEXCLUT
{
	ushort colors[16];
};

//------------------------------------------------------------------------------------------------------------

struct PALLET_INFO
{
	int palette;
	int texnum;
	int tpage;
	int clut_number;
};

struct PALLET_INFO_D1
{
	int palette;
	int texnum;
	int tpage;
};

//---------------------------------------------------------------

struct LUMP
{
	uint	type;
	int		size;
};

struct OUT_CITYLUMP_INFO
{
	uint	loadtime_offset;
	uint	loadtime_size;

	uint	tpage_offset;
	uint	tpage_size;

	uint	inmem_offset;
	uint	inmem_size;

	uint	spooled_offset;
	uint	spooled_size;
};

//------------------------------------------------------------------------------------------------------------

struct OUT_CELL_FILE_HEADER
{
	int cells_across;
	int cells_down;
	int cell_size;

	int num_regions;
	int region_size;

	int num_cell_objects;
	int num_cell_data;

	int ambient_light_level;
	VECTOR_NOPAD light_source;
};

//------------------------------------------------------------------------------------------------------------

// car models
#define MAX_CAR_MODELS		13

struct carmodelentry_t
{
	int cleanOffset;		// -1 if no model
	int damOffset;
	int lowOffset;
};

// FIXME: it's guessed
struct POLYF3
{
	uchar id;
	uchar v0;
	uchar v1;
	uchar v2;
	uchar pad;
	CVECTOR_NOPAD color;
	char pad2[4];
};

// FIXME: it's guessed
struct POLYF4
{
	uchar id; // 0
	uchar pad1;
	uchar pad2;
	uchar spare;
	uchar v0; //4
	uchar v1;
	uchar v2;
	uchar v3;
	CVECTOR_NOPAD color;
	char pad[5];
};

struct POLYFT4
{
	uchar id;
	uchar texture_set;
	uchar texture_id;
	uchar spare;
	uchar v0, v1, v2, v3;
	UV_INFO uv0, uv1, uv2, uv3;
	CVECTOR color;
};

struct POLYGT4
{
	uchar id;
	uchar texture_set;
	uchar texture_id;
	uchar spare;
	uchar v0, v1, v2, v3;
	uchar n0, n1, n2, n3;
	UV_INFO uv0, uv1, uv2, uv3;
	CVECTOR color;
};

struct POLYFT3
{
	uchar id;
	uchar texture_set;
	uchar texture_id;
	uchar spare;
	uchar v0, v1, v2, pad;
	UV_INFO uv0, uv1, uv2, pad2;
	CVECTOR color;
};

struct POLYGT3
{
	uchar id;
	uchar texture_set;
	uchar texture_id;
	uchar spare;
	uchar v0, v1, v2, pad;
	uchar n0, n1, n2, pad2;
	UV_INFO uv0, uv1, uv2, pad3;
	CVECTOR color;
};

#define COLLISION_BOX		0
#define COLLISION_CYLINDER	1
#define COLLISION_CONE		2
#define COLLISION_SPHERE	3
#define	COLLISION_INDOORS	4

struct COLLISION_PACKET
{
	short type;
	short xpos, ypos, zpos;
	short flags;
	short yang;
	short empty;
	short xsize, ysize, zsize;
};

enum ModelShapeFlags
{
	SHAPE_FLAG_LITPOLY			= 0x1,
	SHAPE_FLAG_BSPDATA			= 0x2,
	SHAPE_FLAG_TRANS			= 0x8,
	SHAPE_FLAG_NOCOLLIDE		= 0x10,
	SHAPE_FLAG_WATER			= 0x80,		// model is water
	SHAPE_FLAG_AMBIENT2			= 0x100,	// Ambient sound 2 associated - maybe used in D1
	SHAPE_FLAG_AMBIENT1			= 0x200,	// Ambient sound 1 associated - maybe used in D1
	SHAPE_FLAG_TILE				= 0x400,	// treat as road
	SHAPE_FLAG_SHADOW			= 0x800,	// D1 leftover flag
	SHAPE_FLAG_ALPHA			= 0x1000,	// alpha tested object
	SHAPE_FLAG_ROAD				= 0x2000,	// section of road
	SHAPE_FLAG_SPRITE			= 0x4000,
};

enum ModelFlags2
{
	MODEL_FLAG_MEDIAN = 0x20,		// Hmmmm...
	MODEL_FLAG_JUNC = 0x40,
	MODEL_FLAG_ALLEY = 0x80,		// alley tile
	MODEL_FLAG_INDOORS = 0x100,
	MODEL_FLAG_CHAIR = 0x200,
	MODEL_FLAG_BARRIER = 0x400,
	MODEL_FLAG_SMASHABLE = 0x800,
	MODEL_FLAG_LAMP = 0x1000,
	MODEL_FLAG_TREE = 0x2000,
	MODEL_FLAG_GRASS = 0x4000,
	MODEL_FLAG_PATH = 0x8000,
};

struct MODEL
{
	short	shape_flags;
	short	flags2;
	short	instance_number;

	ubyte	tri_verts;
	ubyte	zBias;

	short	bounding_sphere;
	short	num_point_normals;

	short	num_vertices;
	short	num_polys;

	int		vertices;
	int		poly_block;
	int		normals;
	int		point_normals;
	int		collision_block;

	SVECTOR* pVertex(int i) const
	{
		return (SVECTOR *)((ubyte *)this + vertices) + i;
	}

	SVECTOR* pNormal(int i) const
	{
		return (SVECTOR*)((ubyte*)this + normals) + i;
	}
	
	SVECTOR* pPointNormal(int i) const
	{
		return (SVECTOR *)((ubyte *)this + point_normals) + i;
	}

	char* pPolyAt(int ofs) const
	{
		return (char *)((ubyte *)this + poly_block + ofs);
	}

	int GetCollisionBoxCount()
	{
		if(collision_block != 0)
			return *(int*)((ubyte*)this + collision_block);

		return 0;
	}

	COLLISION_PACKET* pCollisionBox(int i)
	{
		return (COLLISION_PACKET*)((ubyte*)this + collision_block + sizeof(int)) + i;
	}
};

//------------------------------------------------------------------------------------------------------------

struct CELL_DATA {
	ushort num; // size=0, offset=0
};

struct CELL_DATA_D1 {
	ushort num; // size=0, offset=0
	ushort next_ptr;
};

struct PACKED_CELL_OBJECT {
	USVECTOR_NOPAD			pos;
	ushort					value; // packed angle and model param
};

struct CELL_OBJECT {
	struct VECTOR_NOPAD		pos;
	ubyte					pad; // just to be aligned in PSX memory
	ubyte					yang;
	ushort					type;
};

//------------------------------------------------------------------------------------------------------------

enum SurfaceType
{
	SurfType_Asphalt = 0,
	SurfType_Grass = 4,
	SurfType_Water = 6,
	SurfType_DeepWater = 9,		// the default surface
};

struct sdPlane
{
	short surfaceType;
	short a, b, c;
	int d;
};

struct sdNode
{
	int angle : 11;
	int dist : 12;
	int offset : 8;
	int node : 1;
};

struct sdHeightmapHeader
{
	short type;
	short planesOfs;
	short bspOfs;
	short nodesOfs;
};

//------------------------------------------------------------------------------------------------------------

struct DRIVER2_CURVE
{
	int Midx, Midz;
	short start, end;
	short ConnectIdx[4];
	short gradient, height;
	int8 NumLanes, LaneDirs, inside, AILanes;
};

struct DRIVER2_STRAIGHT
{
	int Midx, Midz;
	ushort length;
	short bing, angle;
	short ConnectIdx[4];
	int8 NumLanes, LaneDirs, AILanes, packing;
};

struct OLD_DRIVER2_JUNCTION
{
	int Midx;
	int Midz;
	short length;
	short width;
	short angle;
	short ExitIdx[4];
	ushort flags;
};

struct DRIVER2_JUNCTION
{
	short ExitIdx[4];
	uint flags;
};

//------------------------------------------------------------------------------------------------------------

struct AreaDataStr {
	uint16	gfx_offset;
	uint16	model_offset;
	uint16	music_offset;
	uint16	ambient_offset;
	uint8	model_size;

	uint8	pad;

	uint8	num_tpages;

	uint8	ambient_size;

	uint8	music_size;
	uint8	music_samples_size;
	uint8	music_id;

	uint8	ambient_id;
};

#define SUPERREGION_NONE	(0xFF)

class CTexturePage;

struct AreaTpageList
{
	uint8			pageIndexes[16];
	CTexturePage*	tpage[16];
};

#define REGTEXPAGE_EMPTY	(0xFF)

struct Spool 
{
	uint16	offset;
	uint8	connected_areas[2];
	uint8	pvs_size;
	uint8	cell_data_size[3];
	uint8	super_region;
	uint8	num_connected_areas;
	uint8	roadm_size;
	uint8	roadh_size;
};

#define REGION_EMPTY	(0xFFFF)

#endif
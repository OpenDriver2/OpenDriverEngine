#include "routines/d2_types.h"
#include "render_cars.h"
#include "core/cmdlib.h"

/* TODO:
	- Add car model rendering and denting stuff
	- Add wheel model renderer


*/



void CRender_Cars::MangleWheelModel(MODEL* model)
{
	UV_INFO tmpUV2;
	uchar tmpUV;
	uint v0, v1, v2;

	// do some fuckery swaps
	POLYFT4* src = (POLYFT4*)model->pPolyAt(0);

	// wheel faces
	v0 = *(uint*)&src[2].v0;
	v1 = *(uint*)&src[2].uv0;
	v2 = *(uint*)&src[2].uv2;
	*(uint*)src = *(uint*)(src + 2);
	*(uint*)&src[0].v0 = v0;
	*(uint*)&src[0].uv0 = v1;
	*(uint*)&src[0].uv2 = v2;
	src[0].color = src[2].color;

	v0 = *(uint*)&src[3].v0;
	v1 = *(uint*)&src[3].uv0;
	v2 = *(uint*)&src[3].uv2;
	*(uint*)(src + 1) = *(uint*)(src + 3);
	*(uint*)&src[1].v0 = v0;
	*(uint*)&src[1].uv0 = v1;
	*(uint*)&src[1].uv2 = v2;
	src[1].color = src[3].color;

	v0 = *(uint*)&src[4].v0;
	v1 = *(uint*)&src[4].uv0;
	v2 = *(uint*)&src[4].uv2;
	*(uint*)(src + 2) = *(uint*)(src + 4);
	*(uint*)&src[2].v0 = v0;
	*(uint*)&src[2].uv0 = v1;
	*(uint*)&src[2].uv2 = v2;
	src[2].color = src[4].color;

	v0 = *(uint*)&src[6].v0;
	v1 = *(uint*)&src[6].uv0;
	v2 = *(uint*)&src[6].uv2;
	*(uint*)(src + 3) = *(uint*)(src + 6);
	*(uint*)&src[3].v0 = v0;
	*(uint*)&src[3].uv0 = v1;
	*(uint*)&src[3].uv2 = v2;
	src[3].color = src[6].color;

	src[2].v2 = 23;
	src[2].v3 = 22;

	src[2].v0 = 21;
	src[2].v1 = 20;

	src[3].v2 = 19;
	src[3].v3 = 18;

	src[3].v1 = 17;
	src[3].v0 = 16;

	src[4].v2 = 22;
	src[4].v3 = 23;

	src[4].v0 = 19;
	src[4].v1 = 18;

	src[5].v2 = 16;
	src[5].v3 = 17;

	src[5].v1 = 21;
	src[5].v0 = 20;

	// make untextured
	src[2].texture_set = 255;
	src[3].texture_set = 255;
	src[4].texture_set = 255;
	src[5].texture_set = 255;

	model->num_polys = 6;
}
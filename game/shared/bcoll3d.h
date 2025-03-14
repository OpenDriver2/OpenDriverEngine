#pragma once
#include "math/psx_math_types.h"

class CCar;

struct CRET3D
{
	VECTOR_NOPAD location;
	VECTOR_NOPAD normal;
	int depth;
};

bool collided3d(CCar* cp0, CCar* cp1, CRET3D& least);

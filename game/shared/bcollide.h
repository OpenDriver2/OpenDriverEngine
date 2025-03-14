#pragma once

#include "math/psx_math_types.h"

// collision data
struct CDATA2D
{
	VECTOR_NOPAD x{ 0 };
	VECTOR_NOPAD axis[2]{ 0 };
	VECTOR_NOPAD vel{ 0 };
	int theta{ 0 };
	int length[2]{ 0 };
	int dist[2]{ 0 };
	int limit[2]{ 0 };
	int avel{ 0 };
	bool ignoreBarrier{ false };
};

// The collision result
struct CRET2D
{
	VECTOR_NOPAD hit{ 0 };
	VECTOR_NOPAD surfNormal{ 0 };
	int penetration{ 0 };
	bool neverfree{ false };
};

//----------------------------------------

bool	bcollided2d(CDATA2D body[2], int* boxOverlap = nullptr);
void	bFindCollisionPoint(CDATA2D body[2], CRET2D& collisionResult);
int		bFindCollisionTime(CDATA2D body[2], CRET2D& collisionResult);

#ifndef BCOLLIDE_H
#define BCOLLIDE_H

#include "math/psx_math_types.h"

// collision data
struct CDATA2D
{
	VECTOR_NOPAD x;
	VECTOR_NOPAD axis[2];
	VECTOR_NOPAD vel;
	int theta;
	int length[2];
	int dist[2];
	int limit[2];
	int avel;
	int isCameraOrTanner;
};

// The collision result
struct CRET2D
{
	VECTOR_NOPAD hit;
	VECTOR_NOPAD surfNormal;
	int penetration;
	int neverfree;
};

//----------------------------------------

int		bcollided2d(CDATA2D body[2], int* boxOverlap = nullptr);

void	bFindCollisionPoint(CDATA2D body[2], CRET2D& collisionResult);
int		bFindCollisionTime(CDATA2D body[2], CRET2D& collisionResult);

#endif // BCOLLIDE_H
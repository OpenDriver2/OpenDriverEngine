#include "game/pch.h"
#include "bcollide.h"

// Checks of two bodies collides (basic check) with Separating Axis Theorem
// also initializes axes
bool bcollided2d(CDATA2D body[2], int* boxOverlap)
{
	const int dtheta = body[1].theta - body[0].theta;

	// calc axes of each box
	for (int i = 0; i < 2; i++)
	{
		const int as = isin(body[i].theta);
		const int ac = icos(body[i].theta);

		body[i].axis[0].vx = as;
		body[i].axis[0].vz = ac;

		body[i].axis[1].vz = -as;
		body[i].axis[1].vx = ac;
	}

	const int as = isin(dtheta & 2047);
	const int ac = isin(dtheta + 1024 & 2047);

	VECTOR_NOPAD delta;
	delta.vx = body[0].x.vx - body[1].x.vx;
	delta.vz = body[0].x.vz - body[1].x.vz;

	// do SAT tests for each axis
	for (int i = 1, k = 0; i >= 0; --i, k++)
	{
		for (int j = 1; j >= 0; --j)
		{
			body[i].dist[j] = FIXEDH(body[i].axis[j].vx * delta.vx + body[i].axis[j].vz * delta.vz);
			body[i].limit[j] = body[i].length[j] + FIXEDH(body[k].length[j] * ac + body[k].length[1 - j] * as);

			if (body[i].dist[j] < -body[i].limit[j] ||
				body[i].dist[j] > body[i].limit[j])
			{
				return 0;
			}
		}
	}

	// calc overlap if needed
	// This is absent in Driver 1
	if (boxOverlap)
	{
		int FE = abs(body[1].dist[0]) - abs(body[1].limit[0]);
		int tmp = abs(FIXEDH(body->axis[0].vx * body[1].axis[0].vx + body->axis[0].vz * body[1].axis[0].vz));

		const int xover = (tmp > 10) ? (FE * ONE) / tmp : -1;

		FE = abs(body[1].dist[1]) - abs(body[1].limit[1]);
		tmp = abs(FIXEDH(body->axis[0].vx * body[1].axis[1].vx + body->axis[0].vz * body[1].axis[1].vz));

		const int zover = (tmp > 10) ? (FE * ONE) / tmp : xover;

		if (xover <= -1)
			*boxOverlap = zover;
		else if (zover < xover)
			*boxOverlap = zover;
		else
			*boxOverlap = xover;
	}

	return 1;
}

// [D] [T]
void bFindCollisionPoint(CDATA2D body[2], CRET2D& collisionResult)
{
	bool carBarrierCollision = false;

	if (!body[0].isCameraOrTanner && !body[1].isCameraOrTanner &&
		(body[1].length[1] >= body[1].length[0] * 4 || body[1].length[0] >= body[1].length[1] * 4))
	{
		carBarrierCollision = true;
	}

	int smallest = body[0].limit[0] + 1;
	int besti = 0;
	int bestk = 0;
	int sign = 0;

	for (int i = 1; i >= 0; --i)
	{
		for (int k = 1; k >= 0; --k)
		{
			const int upper = body[i].limit[k] - body[i].dist[k];
			const int lower = body[i].dist[k] + body[i].limit[k];

			if (smallest > upper)
			{
				sign = (i == 1) ? 1 : -1;

				smallest = upper;
				besti = i;
				bestk = k;
			}

			if (smallest > lower)
			{
				sign = (i == 1) ? -1 : 1;

				smallest = lower;
				besti = i;
				bestk = k;
			}
		}
	}

	// calc push
	if (carBarrierCollision)
	{
		for (int k = 1; k >= 0; --k)
		{
			const int upper = body[1].limit[k] - body[1].dist[k];
			const int lower = body[1].dist[k] + body[1].limit[k];

			if (upper < lower && (body[1].length[k] < body[1].length[1 - k] * 4))
			{
				besti = 1;
				sign = 1;
				bestk = k;
			}

			if (lower < upper && (body[1].length[k] < body[1].length[1 - k] * 4))
			{
				besti = 1;
				sign = -1;
				bestk = k;
			}
		}
	}

	const CDATA2D& cd = body[besti ^ 1];

	int sign0;
	if (cd.axis[0].vx * body[besti].axis[bestk].vx + cd.axis[0].vz * body[besti].axis[bestk].vz + 2048 > -1)
		sign0 = -sign;
	else
		sign0 = sign;

	int sign1;
	if (cd.axis[1].vx * body[besti].axis[bestk].vx + cd.axis[1].vz * body[besti].axis[bestk].vz + 2048 > -1)
		sign1 = -sign;
	else
		sign1 = sign;

	collisionResult.penetration = smallest;

	collisionResult.hit.vx = cd.x.vx + FIXEDH(cd.axis[0].vx * cd.length[0] * sign0 + cd.axis[1].vx * cd.length[1] * sign1);
	collisionResult.hit.vz = cd.x.vz + FIXEDH(cd.axis[0].vz * cd.length[0] * sign0 + cd.axis[1].vz * cd.length[1] * sign1);

	if (besti != 0)
		sign = -sign;

	collisionResult.surfNormal.vy = 0;
	collisionResult.surfNormal.vx = body[besti].axis[bestk].vx * sign;
	collisionResult.surfNormal.vz = body[besti].axis[bestk].vz * sign;
}

// [D] [T]
int bFindCollisionTime(CDATA2D cd[2], CRET2D& collisionResult)
{
	CDATA2D original[2];

	bool hit = true;
	int time = 4096;
	int step = 2048;

	collisionResult.neverfree = true;

	for (int i = 0; i < 2; ++i)
		original[i] = cd[i];

	for (int i = 7; i >= 0; --i) 
	{
		for (int q = 0; q < 2; q++)
		{
			cd[q].vel.vx >>= 1;
			cd[q].vel.vz >>= 1;
			cd[q].avel >>= 1;

			if (hit)
			{
				cd[q].x.vx -= cd[q].vel.vx;
				cd[q].x.vz -= cd[q].vel.vz;
				cd[q].theta -= cd[q].avel;
			}
			else
			{
				cd[q].x.vx += cd[q].vel.vx;
				cd[q].x.vz += cd[q].vel.vz;
				cd[q].theta += cd[q].avel;
			}
		}

		if (hit)
		{
			time -= step;
		}
		else
		{
			collisionResult.neverfree = false;
			time += step;
		}

		hit = bcollided2d(cd);

		if (i != 0)
		{
			step >>= 1;
		}
	}

	if (!hit)
	{
		for (int i = 0; i < 2; i++)
		{
			cd[i].x.vx += cd[i].vel.vx;
			cd[i].x.vz += cd[i].vel.vz;
			cd[i].theta += cd[i].avel;
		}

		bcollided2d(cd);

		time += step;
	}
	else if (collisionResult.neverfree)
	{
		for (int i = 0; i < 2; i++)
		{
			cd[i] = original[i];
			bcollided2d(cd);
		}

		time = ONE;
	}

	return time;
}
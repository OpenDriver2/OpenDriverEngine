#include <nstd/Base.hpp>
#include "bcollide.h"
#include "math/isin.h"
#include <stdlib.h>

// Checks of two bodies collides (basic check) with Separating Axis Theorem
// also initializes axes
int bcollided2d(CDATA2D body[2], int* boxOverlap)
{
	int dtheta;
	int ac, as;
	int i, j, k;
	int xover, zover;
	int tmp;
	int FE;
	VECTOR_NOPAD delta;

	dtheta = body[1].theta - body[0].theta;

	// calc axes of each box
	for (i = 0; i < 2; i++)
	{
		as = isin(body[i].theta);
		ac = icos(body[i].theta);

		body[i].axis[0].vx = as;
		body[i].axis[0].vz = ac;

		body[i].axis[1].vz = -as;
		body[i].axis[1].vx = ac;
	}

	// FIXME: can be incorrect
	as = isin(dtheta & 0x7ff);			// rcossin_tbl[(dtheta & 0x7ff) * 2];
	ac = isin(dtheta + 1024 & 0x7ff);	// rcossin_tbl[(dtheta + 1024 & 0x7ff) * 2];

	delta.vx = body[0].x.vx - body[1].x.vx;
	delta.vz = body[0].x.vz - body[1].x.vz;

	k = 0;
	i = 1;

	// do SAT tests for each axis
	do {
		j = 1;
		do {
			body[i].dist[j] = FIXEDH(body[i].axis[j].vx * delta.vx + body[i].axis[j].vz * delta.vz);
			body[i].limit[j] = body[i].length[j] + FIXEDH(body[k].length[j] * ac + body[k].length[1 - j] * as);

			if (body[i].dist[j] < -body[i].limit[j] ||
				body[i].dist[j] > body[i].limit[j])
				return 0;

			j--;
		} while (j >= 0);

		k++;
		i--;
	} while (i >= 0);

	// calc overlap if needed
	// This is absent in Driver 1
	if (boxOverlap)
	{
		FE = abs(body[1].dist[0]) - abs(body[1].limit[0]);
		FE = abs(FE);

		tmp = FIXEDH(body->axis[0].vx * body[1].axis[0].vx + body->axis[0].vz * body[1].axis[0].vz);
		tmp = abs(tmp);

		if (tmp > 10)
			xover = (FE * ONE) / tmp;
		else
			xover = -1;

		FE = abs(body[1].dist[1]) - abs(body[1].limit[1]);
		FE = abs(FE);

		tmp = FIXEDH(body->axis[0].vx * body[1].axis[1].vx + body->axis[0].vz * body[1].axis[1].vz);
		tmp = abs(tmp);

		if (tmp > 10)
			zover = (FE * ONE) / tmp;
		else
			zover = xover;

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
	int carBarrierCollision;
	int lower, upper;
	int i, k, besti, bestk;
	int sign, sign0, sign1, smallest;

	CDATA2D* cd;

	besti = 0;
	bestk = 0;
	sign = 0;
	carBarrierCollision = 0;

	smallest = body[0].limit[0] + 1;

	if (!body[0].isCameraOrTanner && !body[1].isCameraOrTanner &&
		(body[1].length[1] >= body[1].length[0] * 4 || body[1].length[0] >= body[1].length[1] * 4))
	{
		carBarrierCollision = 1;
	}

	i = 1;
	do {
		k = 1;
		do {
			upper = body[i].limit[k] - body[i].dist[k];
			lower = body[i].dist[k] + body[i].limit[k];

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

			k--;
		} while (k >= 0);

		i--;
	} while (i >= 0);

	// calc push
	if (carBarrierCollision)
	{
		k = 1;
		do {
			upper = body[1].limit[k] - body[1].dist[k];
			lower = body[1].dist[k] + body[1].limit[k];

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

			k--;
		} while (k >= 0);
	}

	cd = &body[(besti ^ 1)];

	if (cd->axis[0].vx * body[besti].axis[bestk].vx + cd->axis[0].vz * body[besti].axis[bestk].vz + 2048 > -1)
		sign0 = -sign;
	else
		sign0 = sign;

	if (cd->axis[1].vx * body[besti].axis[bestk].vx + cd->axis[1].vz * body[besti].axis[bestk].vz + 2048 > -1)
		sign1 = -sign;
	else
		sign1 = sign;

	collisionResult.penetration = smallest;

	collisionResult.hit.vx = cd->x.vx + FIXEDH(cd->axis[0].vx * cd->length[0] * sign0 + cd->axis[1].vx * cd->length[1] * sign1);
	collisionResult.hit.vz = cd->x.vz + FIXEDH(cd->axis[0].vz * cd->length[0] * sign0 + cd->axis[1].vz * cd->length[1] * sign1);

	if (besti != 0)
		sign = -sign;

	collisionResult.surfNormal.vy = 0;
	collisionResult.surfNormal.vx = body[besti].axis[bestk].vx * sign;
	collisionResult.surfNormal.vz = body[besti].axis[bestk].vz * sign;
}

// [D] [T]
int bFindCollisionTime(CDATA2D cd[2], CRET2D& collisionResult)
{
	int hit;
	int i, q;
	int time, step;
	int neverfree;
	CDATA2D original[2];

	hit = 1;
	neverfree = 1;
	time = 4096;
	step = 2048;

	for (i = 0; i < 2; i++)
		original[i] = cd[i];

	i = 7;
	do {
		for (q = 0; q < 2; q++)
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
			neverfree = 0;
			time += step;
		}

		hit = bcollided2d(cd);

		if (i != 0)
			step >>= 1;

		i--;
	} while (i >= 0);

	if (hit == 0)
	{
		for (i = 0; i < 2; i++)
		{
			cd[i].x.vx += cd[i].vel.vx;
			cd[i].x.vz += cd[i].vel.vz;
			cd[i].theta += cd[i].avel;
		}

		bcollided2d(cd);

		time += step;
	}
	else if (neverfree)
	{
		for (i = 0; i < 2; i++)
		{
			cd[i] = original[i];
			bcollided2d(cd);
		}

		time = ONE;
	}

	collisionResult.neverfree = neverfree;

	return time;
}
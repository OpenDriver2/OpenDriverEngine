#include "game/pch.h"
#include "bcoll3d.h"

struct TestResult
{
	int depth;
	VECTOR_NOPAD location;
	VECTOR_NOPAD normal;
};

// [D] [T]
int PointFaceCheck(CCar* cp0, CCar* cp1, int i, TestResult& least, int nSign)
{
	int partialDepth, depth;
	const SVECTOR_NOPAD* radii;
	int k;

	VECTOR_NOPAD normal;
	VECTOR_NOPAD diff;
	VECTOR_NOPAD point;

	const OrientedBox& cp0_oBox = cp0->GetOrientedBox();
	const OrientedBox& cp1_oBox = cp1->GetOrientedBox();
	const MATRIX& cp0_matrix = cp0->GetMatrix();

	point.vx = cp1_oBox.location.vx;
	point.vy = cp1_oBox.location.vy;
	point.vz = cp1_oBox.location.vz;

	normal.vx = cp0_matrix.m[0][i];
	normal.vy = cp0_matrix.m[1][i];
	normal.vz = cp0_matrix.m[2][i];

	diff.vx = point.vx - cp0_oBox.location.vx;
	diff.vy = point.vy - cp0_oBox.location.vy;
	diff.vz = point.vz - cp0_oBox.location.vz;

	depth = FIXEDH(diff.vx * normal.vx + diff.vy * normal.vy + diff.vz * normal.vz);

	if (depth < 0)
	{
		normal.vx = -normal.vx;
		normal.vy = -normal.vy;
		normal.vz = -normal.vz;
	}
	else
	{
		depth = -depth;
	}

	radii = cp1_oBox.radii;
	depth += cp0_oBox.length[i];

	for (k = 0; k < 3; k++)
	{
		diff.vx = radii[k].vx;
		diff.vy = radii[k].vy;
		diff.vz = radii[k].vz;

		partialDepth = FIXEDH(diff.vx * normal.vx + diff.vy * normal.vy + diff.vz * normal.vz);

		if (partialDepth < 0)
		{
			partialDepth = -partialDepth;
		}
		else
		{
			diff.vx = -diff.vx;
			diff.vy = -diff.vy;
			diff.vz = -diff.vz;
		}

		point.vx += diff.vx;
		point.vy += diff.vy;
		point.vz += diff.vz;

		depth += partialDepth;
	}

	if (depth < least.depth)
	{
		least.depth = depth;

		if (depth >= 0)
		{
			least.location.vx = point.vx;
			least.location.vy = point.vy;
			least.location.vz = point.vz;

			if (nSign < 0)
			{
				normal.vx = -normal.vx;
				normal.vy = -normal.vy;
				normal.vz = -normal.vz;
			}

			least.normal.vx = normal.vx;
			least.normal.vy = normal.vy;
			least.normal.vz = normal.vz;
		}

		return depth;
	}

	return least.depth;
}

// [D] [T]
bool collided3d(CCar* cp0, CCar* cp1, TestResult& least)
{
	int i;

	least.depth = 0x40000000;

	// check up-down first
	if (PointFaceCheck(cp0, cp1, 1, least, 1) >= 0 &&
		PointFaceCheck(cp1, cp0, 1, least, -1) >= 0)
	{
		least.depth = 0x40000000;

		i = 0;

		// check sides
		while (PointFaceCheck(cp0, cp1, i, least, 1) >= 0)
		{
			if (PointFaceCheck(cp1, cp0, i, least, -1) >= 0)
			{
				i += 2;

				if (i <= 2)
					continue;

				return true;
			}
		}
	}

	return false;
}

// [D] [T]
int CarCarCollision3(CCar* c0, CCar* c1, int& depth, VECTOR_NOPAD& where, VECTOR_NOPAD& normal)
{
	int res;
	TestResult tr;

	res = collided3d(c0, c1, tr);

	if (res)
	{
		depth = tr.depth;

		where.vx = tr.location.vx;
		where.vy = tr.location.vy;
		where.vz = tr.location.vz;

		normal.vx = tr.normal.vx;
		normal.vy = tr.normal.vy;
		normal.vz = tr.normal.vz;
	}

	return res;
}




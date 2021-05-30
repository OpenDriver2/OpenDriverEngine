//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Box/Frustum volume
//////////////////////////////////////////////////////////////////////////////////

#include "Volume.h"

void Volume::LoadAsFrustum(const Matrix4x4 &mvp)
{
	m_planes[VOLUME_PLANE_LEFT  ] = Plane(mvp[12] - mvp[0], mvp[13] - mvp[1], mvp[14] - mvp[2],  mvp[15] - mvp[3]);
	m_planes[VOLUME_PLANE_RIGHT ] = Plane(mvp[12] + mvp[0], mvp[13] + mvp[1], mvp[14] + mvp[2],  mvp[15] + mvp[3]);

	m_planes[VOLUME_PLANE_TOP   ] = Plane(mvp[12] - mvp[4], mvp[13] - mvp[5], mvp[14] - mvp[6],  mvp[15] - mvp[7]);
	m_planes[VOLUME_PLANE_BOTTOM] = Plane(mvp[12] + mvp[4], mvp[13] + mvp[5], mvp[14] + mvp[6],  mvp[15] + mvp[7]);

	m_planes[VOLUME_PLANE_FAR   ] = Plane(mvp[12] - mvp[8], mvp[13] - mvp[9], mvp[14] - mvp[10], mvp[15] - mvp[11]);
	m_planes[VOLUME_PLANE_NEAR  ] = Plane(mvp[12] + mvp[8], mvp[13] + mvp[9], mvp[14] + mvp[10], mvp[15] + mvp[11]);
}

void Volume::LoadAsFrustum(const Matrix4x4 &mvp, bool _PRECISION)
{
	m_planes[VOLUME_PLANE_LEFT] = Plane(mvp[12] - mvp[0], mvp[13] - mvp[1], mvp[14] - mvp[2], mvp[15] - mvp[3], _PRECISION);
	m_planes[VOLUME_PLANE_RIGHT] = Plane(mvp[12] + mvp[0], mvp[13] + mvp[1], mvp[14] + mvp[2], mvp[15] + mvp[3], _PRECISION);

	m_planes[VOLUME_PLANE_TOP] = Plane(mvp[12] - mvp[4], mvp[13] - mvp[5], mvp[14] - mvp[6], mvp[15] - mvp[7], _PRECISION);
	m_planes[VOLUME_PLANE_BOTTOM] = Plane(mvp[12] + mvp[4], mvp[13] + mvp[5], mvp[14] + mvp[6], mvp[15] + mvp[7], _PRECISION);

	m_planes[VOLUME_PLANE_FAR] = Plane(mvp[12] - mvp[8], mvp[13] - mvp[9], mvp[14] - mvp[10], mvp[15] - mvp[11], _PRECISION);
	m_planes[VOLUME_PLANE_NEAR] = Plane(mvp[12] + mvp[8], mvp[13] + mvp[9], mvp[14] + mvp[10], mvp[15] + mvp[11], _PRECISION);
}

void Volume::LoadBoundingBox(const Vector3D &mins, const Vector3D &maxs)
{
	m_planes[VOLUME_PLANE_LEFT  ] = Plane(1.0f, 0.0f, 0.0f,  -mins.x);
	m_planes[VOLUME_PLANE_RIGHT ] = Plane(-1.0f, 0.0f, 0.0f,  maxs.x);

	m_planes[VOLUME_PLANE_TOP   ] = Plane(0.0f, -1.0f, 0.0f, maxs.y);
	m_planes[VOLUME_PLANE_BOTTOM] = Plane(0.0f, 1.0f, 0.0f, -mins.y);

	m_planes[VOLUME_PLANE_FAR   ] = Plane(0.0f, 0.0f, -1.0f, maxs.z);
	m_planes[VOLUME_PLANE_NEAR  ] = Plane(0.0f, 0.0f, 1.0f, -mins.z);
}

void Volume::LoadBoundingBox(const Vector3D &mins, const Vector3D &maxs, bool _PRECISION)
{
	m_planes[VOLUME_PLANE_LEFT] = Plane(1.0f, 0.0f, 0.0f, -mins.x, _PRECISION);
	m_planes[VOLUME_PLANE_RIGHT] = Plane(-1.0f, 0.0f, 0.0f, maxs.x, _PRECISION);

	m_planes[VOLUME_PLANE_TOP] = Plane(0.0f, -1.0f, 0.0f, maxs.y, _PRECISION);
	m_planes[VOLUME_PLANE_BOTTOM] = Plane(0.0f, 1.0f, 0.0f, -mins.y, _PRECISION);

	m_planes[VOLUME_PLANE_FAR] = Plane(0.0f, 0.0f, -1.0f, maxs.z, _PRECISION);
	m_planes[VOLUME_PLANE_NEAR] = Plane(0.0f, 0.0f, 1.0f, -mins.z, _PRECISION);
}

// returns back bounding box if not frustum.
void Volume::GetBBOXBack(Vector3D &mins, Vector3D &maxs) const
{
	mins = Vector3D(-m_planes[VOLUME_PLANE_LEFT].offset, -m_planes[VOLUME_PLANE_BOTTOM].offset, -m_planes[VOLUME_PLANE_NEAR].offset);
	maxs = Vector3D(m_planes[VOLUME_PLANE_RIGHT].offset, m_planes[VOLUME_PLANE_TOP].offset, m_planes[VOLUME_PLANE_FAR].offset);
}

void Volume::SetupPlane(Plane &pl, int n)
{
	m_planes[n] = pl;
}

bool Volume::IsPointInside(const Vector3D &pos) const
{
	for (int i = 0; i < 6; i++)
	{
		if (m_planes[i].Distance(pos) <= 0) 
			return false;
	}
    return true;
}

bool Volume::IsSphereInside(const Vector3D &pos, const float radius) const
{
	for (int i = 0; i < 6; i++)
	{
		if (m_planes[i].Distance(pos) <= -radius) return false;
	}
    return true;
}

bool Volume::IsTriangleInside(const Vector3D& v0, const Vector3D& v1, const Vector3D& v2) const
{
	for (int i = 0; i < 6; i++)
	{
		if (m_planes[i].Distance(v0) > 0) continue;
		if (m_planes[i].Distance(v1) > 0) continue;
		if (m_planes[i].Distance(v2) > 0) continue;
		return false;
	}

	return true;
}

bool Volume::IsBoxInside(const Vector3D &mins, const Vector3D &maxs, const float eps) const
{
	return IsBoxInside(mins.x, maxs.x,mins.y, maxs.y, mins.z, maxs.z);
}

bool Volume::IsBoxInside(const float minX, const float maxX, const float minY, const float maxY, const float minZ, const float maxZ, const float eps) const
{
    for (int i = 0; i < 6; i++)
	{
		if (m_planes[i].Distance(Vector3D(minX, minY, minZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(minX, minY, maxZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(minX, maxY, minZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(minX, maxY, maxZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(maxX, minY, minZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(maxX, minY, maxZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(maxX, maxY, minZ)) > -eps) continue;
		if (m_planes[i].Distance(Vector3D(maxX, maxY, maxZ)) > -eps) continue;

        return false;
    }
    return true;
}

bool Volume::IsIntersectsRay(const Vector3D &start,const Vector3D &dir, Vector3D &intersectionPos, float eps) const
{
	bool isinstersects = false;

	Vector3D outintersection;
	float best_dist = V_MAX_COORD;

	for (int i = 0; i < 6; i++)
	{
		if( m_planes[i].GetIntersectionWithRay( start, dir, outintersection ) )
		{
			Vector3D v = start - outintersection;
			float dist = length(start - outintersection);

			// check sphere because we have epsilon
			if(dist < best_dist && IsSphereInside( outintersection + dir*0.1f, eps))
			{
				intersectionPos = outintersection;
				best_dist = dist;
				isinstersects = true;
			}
		}
	}

	return isinstersects;
}

Vector3D Volume::GetFarLeftUp() const
{
	Vector3D p;
	m_planes[VOLUME_PLANE_FAR].GetIntersectionWithPlanes(
		m_planes[VOLUME_PLANE_TOP],
		m_planes[VOLUME_PLANE_LEFT], p);

	return p;
}

Vector3D Volume::GetFarLeftDown() const
{
	Vector3D p;
	m_planes[VOLUME_PLANE_FAR].GetIntersectionWithPlanes(
		m_planes[VOLUME_PLANE_BOTTOM],
		m_planes[VOLUME_PLANE_LEFT], p);

	return p;
}

Vector3D Volume::GetFarRightUp() const
{
	Vector3D p;
	m_planes[VOLUME_PLANE_FAR].GetIntersectionWithPlanes(
		m_planes[VOLUME_PLANE_TOP],
		m_planes[VOLUME_PLANE_RIGHT], p);

	return p;
}

Vector3D Volume::GetFarRightDown() const
{
	Vector3D p;
	m_planes[VOLUME_PLANE_FAR].GetIntersectionWithPlanes(
		m_planes[VOLUME_PLANE_BOTTOM],
		m_planes[VOLUME_PLANE_RIGHT], p);

	return p;
}
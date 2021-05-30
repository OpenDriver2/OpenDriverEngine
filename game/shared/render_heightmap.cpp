#include "debug_overlay.h"
#include "driver_routines/regions_d2.h"
#include "math/isin.h"
#include "math/Vector.h"

extern CBaseLevelMap* g_levMap;

VECTOR_NOPAD g_debugCellPos;

void DebugDrawSdNode(sdNode* node)
{
	Vector3D cpos(g_debugCellPos.vx - 512, g_debugCellPos.vy, g_debugCellPos.vz - 512);
	cpos /= ONE_F;

	Vector3D dir(icos(node->angle), 0.0f, isin(node->angle));
	dir /= ONE_F;

	Vector3D tangent = cross(dir, vec3_up);

	ColorRGBA color(0, 1, 0, 1);
	DebugOverlay_Line(cpos - dir + tangent * float(node->dist / ONE_F), cpos + dir + tangent * float(node->dist / ONE_F), color);
}

// recursively walks heightmap nodes
short* DebugDriver2SdCell_r(sdNode* node, XZPAIR* pos)
{
	if (node->node)
	{
		DebugDrawSdNode(node);

		DebugDriver2SdCell_r(node + 1, pos);
		DebugDriver2SdCell_r(node + node->offset, pos);
	}

	return SdGetBSP(node, pos);
}

//-------------------------------------------------------------
// Displays heightmap cell BSP
//-------------------------------------------------------------
void DebugDrawDriver2HeightmapCell(const VECTOR_NOPAD& cellPos, const ColorRGBA& color)
{
	// cell bounds
	int cellMinX = (((cellPos.vx - 512) >> 10) << 10) + 512;
	int cellMinZ = (((cellPos.vz - 512) >> 10) << 10) + 512;

	Vector3D cMin, cMax;
	cMin.y = cMax.y = cellPos.vy / ONE_F;

	cMin.x = cellMinX / ONE_F;
	cMin.z = cellMinZ / ONE_F;

	cMax.x = (cellMinX + 1024) / ONE_F;
	cMax.z = (cellMinZ + 1024) / ONE_F;

	XZPAIR cell;
	g_levMap->WorldPositionToCellXZ(cell, cellPos);
	CDriver2LevelRegion* region = (CDriver2LevelRegion*)g_levMap->GetRegion(cell);

	VECTOR_NOPAD cpos = cellPos;
	cpos.vx -= 512;
	cpos.vz -= 512;

	DebugOverlay_Line(Vector3D(cMax.x, cMin.y, cMin.z),
		Vector3D(cMax.x, cMin.y, cMax.z), color);

	DebugOverlay_Line(Vector3D(cMin.x, cMin.y, cMax.z),
		Vector3D(cMin.x, cMin.y, cMin.z), color);

	DebugOverlay_Line(Vector3D(cMin.x, cMin.y, cMin.z),
		Vector3D(cMax.x, cMin.y, cMin.z), color);

	DebugOverlay_Line(Vector3D(cMin.x, cMin.y, cMax.z),
		Vector3D(cMax.x, cMin.y, cMax.z), color);

	g_debugCellPos.vx = cellMinX + 512;
	g_debugCellPos.vy = cellPos.vy;
	g_debugCellPos.vz = cellMinZ + 512;

	int level = 0;
	region->SdGetCell(cpos, level, DebugDriver2SdCell_r);
}
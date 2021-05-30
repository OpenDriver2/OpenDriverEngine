#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include "math/Matrix.h"
#include "math/Vector.h"

void DebugOverlay_Init();

void DebugOverlay_Destroy();

void DebugOverlay_SetTransform(const Matrix4x4& transform);

void DebugOverlay_Line(const Vector3D& posA, const Vector3D& posB, const ColorRGBA& color);

void DebugOverlay_Box(const Vector3D& mins, const Vector3D& maxs, const ColorRGBA& color);


void DebugOverlay_Draw();

#endif // DEBUG_OVERLAY_H
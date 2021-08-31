#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include "math/Matrix.h"
#include "math/Vector.h"

#define MAX_LINE_BUFFER_SIZE		8192

class CDebugOverlay
{
public:
	static void		Init();
	static void		Destroy();

	static void		Enable(bool enable);
	static bool		IsEnabled();

	static void		SetTransform(const Matrix4x4& transform);

	static void		Line(const Vector3D& posA, const Vector3D& posB, const ColorRGBA& color);
	static void		Box(const Vector3D& mins, const Vector3D& maxs, const ColorRGBA& color);

	static void		Draw();
};

#endif // DEBUG_OVERLAY_H
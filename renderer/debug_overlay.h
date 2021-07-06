#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include "math/Matrix.h"
#include "math/Vector.h"

#include "core/ignore_vc_new.h"
#include <sol/forward.hpp>

#define MAX_LINE_BUFFER_SIZE		8192

class CDebugOverlay
{
public:
	static void		Init();
	static void		Destroy();

	static void		SetTransform(const Matrix4x4& transform);

	static void		Line(const Vector3D& posA, const Vector3D& posB, const ColorRGBA& color);
	static void		Box(const Vector3D& mins, const Vector3D& maxs, const ColorRGBA& color);

	static void		Draw();

	static void		Lua_Init(sol::state& lua);
};

#endif // DEBUG_OVERLAY_H
//#include <sol/forward.hpp>
#include <sol/sol.hpp>
#include <nstd/Memory.hpp>
#include <nstd/Array.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Map.hpp>
#include <nstd/Time.hpp>
#include <nstd/String.hpp>
#include <nstd/Math.hpp>
#include <nstd/File.hpp>
#include <nstd/Directory.hpp>
#include <nstd/Mutex.hpp>
#include <nstd/Thread.hpp>
#include <nstd/RefCount.hpp>

#include "core/dktypes.h"
#include "core/cmdlib.h"
#include "core/VirtualStream.h"
#include "core/FileStream.h"

#include "routines/d2_types.h"
#include "routines/models.h"
#include "routines/regions.h"
#include "routines/textures.h"
#include "routines/level.h"

#include "math/psx_math_types.h"
#include "math/psx_matrix.h"
#include "math/convert.h"
#include "math/ratan2.h"
#include "math/isin.h"
#include "math/squareroot0.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Volume.h"
#include "math/Plane.h"

#include "renderer/gl_renderer.h"
#include "renderer/mesh_builder.h"
#include "renderer/debug_overlay.h"

#include "audio/IAudioSystem.h"
#include "audio/source/snd_source.h"

#include "render/render_model.h"
#include "render/render_level.h"
#include "render/render_sky.h"
#include "render/render_cars.h"
#include "render/render_util.h"

#include "luabinding/luadocs.h"
#include "luabinding/luarefvalue.h" 

#include "shared/manager_cars.h"
#include "shared/cars.h"
#include "shared/players.h"
#include "shared/bcoll3d.h"
#include "shared/bcollide.h"
#include "shared/effects.h"
#include "shared/replay.h"
#include "shared/world.h"
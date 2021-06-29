
#include <string.h>

#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include "game/shared/viewer.h"

#include <nstd/String.hpp>
#include <nstd/Directory.hpp>
#include <nstd/File.hpp>

#include "routines/level.h"
#include "routines/textures.h"
#include "routines/regions_d1.h"
#include "routines/regions_d2.h"

bool g_export_carmodels = false;
bool g_export_models = false;
bool g_extract_dmodels = false;

bool g_export_world = false;
bool g_export_worldUnityScript = false;

bool g_export_textures = false;
bool g_explode_tpages = false;

bool g_export_overmap = false;

int g_overlaymap_width = 1;

//---------------------------------------------------------------------------------------------------------------------------------

OUT_CITYLUMP_INFO		g_levInfo;
CDriverLevelTextures	g_levTextures;
CDriverLevelModels		g_levModels;
CBaseLevelMap*			g_levMap = nullptr;

String					g_levname;
String					g_levname_moddir;
String					g_levname_texdir;

const float				texelSize = 1.0f / 256.0f;
const float				halfTexelSize = texelSize * 0.5f;

int main(int argc, char* argv[])
{
#ifdef _WIN32
	Install_ConsoleSpewFunction();
#endif

	Msg("---------------\nOpenDriverEngine startup\n---------------\n\n");

	if (argc <= 1)
	{
		MsgError("You must specify LEV file name!\n");
		return -1;
	}

	bool generate_denting = false;
	int main_routine = 2;

	char* arg = argv[1];

	String test = String::fromCString(arg, strlen(arg));

	if (File::extension(test).compareIgnoreCase("lev"))
	{
		MsgWarning("Unknown command line parameter '%s'\n", arg);
		return 0;
	}

	g_levname = test;

	if (g_levname.length() == 0)
	{
		return 0;
	}

	// prepare anyway
	String lev_no_ext = File::dirname(g_levname) + "/" + File::basename(g_levname, File::extension(g_levname));
	g_levname_moddir = lev_no_ext + "_models";
	g_levname_texdir = lev_no_ext + "_textures";

	ViewerMain();

	return 0;
}
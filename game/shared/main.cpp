
#include <string.h>

#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include "world.h"

#include <nstd/String.hpp>
#include <nstd/Directory.hpp>
#include <nstd/File.hpp>

#include "routines/level.h"
#include "routines/textures.h"
#include "routines/regions_d1.h"
#include "routines/regions_d2.h"

//---------------------------------------------------------------------------------------------------------------------------------

String					g_levname;

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

	ViewerMain();

	return 0;
}
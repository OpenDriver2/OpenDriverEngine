project "libvorbis"
	kind "StaticLib"
	language "C"

	includedirs
	{
		"include",
		"../libogg/include"
	}

	files
	{
		"include/**.h",
		"lib/**.h",
		"lib/**.c"
	}

	links
	{
		"libogg"
	}

	defines
	{
		"LIBOGG_EXPORTS"
	}
	
	filter "system:windows"
		systemversion "latest"

		excludes
		{
			"lib/barkmel.c",
			"lib/misc.c",
			"lib/psytune.c",
			"lib/tone.c"
		}

		defines
		{
			"WIN32",
			"NDEBUG"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		
usage "libvorbis"
	includedirs { 
		"./include"
	}
	links "libvorbis"
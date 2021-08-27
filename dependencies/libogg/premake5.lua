project "libogg"
	kind "StaticLib"
	language "C"

	includedirs
	{
		"include"
	}

	files
	{
		"include/**.h",
		"src/**.c"
	}

	defines
	{
		"LIBOGG_EXPORTS"
	}
	
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"WIN32",
			"NDEBUG",
			"_WINDOWS",
			"_USRDLL"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		
usage "libogg"
	includedirs { 
		"./include"
	}
	links "libogg"
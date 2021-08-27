-- FIXME: if NOT using OpenAL from sources but precompiled it needs to be removed
--[[project "openal-soft"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"

	includedirs
	{
		"src",
		"src/alc",
		"src/common",
		"include",
		"include/AL"
	}

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	excludes
	{
		"src/alc/mixer/mixer_neon.cpp"
	}

	defines
	{
		"AL_LIBTYPE_STATIC"
	}
	
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"WIN32",
			"_WINDOWS",
			"AL_BUILD_LIBRARY",
			"AL_ALEXT_PROTOTYPES",
			"_WIN32",
			"_WIN32_WINNT=0x0502",
			"_CRT_SECURE_NO_WARNINGS",
			"NOMINMAX",
			"CMAKE_INTDIR=\"Debug\"",
			"OpenAL_EXPORTS"
		}

		links
		{
			"winmm"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
]]
usage "openal-soft"
	includedirs {
		"include",
		"include/AL"
	}

	links "OpenAL32"
	
	filter "platforms:x64"
		libdirs {
			"./libs/Win64"
		}
		
	filter "platforms:x86"
		libdirs {
			"./libs/Win32"
		}
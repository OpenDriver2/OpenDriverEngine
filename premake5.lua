-- premake5.lua

require ".premake_modules/usage"
require ".premake_modules/unitybuild"

-- you can redefine dependencies
SDL2_DIR = os.getenv("SDL2_DIR") or "dependencies/SDL2"

workspace "OpenDriverEngine"
	location "project_%{_ACTION}_%{os.target()}"
	targetdir "bin/%{cfg.buildcfg}"
	
	language "C++"
	cppdialect "C++17"	-- required for sol2
	
	configurations { "Debug", "Release" }
	linkgroups 'On'
	characterset "ASCII"
	
	startproject "OpenDriverGame"
	
	includedirs {
		"./"
	}	
	
	filter "system:linux"
		buildoptions {
			"-Wno-narrowing",
			"-fpermissive",
		}
		links "dl"

	filter "system:Windows"
		disablewarnings { "4996", "4554", "4244", "4101", "4838", "4309" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
		linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
		}

	filter "configurations:Debug"
		defines { 
			"DEBUG", 
		}
		targetsuffix "_dbg"
		symbols "On"

	filter "configurations:Release"
		defines {
			"NDEBUG",
		}
		optimize "Full"
		
group "Dependencies"

-- NoSTD
project "libnstd"
	kind "StaticLib"

	filter "system:Windows"
		defines { "__PLACEMENT_NEW_INLINE" }
	
	includedirs {
		"dependencies/libnstd/include"
	}
	
	files {
		"dependencies/libnstd/src/**.cpp",
		"dependencies/libnstd/src/**.h",
	}
	
usage "libnstd"
	includedirs {
		"dependencies/libnstd/include"
	}
	links "libnstd"

-- libXM
project "libxm"
	kind "StaticLib"

	defines{
		"XM_DEBUG=0", 						-- Enable debug symbols and print debugging messages to stderr
		"XM_DEFENSIVE=1", 					-- Defensively check XM data for errors/inconsistencies
		"XM_BIG_ENDIAN=0", 					-- Use big endian byte order (unfinished)
		"XM_LINEAR_INTERPOLATION=1", 		-- Use linear interpolation (CPU hungry)
		"XM_RAMPING=1", 					-- Enable ramping (smooth volume/panning transitions, CPU hungry)
		"XM_STRINGS=1", 					-- Store module, instrument and sample names in context
		"XM_LIBXMIZE_DELTA_SAMPLES=1", 		-- Delta-code samples in libxmize format
	}

	includedirs {
		"dependencies/libxm/include"
	}
	
	files {
		"dependencies/libxm/src/**.c",
		"dependencies/libxm/src/**.h",
	}
	
usage "libxm"
	includedirs {
		"dependencies/libxm/include"
	}
	links "libxm"
	
-- GLAD
project "glad"
	kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	
	files {
		"dependencies/glad/*.c",
		"dependencies/glad/*.h",
	}
	
usage "glad"
	includedirs {
		"dependencies",
	}

	filter "system:linux"
		links {
			"GL",
		}
	
-- ImGui
project "ImGui"
	kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	unitybuild "on"

	uses {"SDL2", "glad"}
	
	filter "system:Windows"
		defines { "IMGUI_IMPL_OPENGL_LOADER_GLAD" }
	
	includedirs {
		"dependencies/imgui",
	}
	
	files {
		"dependencies/imgui/*.cpp",
		"dependencies/imgui/*.h",
		"dependencies/imgui/backends/imgui_impl_opengl3.cpp",
		"dependencies/imgui/backends/imgui_impl_opengl3.h",
		"dependencies/imgui/backends/imgui_impl_sdl.cpp",
		"dependencies/imgui/backends/imgui_impl_sdl.h",
	}
	
usage "ImGui"
	includedirs {
		"dependencies/imgui",
	}
	links { "ImGui" }
	
-- SDL2 as a usage
usage "SDL2"
	links {
		"SDL2",
	}
	
	filter "system:linux"
		includedirs {
			"/usr/include/SDL2"
		}

	filter "system:windows"
		includedirs {
			SDL2_DIR.."/include"
		}
		libdirs { 
			SDL2_DIR.."/lib/x86",
		}
		
include "dependencies/lua54/premake5.lua"
include "dependencies/sol2/premake5.lua"
include "dependencies/openal-soft/premake5.lua"
include "dependencies/libogg/premake5.lua"
include "dependencies/libvorbis/premake5.lua"
		
group "Game"

-- little framework
project "frameworkLib"
	kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	uses { 
		"libnstd",
		"glad", "SDL2",
		"openal-soft", "libvorbis", "libogg", "libxm"
	}
	
	files {
		"math/**.cpp",
		"math/**.h",
		"core/**.cpp",
		"core/**.h",
		"util/**.cpp",
		"util/**.h",
		"audio/**.cpp",
		"audio/**.h",
		"renderer/**.cpp",
		"renderer/**.h",
	}
	
-- driver routines lib
project "driverLib"
	kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	unitybuild "on"
	uses { 
		"libnstd", 
	}
	
	files {
		"routines/**.cpp",
		"routines/**.h",
	}

project "OpenDriverGame"
	kind "ConsoleApp"

	uses { 
		"ImGui", 
		"frameworkLib",
		"driverLib",
		"lua", "sol2"
	}

	includedirs {
		"./",
	}

	files {
		"game/**.cpp",
		"game/**.h",
	}
	
	pchheader "game/pch.h"
	pchsource "game/pch.cpp"
	
	files { "TODO.md" }
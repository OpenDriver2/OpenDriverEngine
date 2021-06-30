-- premake5.lua

require "premake_modules/usage"

-- you can redefine dependencies
SDL2_DIR = os.getenv("SDL2_DIR") or "dependencies/SDL2"

workspace "OpenDriverEngine"
	location "project_%{_ACTION}_%{os.target()}"
    language "C++"
    configurations { "Debug", "Release" }
	linkgroups 'On'
	characterset "ASCII"
	
	includedirs {
		"./"
	}
	
	startproject "OpenDriverGame"
	
    filter "system:linux"
        buildoptions {
            "-Wno-narrowing",
            "-fpermissive",
        }

	filter "system:Windows"
		disablewarnings { "4996", "4554", "4244", "4101", "4838", "4309" }

    filter "configurations:Debug"
        defines { 
            "DEBUG", 
        }
        symbols "On"

    filter "configurations:Release"
        defines {
            "NDEBUG",
        }
		characterset "ASCII"

group "Dependencies"

-- NoSTD
project "libnstd"
    kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	
	filter "system:Windows"
		defines { "_CRT_SECURE_NO_WARNINGS", "__PLACEMENT_NEW_INLINE" }
    
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
	
-- GLAD
project "glad"
    kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	
	filter "system:Windows"
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
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

	uses {"SDL2", "glad"}
	
	filter "system:Windows"
		defines { "_CRT_SECURE_NO_WARNINGS", "IMGUI_IMPL_OPENGL_LOADER_GLAD" }
    
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
		
group "Game"

-- little framework
project "frameworkLib"
    kind "StaticLib"
	targetdir "bin/%{cfg.buildcfg}"
	
	uses "libnstd"

	filter "system:Windows"
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	files {
		"math/**.cpp",
		"math/**.h",
		"core/**.cpp",
		"core/**.h",
		"util/**.cpp",
		"util/**.h",
    }

project "OpenDriverGame"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/%{cfg.buildcfg}"

	uses { "ImGui", "libnstd", "frameworkLib", "lua" }

	includedirs {
		"./",
	}

    files {
		"renderer/**.cpp",
		"renderer/**.h",
		"game/**.cpp",
		"game/**.h",
		"routines/**.cpp",
		"routines/**.h",
    }
        
    filter "system:linux"
        buildoptions {
            "-Wno-narrowing",
            "-fpermissive",
        }
		links {
			"dl"
        }
        
        cppdialect "C++11"

    filter "configurations:Debug"
		targetsuffix "_dbg"
		 symbols "On"

    filter "configurations:Release"
        optimize "Full"
	
WORKSPACE_NAME = "OpenDriver"

include "eq2engine"

property "gameapp"
	objdir "build/obj"
	if _ACTION ~= "vscode" then
		location "%{ prj_location(prj) }"
	end

group ""

include "dependencies"

group ""
	
project "OpenDriverGame"
	kind "WindowedApp"
	
	targetname "OpenDriver_%{cfg.buildcfg}"
	properties { "unitybuild", "live_pp", "app", "gameapp" }

	uses { 
		"e2Core", "frameworkLib", "coreLib",
		"fontLib", "grimLib", "equiLib", "sysLib", "soundSystemLib", "studioLib",
		"animatingLib", "movieLib", "BaseShader", "scriptLib",
		"openal-soft", "imgui", "imgui_lua", "lua"
	}

	includedirs {
		"./",
	}

	files {
		"math/**",
		"game/**",
		"util/**",
		"routines/**",
		"TODO.md"
	}
	
	filter "system:Windows"
		files {
			"./DriversGame/**.rc"
		}
		linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
		}
	
	filter "configurations:Retail"
		targetname "OpenDriver"
project "enet"
	kind "StaticLib"
	language "C"
	
	files { "*.c" }
	
	includedirs { "./include" }
		
usage "enet"
	includedirs {
		"./include",
	}
	links {"ws2_32", "wsock32", "winmm"}

project "enet"
	kind "StaticLib"
	language "C"
	
	files { "*.c" }
	
	includedirs { "./include" }
		
usage "enet"
	includedirs {
		"./include",
	}


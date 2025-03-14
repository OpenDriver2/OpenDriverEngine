-- libXM
project "libxm"
	properties { "staticlib", "thirdpartylib" }

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

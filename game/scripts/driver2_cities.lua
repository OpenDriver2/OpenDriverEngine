-----------------------------------------------------
-- DRIVER 2 Cities
-----------------------------------------------------

return {
	["Chicago"] = {
		number 		= 0,		-- city index; For necessary hardcoded stuff
		gameId 		= 2,		-- Game Index. This is Driver 2

		skyPath		= "DRIVER2/DATA/SKY0.RAW",

		-- lev path for all city types
		levPath		= {
			"DRIVER2/LEVELS/CHICAGO.LEV",
			"DRIVER2/NLEVELS/CHICAGO.LEV",
			"DRIVER2/MLEVELS/CHICAGO.LEV",
			"DRIVER2/MNLEVELS/CHICAGO.LEV",
		},
		
		events 		= {},--dofile("driver2_chicago_events.lua"),
		
		cosmetics 	= "scripts/driver2_cosmetics.lua",
		lcf			= "DRIVER2/LEVELS/CHICAGO.LCF",

		-- this is still not decided
		frontendPath = {
			Selection = "DRIVER2/DATA/CITY.RAW",
			Cars = "DRIVER2/DATA/CARS/CCARS.RAW",
			Cuts = "DRIVER2/DATA/CUTS/CCUTS.RAW",
		},

		-- map settings
		overlayMap = {
			x_offset = 197, y_offset = 318,
			width = 384, height = 672,
			
			toptile = 252, dummyImage = 0x99,
			
			scale = 2145,	-- / 4096
		}
	},
	["Havana"] = {
		number 		= 1,		-- city index; For necessary hardcoded stuff
		gameId 		= 2,		-- Game Index. This is Driver 2

		skyPath		= "DRIVER2/DATA/SKY1.RAW",

		-- lev path for all city types
		levPath		= {
			"DRIVER2/LEVELS/HAVANA.LEV",
			"DRIVER2/NLEVELS/HAVANA.LEV",
			"DRIVER2/MLEVELS/HAVANA.LEV",
			"DRIVER2/MNLEVELS/HAVANA.LEV",
		},
		
		events 		= {},--dofile("driver2_chicago_events.lua"),
	
		cosmetics 	= "scripts/driver2_cosmetics.lua",
		lcf			= "DRIVER2/LEVELS/HAVANA.LCF",

		-- map settings
		overlayMap = {
			x_offset = 229, y_offset = 85,
			width = 544, height = 352,
			
			toptile = 187, dummyImage = 0x88,
			
			scale = 2048,	-- / 4096
		}
	},
	["LasVegas"] = {
		number 		= 2,		-- city index; For necessary hardcoded stuff
		gameId 		= 2,		-- Game Index. This is Driver 2

		skyPath		= "DRIVER2/DATA/SKY2.RAW",

		-- lev path for all city types
		levPath		= {
			"DRIVER2/LEVELS/VEGAS.LEV",
			"DRIVER2/NLEVELS/VEGAS.LEV",
			"DRIVER2/MLEVELS/VEGAS.LEV",
			"DRIVER2/MNLEVELS/VEGAS.LEV",
		},
		
		events 		= {},--dofile("driver2_chicago_events.lua"),
	
		cosmetics 	= "scripts/driver2_cosmetics.lua",
		lcf			= "DRIVER2/LEVELS/VEGAS.LCF",

		-- map settings
		overlayMap = {
			x_offset = 68, y_offset = 457,
			width = 288, height = 672,
			
			toptile = 189, dummyImage = 0xBB,
			
			scale = 1911,	-- / 4096
		}
	},
	["RioDeJaneiro"] = {
		number 		= 2,		-- city index; For necessary hardcoded stuff
		gameId 		= 2,		-- Game Index. This is Driver 2

		skyPath		= "DRIVER2/DATA/SKY3.RAW",

		-- lev path for all city types
		levPath		= {
			"DRIVER2/LEVELS/RIO.LEV",
			"DRIVER2/NLEVELS/RIO.LEV",
			"DRIVER2/MLEVELS/RIO.LEV",
			"DRIVER2/MNLEVELS/RIO.LEV",
		},
		
		events 		= {},--dofile("driver2_chicago_events.lua"),
	
		cosmetics 	= "scripts/driver2_cosmetics.lua",
		lcf			= "DRIVER2/LEVELS/RIO.LCF",

		-- map settings
		overlayMap = {
			x_offset = 159, y_offset = 207,
			width = 416, height = 576,
			
			toptile = 252, dummyImage = 0x44,
			
			scale = 2048,	-- / 4096
		}
	},
}
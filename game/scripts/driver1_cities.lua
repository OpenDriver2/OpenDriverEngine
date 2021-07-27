
-----------------------------------------------------
-- DRIVER 1 Cities table
-----------------------------------------------------

return {
	["Miami"] = {
		number 		= 0,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 0.6,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/MIAMI.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 197, y_offset = 318,
			width = 384, height = 672,
			
			toptile = 252, dummyImage = 0x99,
			
			scale = 2145,	-- / 4096
		}
	},
	["SanFrancisco"] = {
		number 		= 1,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 0.85,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/FRISCO.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 197, y_offset = 318,
			width = 384, height = 672,
			
			toptile = 252, dummyImage = 0x99,
			
			scale = 2145,	-- / 4096
		}
	},
	["LosAngeles"] = {
		number 		= 2,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 1.5,
		forceNight	= true,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/LA.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 197, y_offset = 318,
			width = 384, height = 672,
			
			toptile = 252, dummyImage = 0x99,
			
			scale = 2145,	-- / 4096
		}
	},
	["NewYork"] = {
		number 		= 3,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY0.RAW",
	
		brightnessScale = 0.85,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/NEWYORK.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 197, y_offset = 318,
			width = 384, height = 672,
			
			toptile = 252, dummyImage = 0x99,
			
			scale = 2145,	-- / 4096
		}
	},
}
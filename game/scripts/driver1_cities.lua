
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
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 8, y_offset = 80,
			width = 384, height = 256,
			
			toptile = 96, dummyImage = 0,
			
			scale = 2812,	-- / 4096
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
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 83, y_offset = 162,
			width = 384, height = 416,
			
			toptile = 156, dummyImage = 0,
			
			scale = 2812,	-- / 4096
		}
	},
	["Interview"] = {
		number 		= 2,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 2,
		forceNight	= true,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/IVIEW.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 0, y_offset = 0,
			width = 448, height = 352,
			
			toptile = 0, dummyImage = 0,
			
			scale = 0,	-- / 4096
		}
	},
	["LosAngeles"] = {
		number 		= 3,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 1.5,
		forceNight	= true,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/LA.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 324, y_offset = 175,
			width = 192, height = 416,
			
			toptile = 78, dummyImage = 0,
			
			scale = 2812,	-- / 4096
		},
	},
	["Flat"] = {
		number 		= 4,
		gameId		= 1,

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/FLAT.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {}
	},
	["NewYork"] = {
		number 		= 5,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY0.RAW",
	
		brightnessScale = 0.85,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/NEWYORK.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {
			x_offset = 126, y_offset = 93,
			width = 384, height = 224,
	
			toptile = 84, dummyImage = 0,
	
			scale = 2812,	-- / 4096
		},
	},
	["Training"] = {
		number 		= 6,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/TRAIN.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {}
	},
	["NewCastle"] = {
		number 		= 7,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER2/DATA/SKY2.RAW",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/CREDITS.LEV",
		
		events 		= {},
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {}
	},
}
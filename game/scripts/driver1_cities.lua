
-----------------------------------------------------
-- DRIVER 1 Cities table
-----------------------------------------------------

return {
	["Miami"] = {
		number 		= 0,		-- city index; For necessary hardcoded stuff
		gameId 		= 1,		-- Game Index.

		skyPath		= "DRIVER/DATA/SKY.BIN#4",
		
		brightnessScale = 0.6,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/MIAMI.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#10",
		
		brightnessScale = 0.85,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/FRISCO.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#0",
		
		brightnessScale = 2,
		forceNight	= true,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/IVIEW.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#1",
		
		brightnessScale = 1,
		forceNight	= true,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/LA.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#0",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/FLAT.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#7",
	
		brightnessScale = 0.85,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/NEWYORK.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#0",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/TRAIN.LEV",
		
		events 		= nil,
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

		skyPath		= "DRIVER/DATA/SKY.BIN#1",
		
		brightnessScale = 0.8,

		-- lev path for all city types
		levPath		= "DRIVER/LEVELS/CREDITS.LEV",
		
		events 		= nil,
		cosmetics 	= "scripts/driver1_cosmetics.lua",
		lcf			= "DRIVER/COSMETICS.LCF",
		
		-- this is still not decided
		frontendPath = {},

		-- map settings
		overlayMap = {}
	},
}
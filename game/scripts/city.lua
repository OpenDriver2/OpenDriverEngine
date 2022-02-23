-----------------------------------------
-- Driver 2 city list
-----------------------------------------

CityType = {
	Day = 1,
	Night = 2,
	MPDay = 3,
	MPNight = 4
}

SkyType = {
	Day = 0,
	Night = 1,
	Rain = 2,
	Dawn = 3,
	Dusk = 4,
}

LightPresets = 
{
	[SkyType.Day] = {
		ambientColor = vec.vec3(0.8,0.84,0.88),
		lightColor 	 = vec.vec3(0.95,0.92,0.78),
		skyColor	 = vec.vec3(1,1,1)
	},
	[SkyType.Night] = {
		ambientColor = vec.vec3(1,1,1),
		lightColor 	 = vec.vec3(1,1,1),
		skyColor	 = vec.vec3(1,1,1)
	},
	[SkyType.Rain] = {
		ambientColor = vec.vec3(1,1,1),
		lightColor 	 = vec.vec3(0.35,0.35,0.35),
		skyColor	 = vec.vec3(1,1,1)
	},
	[SkyType.Dawn] = {
		ambientColor = vec.vec3(0.7,0.7,0.7),
		lightColor 	 = vec.vec3(0.5,0.5,0.5), -- TODO: function preset
		skyColor	 = vec.vec3(1,1,1)
	},
	[SkyType.Dusk] = {
		ambientColor = vec.vec3(0.7,0.7,0.8),
		lightColor 	 = vec.vec3(0.35,0.3,0.21), 		 -- TODO: function preset
		skyColor	 = vec.vec3(1,1,1)
	}
}

function IsMultiplayerCity(typeVal)
	return typeVal >= CityType.MPDay
end

function IsNightCity(typeVal)
	return typeVal == CityType.Night or typeVal == CityType.MPNight
end

CityInfo = {}

-- add cities
local d1_cities = dofile("scripts/driver1_cities.lua")
local d2_cities = dofile("scripts/driver2_cities.lua")

for k,v in pairs(d1_cities) do
	CityInfo[k] = v
end

for k,v in pairs(d2_cities) do
	CityInfo[k] = v
end





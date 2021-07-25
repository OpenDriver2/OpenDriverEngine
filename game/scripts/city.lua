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





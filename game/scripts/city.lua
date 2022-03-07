-----------------------------------------
-- City definitions
-----------------------------------------

local world = engine.World						-- collision and rendering world
local sky = engine.Sky							-- Sky renderer
local cars = engine.Cars						-- cars, handling
local levRenderProps = engine.LevelRenderProps	-- Level render properties (mode, lighting, etc)

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

---------------------------------------------------------

CityHardcoding = {}

CityHardcoding.MakeTreesAtNight = function()
	if CurrentCityInfo.gameId == 1 then -- D1 don't need this
		return
	end
	if CurrentCityInfo.number == 0 then -- Chicago trees are already lit
		return
	end
	if IsNightCity(CurrentCityType) == false then
		return
	end
	for i=0,1535 do
		local modelRef = world.GetModelByIndex(i)
		if modelRef ~= nil and i ~= 945 and i ~= 497 then
			if (modelRef.shapeFlags & ShapeFlags.Sprite) > 0 and (modelRef.modelFlags & ModelFlags.Tree) > 0 or i == 1223 then
				modelRef.lightingLevel = 0.28
			end
		end
	end
end

---------------------------------------------------------

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

-------------------------------

CurrentCityName = nil
CurrentCityInfo = {}
CurrentCityType = CityType.Day
CurrentSkyType = SkyType.Day

CityEvents = {
	OnLoading = AddCallback(function() end),
	OnLoaded = AddCallback(function() end),

	OnUnloading = AddCallback(function() end),
	OnUnloaded = AddCallback(function() end),

	OnRegionsSpooled = AddCallback(function() 
		CityHardcoding.MakeTreesAtNight()
	end),

	OnStep = AddCallback(function() end)
}

--
-- SpoolRegions : loads city regions
--		@position:		fixed point VECTOR
--		@regRadius: 	radius in regions. (Default: 1)
function SpoolRegions(position, regRadius)
	if position == nil then
		MsgWarning("Spooling all regions!")
		world.SpoolAllRegions();
		CityEvents.OnRegionsSpooled()
		return
	end

	local numSpooled = world.SpoolRegions(position, regRadius or 1)
	if numSpooled > 0 then
		CityEvents.OnRegionsSpooled()
	end
	return anySpooled
end

--
-- ChangeCity : level changer
-- 		@newCityName: 	new city name according to CityInfo table keys
--		@newCityType: 	city type (Night, MPDay etc)
--		@newWeather:	weather and sky to use
--
function ChangeCity(newCityName, newCityType, newWeather)
	local newCity = CurrentCityInfo
	if newCityName ~= nil then
		newCity = CityInfo[newCityName]
	end

	-- sky is night? make level night!
	if newWeather == SkyType.Night then
		if IsMultiplayerCity(newCityType) then
			newCityType = CityType.MPNight
		else
			newCityType = CityType.Night
		end
	else
		if IsMultiplayerCity(newCityType) then
			newCityType = CityType.MPDay
		else
			newCityType = CityType.Day
		end
	end
	
	local triggerLoading = false
	
	if  CurrentCityInfo ~= newCity or 
		CurrentCityType ~= newCityType or
		world.IsLevelLoaded() == false then

		MsgInfo("NewLevel!\n")
		
		triggerLoading = true
	end
	
	if newCityName ~= nil then
		CurrentCityName = newCityName
	end
	CurrentCityInfo = newCity
	CurrentCityType = newCityType
	CurrentSkyType = if_then_else(newCity.forceNight, SkyType.Night, newWeather)
	
	if newCity.levPath == nil then
		return
	end

	levRenderProps.nightMode = newCity.forceNight or IsNightCity(CurrentCityType)
	
	-- TODO: City lighting presets for each mode
	if newCity.forceNight == true then
		levRenderProps.nightAmbientScale = 1
		levRenderProps.nightLightScale = 0.15
		levRenderProps.ambientScale = 1
		levRenderProps.lightScale = 0.15
	elseif levRenderProps.nightMode then
		levRenderProps.nightAmbientScale = 0.5 * (CurrentCityInfo.brightnessScale or 1)
		levRenderProps.nightLightScale = 0
		levRenderProps.ambientScale = 2
		levRenderProps.lightScale = 0
	else
		levRenderProps.ambientScale = 1.0 * (CurrentCityInfo.brightnessScale or 1)
		levRenderProps.lightScale = 1.25 * (CurrentCityInfo.brightnessScale or 1)
	end

	levRenderProps.ambientColor = LightPresets[CurrentSkyType].ambientColor
	levRenderProps.lightColor = LightPresets[CurrentSkyType].lightColor
	
	if triggerLoading then
		UnloadCity()

		-- pick the LEV file from the table
		local levPath
		if type(CurrentCityInfo.levPath) == "table" then
			levPath = CurrentCityInfo.levPath[CurrentCityType]
		else
			levPath = CurrentCityInfo.levPath
		end

		CityEvents.OnLoading()
		
		if world.LoadLevel(levPath) then
			-- try load all 13 car models
			for i = 0,12 do
				cars:LoadModel(i)
			end
			sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
		
			CityEvents.OnLoaded()
		end
	else
		-- reload sky only
		sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
	end
end

--
-- UnloadCity : unloads current city
--
function UnloadCity()
	CityEvents.OnUnloading()

	cars:UnloadAllModels()
	world.UnloadLevel()

	CityEvents.OnUnloaded()
end
-- state machine setup

dofile "scripts/city.lua"
dofile "scripts/updates.lua"

-- "engine" namespace has everything dynamically updated
-- "driver" & "driver2" namespace has every class to be used with "engine"

local players = engine.Players					-- local players
local cars = engine.Cars						-- cars, handling
--local peds = engine.Pedestrians				-- pedestrians and motion capture system
local world = engine.World						-- collision and rendering world
local sky = engine.Sky							-- Sky renderer
local levRenderProps = engine.LevelRenderProps	-- Level render properties (mode, lighting, etc)

function StepSim(dt)
	-- TODO: direct port from MAIN.C
	-- 
	-- events:Update()
	-- mission:Update()
	-- civAI:PingIn()
	cars:UpdateControl()	-- controls and replay
	
	-- cops.Update()
	-- peds.Update()

	cars:GlobalTimeStep( dt )
	--players:Update()
	cars:DoScenaryCollisions()
	--players:CheckMiscFelonies()
end

CurrentCityInfo = nil
CurrentCityType = nil
CurrentSkyType = nil

function ChangeCity(newCity, newCityType, newWeather)

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
		CurrentCityType ~= newCityType then
		print("NewLevel!\n")
		triggerLoading = true
	end
	
	CurrentCityInfo = newCity
	CurrentCityType = newCityType
	CurrentSkyType = newWeather

	levRenderProps.nightMode = (CurrentCityType == CityType.Night)
	
	if triggerLoading then
		if world.LoadLevel(CurrentCityInfo.levPath[CurrentCityType]) then
			sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
			--SetUpdateFunc("StepSim", StepSim)
		end
	else
		-- reload sky only
		sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
	end
end

function StartTest()
	-- permanently disable lods
	levRenderProps.noLod = true

	ChangeCity(CityInfo["Chicago"], CityType.Day, SkyType.Day)
end

--[[
function StartTestGame()
	-- We are going to load Driver 2 level TODO: factory
	local level = driver2.Level.new()

	CurrentCityInfo = CityInfo["Havana"]
	CurrentCityType = CityType.Day
	CurrentSkyType = SkyType.Dusk

	-- set level to be used for rendering, collision etc
	world:SetLevel(level)

	-- load a level file
	if level:Load( CurrentCityInfo.levPath[CurrentCityType] ) then
		-- load sky
		sky:Load( CurrentCityInfo.skyPath, 0 )
		
		-- load car models, cosmetics and denting
		cars:Load( CurrentCityInfo, level )
		
		-- TODO: ability to load cars from another city. Need to somehow handle additional model slots
		--cars:Load()
	end
	
	-- create a car and a player
	local model = 0
	local palette = 2
		
	positionInfo = {X = 200000, Z = 300000, dir = 2048} -- X, Z, direction. Not specifying Y will place car at level.MapHeight(XZ)
		
	local car = cars:Create(model, palette, CONTROL_TYPE_PLAYER, positionInfo)

	-- create player at slot 0
	local localPlayer = players:InitWithCar(0, car)
	
	players:SetLocalPlayer(localPlayer)
	
	SetUpdateFunc("StepSim", StepSim)
end]]

--------------------------------------------------

-- This function is called every frame before drawing world
function Sys_Frame( dt )

	DoUpdateFuncs(dt)
	
end

-- This function called every frame after drawing world
-- You can draw here ImGUI stuff
function Sys_PostFrame( dt )

	if ImGui.BeginMainMenuBar() then
		if ImGui.BeginMenu("Level") then
			if ImGui.BeginMenu("Change level") then
				for k,v in pairs(CityInfo) do
					if ImGui.MenuItem(k) then
						ChangeCity(v, CurrentCityType, CurrentSkyType)
					end
				end
				ImGui.EndMenu()
			end
		
			ImGui.Separator()
			
			if ImGui.BeginMenu("Change weather type") then
				for k,v in pairs(SkyType) do
					if ImGui.MenuItem(k) then
						ChangeCity(CurrentCityInfo, CurrentCityType, v)
					end
				end
				ImGui.EndMenu()
			end
			
			ImGui.Separator()
		
			if ImGui.MenuItem("Spool all Area Data") then
				world.SpoolAllAreaDatas();
			end

			ImGui.EndMenu()
		end

		if ImGui.BeginMenu("View") then

			local selected,activated;
			
			selected,activated = ImGui.MenuItem("Disable LODs", "", levRenderProps.noLod)
			if activated then
				levRenderProps.noLod = not levRenderProps.noLod
			end

			ImGui.Separator()

			selected,activated = ImGui.MenuItem("Display collision boxes", "", levRenderProps.displayCollisionBoxes)
			if activated then
				levRenderProps.displayCollisionBoxes = not levRenderProps.displayCollisionBoxes
			end

			selected,activated = ImGui.MenuItem("Display heightmap", "", levRenderProps.displayHeightMap)
			if activated then
				levRenderProps.displayHeightMap = not levRenderProps.displayHeightMap
			end

			selected,activated = ImGui.MenuItem("Display hidden objects", "", levRenderProps.displayAllCellLevels)
			if activated then
				levRenderProps.displayAllCellLevels = not levRenderProps.displayAllCellLevels
			end
			
			--[[
			ImGui.Separator();

			if ImGui.MenuItem("Reset camera", nil, g_noLod) then
				g_cameraPosition = 0;
				g_cameraAngles = Vector3D(25.0f, 45.0f, 0);

				--g_cameraPosition = FromFixedVector({ 230347, 372, 704038 });
				--g_cameraAngles = FromFixedVector({ 0, 3840 - 1024, 0 }) * 360.0f;
			end
			]]
			ImGui.EndMenu();
		end

		ImGui.EndMainMenuBar();
	end
end

-- TEST ROUTINE
StartTest()
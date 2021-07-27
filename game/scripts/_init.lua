-- TODO:
--		Implement dynamic script loader based on Updates.lua
--		Implement engine host sandboxing
--		Implement quick Lua script reloading
--		Implement menu and FPS meters hide (also need engine code)

MsgInfo("OpenDriverEngine Lua host initialization")

-- main OpenDriverEngine file
dofile "scripts/common.lua"
dofile "scripts/city.lua"
dofile "scripts/updates.lua"
dofile "scripts/free_camera.lua"
dofile "scripts/test_game.lua"

-- "engine" namespace has everything dynamically updated
-- "driver" & "driver2" namespace has every class to be used with "engine"

local players = engine.Players					-- local players
local cars = engine.Cars						-- cars, handling
--local peds = engine.Pedestrians				-- pedestrians and motion capture system
local world = engine.World						-- collision and rendering world
local sky = engine.Sky							-- Sky renderer
local levRenderProps = engine.LevelRenderProps	-- Level render properties (mode, lighting, etc)
local camera = engine.Camera

local testGameCamera = false					-- See: RenderUI

local timeAccumulator = 0
local fixed_timestep <const> = 1.0 / 30.0

--
-- StepSim: performs physics fixed time step (callback)
--
function StepSim(dt)

	timeAccumulator = timeAccumulator + dt
	
	if timeAccumulator >= fixed_timestep then
		while timeAccumulator > fixed_timestep do

			TestGame.UpdateCarPads()

			-- TODO: direct port from MAIN.C
			-- 
			-- events:Update()
			-- mission:Update()
			-- civAI:PingIn()
			cars:UpdateControl()	-- controls and replay
			
			-- cops.Update()
			-- peds.Update()

			cars:GlobalTimeStep()
			--players:Update()
			cars:DoScenaryCollisions()
			--players:CheckMiscFelonies()


			
			timeAccumulator = timeAccumulator - fixed_timestep
		end
	end

	if testGameCamera then
		TestGame.UpdateCamera(dt)
	end
end

--
-- InitCamera : updates the game engine main camera with specified parameters
-- 		@params: 	a table of { position: vec.vec3, angles: vec.vec3, fov: float}
--
function InitCamera( params )
	-- you can't replace MainView but we can copy all parameters
	camera.MainView.position = params.position
	camera.MainView.angles = params.angles
	camera.MainView.fov = params.fov
end

local trainModel
local truckModel

--
-- Main game loop (callback)
--
function GameLoop(dt)

	-- delete old event objects
	world.PurgeCellObjects()

	FreeCamera.UpdateCameraMovement(dt)
	if testGameCamera == false then
		InitCamera({
			position = FreeCamera.Position,
			angles = FreeCamera.Angles,
			fov = FreeCamera.FOV
		})
	end
	
	local spoolPos = fix.ToFixedVector(camera.MainView.position)
	world.SpoolRegions(spoolPos, 1)
	
	-- Chicago ONLY
	if trainModel then
	
		-- push event cell object
		-- any collision checks afterwards will have an effect with it
		world.PushCellObject(CELL_OBJECT { position = fix.VECTOR(4749, -250, -11082), yang = 512 // 64, type = trainModel.index })
	
	end
	
	if truckModel then
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2579, -300, -14235), 1024 // 64, truckModel.index))
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2434, -300, -9641), 128 // 64, truckModel.index))
	end
	
	StepSim( dt )
end

CurrentCityName = nil
CurrentCityInfo = nil
CurrentCityType = nil
CurrentSkyType = nil

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
	


	levRenderProps.nightMode = newCity.forceNight or (CurrentCityType == CityType.Night)
	
	-- TODO: City lighting presets for each mode
	if levRenderProps.nightMode then
		levRenderProps.nightAmbientScale = 0.8 * (CurrentCityInfo.brightnessScale or 1)
		levRenderProps.nightLightScale = 0
		levRenderProps.ambientScale = 3
		levRenderProps.lightScale = 0
	else
		levRenderProps.ambientScale = 0.6 * (CurrentCityInfo.brightnessScale or 1)
		levRenderProps.lightScale = 1.25 * (CurrentCityInfo.brightnessScale or 1)
	end
	
	if triggerLoading then
		
		-- TODO: call a callback instead of this
		TestGame.Terminate()
		cars:RemoveAll()
		ResetFreeCamera()
		
		-- pick the LEV file from the table
		local levPath
		if type(CurrentCityInfo.levPath) == "table" then
			levPath = CurrentCityInfo.levPath[CurrentCityType]
		else
			levPath = CurrentCityInfo.levPath
		end
		
		if world.LoadLevel(levPath) then
			sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
					
			SetUpdateFunc("GameLoop", GameLoop)
			
			trainModel = world.GetModelByName("ELTRAIN")
			truckModel = world.GetModelByName("LORRY")
		end
	else
		-- reload sky only
		sky.Load( CurrentCityInfo.skyPath, CurrentSkyType )
	end
end

--------------------------------------------------------------------------
--          UI STUFF
--------------------------------------------------------------------------

function ResetFreeCamera()
	FreeCamera.Position = vec.vec3(5100 / fix.ONE, 590 / fix.ONE, -13651 / fix.ONE)
	FreeCamera.Angles = vec.vec3(25.0, 45.0, 0)
	
	--g_cameraPosition = FromFixedVector({ 230347, 372, 704038 });
	--g_cameraAngles = FromFixedVector({ 0, 3840 - 1024, 0 }) * 360.0f;
end

function StartTest()
	-- permanently disable lods
	levRenderProps.noLod = true

	CurrentCityInfo = {}
	CurrentCityType = CityType.Day
	CurrentSkyType = SkyType.Day

	
	ResetFreeCamera()
	
	--ChangeCity(CityInfo["Chicago"], CityType.Day, SkyType.Day)
end

function StopTest()
	world.UnloadLevel()
	SetUpdateFunc("GameLoop", nil)
end

function RenderUI()

	if ImGui.BeginMainMenuBar() then
		if ImGui.BeginMenu("Level") then
			if ImGui.BeginMenu("Change level") then
				for k,v in pairs(CityInfo) do
					if ImGui.MenuItem(k) then
						ChangeCity(k, CurrentCityType, CurrentSkyType)
					end
				end
				ImGui.EndMenu()
			end
		
			ImGui.Separator()
			
			if ImGui.BeginMenu("Change weather type") then
				local num = 1
				for k,v in pairs(SkyType) do

					if ImGui.MenuItem(k, tostring(num)) then
						ChangeCity(nil, CurrentCityType, v)
					end
					
					num = num + 1
				end
				ImGui.EndMenu()
			end
			
			ImGui.Separator()
		
			if ImGui.MenuItem("Spool all regions") then
				world.SpoolAllRegions();
			end
			
			if ImGui.MenuItem("Unload") then
				StopTest()
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
			
			ImGui.Separator();

			if ImGui.MenuItem("Reset camera") then
				ResetFreeCamera()
			end

			ImGui.EndMenu();
		end
		
		if ImGui.BeginMenu("Test") then
			local selected,activated;
			if ImGui.MenuItem("Begin game") then
				TestGame.Init()
			end
			
			selected,activated = ImGui.MenuItem("Game camera", "", testGameCamera)
			if activated then
				testGameCamera = not testGameCamera
			end
			ImGui.EndMenu()
		end

		ImGui.EndMainMenuBar();
	end
end

-------------------------------------------------
-- Game engine host.
-- Don't touch this code 
--    unless you know what it is
-------------------------------------------------

local function errorHandler ( errobj )
	MsgError("ERROR - "..errobj)
	MsgError(debug.traceback())
	return false
end

-- this table is always aqquired by engine
EngineHost = {
	-- This function is called every frame before drawing world
	Frame = function( dt )
		xpcall(function() 
			DoUpdateFuncs(dt) 
		end, errorHandler)
	end,

	-- This function called every frame after drawing world
	-- You can draw here ImGUI stuff
	PostFrame = function( dt )
		xpcall(RenderUI, errorHandler)
	end,
	
	MouseMove = function( x, y, xrel, yrel)
		xpcall(function() 
			FreeCamera.MouseMove(xrel, yrel)
		end, errorHandler)
	end,
	
	MouseButton = function( num, down )
		xpcall(function() 
			if num == 1 then
				FreeCamera.Look = down
			end
		end, errorHandler)

	end,
	
	KeyPress = function( num, down )
		xpcall(function() 
			FreeCamera.KeyPress(num, down)
			
			TestGame.UpdateCarControls(num, down)
		end, errorHandler)

	end
}

-- TEST ROUTINE
StartTest()
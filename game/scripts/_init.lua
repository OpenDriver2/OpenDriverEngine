-- main OpenDriverEngine file
dofile "scripts/common.lua"
dofile "scripts/city.lua"
dofile "scripts/updates.lua"
dofile "scripts/free_camera.lua"

-- "engine" namespace has everything dynamically updated
-- "driver" & "driver2" namespace has every class to be used with "engine"

local players = engine.Players					-- local players
local cars = engine.Cars						-- cars, handling
--local peds = engine.Pedestrians				-- pedestrians and motion capture system
local world = engine.World						-- collision and rendering world
local sky = engine.Sky							-- Sky renderer
local levRenderProps = engine.LevelRenderProps	-- Level render properties (mode, lighting, etc)
local camera = engine.Camera


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

function InitCamera( params )
	-- you can't replace MainView but we can copy all parameters
	camera.MainView.position = params.position
	camera.MainView.angles = params.angles
	camera.MainView.fov = params.fov
end

local trainModel
local truckModel

function GameLoop(dt)

	world.PurgeCellObjects()

	FreeCamera.UpdateCameraMovement(dt)
	InitCamera({
		position = FreeCamera.Position,
		angles = FreeCamera.Angles,
		fov = FreeCamera.FOV
	})
	
	local spoolPos = fix.ToFixedVector(camera.MainView.position)
	world.SpoolRegions(spoolPos, 1)
	
	-- Chicago ONLY
	if trainModel then
	
		-- push event cell object
		-- any collision checks afterwards will have an effect with it
		world.PushCellObject(CELL_OBJECT { position = fix.VECTOR(4749, -250, -11082), yang = 512 / 64, type = trainModel.index })
	
	end
	
	if truckModel then
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2579, -300, -14235), 1024 / 64, truckModel.index))
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2434, -300, -9641), 128 / 64, truckModel.index))
	end
	
	--StepSim( dt )
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
		CurrentCityType ~= newCityType or
		world.IsLevelLoaded() == false then

		MsgInfo("NewLevel!\n")
		
		triggerLoading = true
	end
	
	CurrentCityInfo = newCity
	CurrentCityType = newCityType
	CurrentSkyType = newWeather
	
	if newCity.levPath == nil then
		return
	end

	levRenderProps.nightMode = (CurrentCityType == CityType.Night)
	
	-- TODO: City lighting presets for each mode
	if levRenderProps.nightMode then
		levRenderProps.nightAmbientScale = 0.8
		levRenderProps.nightLightScale = 0
		levRenderProps.ambientScale = 3
		levRenderProps.lightScale = 0
	else
		levRenderProps.ambientScale = 1
		levRenderProps.lightScale = 1
	end
	
	if triggerLoading then
		if world.LoadLevel(CurrentCityInfo.levPath[CurrentCityType]) then
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

function ResetFreeCamera()
	FreeCamera.Position = vec.vec3(0)
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
	
	SetUpdateFunc("GameLoop", GameLoop)
end]]


function RenderUI()

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
				local num = 1
				for k,v in pairs(SkyType) do

					if ImGui.MenuItem(k, tostring(num)) then
						ChangeCity(CurrentCityInfo, CurrentCityType, v)
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

		ImGui.EndMainMenuBar();
	end
end

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
		xpcall(function() DoUpdateFuncs(dt) end, errorHandler)
	end,

	-- This function called every frame after drawing world
	-- You can draw here ImGUI stuff
	PostFrame = function( dt )
		xpcall(RenderUI, errorHandler)
	end,
	
	MouseMove = function( x, y, xrel, yrel)
		FreeCamera.MouseMove(xrel, yrel)
	end,
	
	MouseButton = function( num, down )
		if num == 1 then
			FreeCamera.Look = down
		end
	end,
	
	KeyPress = function( num, down )
		FreeCamera.KeyPress(num, down)
	end
}

-- TEST ROUTINE
StartTest()
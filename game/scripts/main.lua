
-- test print documentation
--PrintDocumentation()

-- "engine" namespace has everything dynamically updated
-- "driver" & "driver2" namespace has every class to be used with "engine"

local audio = engine.Audio
local players = engine.Players					-- local players
local cars = engine.Cars						-- cars, handling
--local peds = engine.Pedestrians				-- pedestrians and motion capture system
local world = engine.World						-- collision and rendering world
local levRenderProps = engine.LevelRenderProps	-- Level render properties (mode, lighting, etc)
local camera = engine.Camera
local eventModels = dofile("scripts/test_eventmodels.lua")
local testGame = dofile("scripts/test_game.lua")

local forceShowUI = false
local testGameCamera = true					-- See: RenderUI

-- TODO: mission.lua
local oldPauseState = false
local pauseState = false

local fixed_timestep <const> = 1.0 / 30.0
local timeAccumulator = 0.0

local function IsGameCameraEnabled()
	return testGameCamera and players.localPlayer.currentCar ~= nil
end

---
--- ControlMap: spool regions from player car position
---
local function ControlMap()
	if players.localPlayer.currentCar ~= nil then
		local spoolPos = players.localPlayer.currentCar.position
		SpoolRegions(spoolPos)
	end
end

--
-- StepSim: performs physics fixed time step (callback)
--
local function StepSim(dt)
	-- handle pause state
	if pauseState then
		if not oldPauseState then
			audio:SetChannelPitch(ChannelId.Sfx, 0.0)
			audio:SetChannelPitch(ChannelId.Music, 0.0)
		end
	else
		if oldPauseState then
			audio:SetChannelPitch(ChannelId.Sfx, 1.0)
			audio:SetChannelPitch(ChannelId.Music, 1.0)
		end
	end
	oldPauseState = pauseState
	
	if not pauseState then
		timeAccumulator = timeAccumulator + dt
		local numSteps = 0
	
		while timeAccumulator >= fixed_timestep do
			if numSteps > 4 then
				timeAccumulator = 0.0
				break
			end

			timeAccumulator = timeAccumulator - fixed_timestep
		
			ControlMap()

			eventModels.AddEventObjects()
			
			testGame.UpdateCarPads()

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
			--players:CheckMiscFelonies()

			numSteps = numSteps + 1

			-- advance game frame, replay time, etc
			world.EndStep()
			CityEvents.OnStep()
		end
		
		-- move and draw smashables
		MoveSmashables(dt)
	end

	DrawSmashables()
	CityEvents.OnDraw(dt)

	if IsGameCameraEnabled() then
		testGame.UpdateCamera(dt)

		-- take the main view position
		FreeCamera.Position = camera.MainView.position
		FreeCamera.Angles = camera.MainView.angles
	end
end


--
-- Main game loop (callback)
--
local function GameLoop(dt)

	FreeCamera.UpdateCameraMovement(dt)
	if IsGameCameraEnabled() == false then
		InitCamera({
			position = FreeCamera.Position,
			angles = FreeCamera.Angles,
			fov = FreeCamera.FOV
		})
			
		local spoolPos = fix.ToFixedVector(camera.MainView.position)
		SpoolRegions(spoolPos)
	end
		
	StepSim( dt )
end

--------------------------------------------------------------------------
--          UI STUFF
--------------------------------------------------------------------------

local function ResetFreeCamera()
	FreeCamera.Position = vec.vec3(5100 / fix.ONE, 590 / fix.ONE, -13651 / fix.ONE)
	FreeCamera.Angles = vec.vec3(25.0, 45.0, 0)
	
	--g_cameraPosition = FromFixedVector({ 230347, 372, 704038 });
	--g_cameraAngles = FromFixedVector({ 0, 3840 - 1024, 0 }) * 360.0f;
end

local function OnLevelLoaded()
	LoadSoundbank("permanent", SBK_Permanent)
	eventModels.InitEventModels()
	SetUpdateFunc("GameLoop", GameLoop)
end

local function OnLevelUnloading()
	testGame.Terminate()
	SetUpdateFunc("GameLoop", nil)
end

local function StartTest()
	-- permanently disable lods
	levRenderProps.noLod = true
	ResetFreeCamera()

	AddCallback(OnLevelLoaded, CityEvents.OnLoaded)
	AddCallback(OnLevelUnloading, CityEvents.OnUnloading)
end

local newCameraPos = fix.VECTOR(0,0,0)
local cameraWindowShown = false
local cameraWindowShownLast = false
local function ShowFreeCameraWindow()
	local open, draw = ImGui.Begin("Camera", cameraWindowShown)
	cameraWindowShown = open
	if draw then
		ImGui.SetWindowSize(332, 159)
		
		if ImGui.Button("Reset camera") then
			ResetFreeCamera()
		end

		do
			local newVal, changed = ImGui.InputInt("X", newCameraPos.vx);
			newCameraPos.vx = newVal
		end

		do
			local newVal, changed = ImGui.InputInt("Y", newCameraPos.vy);
			newCameraPos.vy = newVal
		end

		do
			local newVal, changed = ImGui.InputInt("Z", newCameraPos.vz);
			newCameraPos.vz = newVal
		end

		if cameraWindowShownLast ~= open or ImGui.Button("Get") then
			newCameraPos = fix.ToFixedVector(FreeCamera.Position)
		end
		ImGui.SameLine()
		if ImGui.Button("Set") then
			FreeCamera.Position = fix.FromFixedVector(newCameraPos)
		end
		ImGui.End()
	end
	cameraWindowShownLast = cameraWindowShown
end

local cellObjectWindowShown = false
local function ShowCellObjectDebugListWindow()
	local open, draw = ImGui.Begin("Debug cell object list", cellObjectWindowShown)
	cellObjectWindowShown = open
	if draw then
		local stats = engine.Stats

		local selected,activated = ImGui.Checkbox("Enable debug display", levRenderProps.displayAllCellLevels)
		if activated then
			levRenderProps.displayAllCellLevels = not levRenderProps.displayAllCellLevels
		end

		local newVal, changed = ImGui.InputInt("Index", levRenderProps.displayCellObjectList);
		levRenderProps.displayCellObjectList = math.max(newVal, -1)

		ImGui.Text(string.format("Drawn cell objects: %d", stats.debugListCellsDrawn))

		ImGui.End()
	end
end

local function RenderUI()

	if forceShowUI == false and testGame.IsRunning() and testGameCamera then
		return
	end
	
	if ImGui.Begin("HelpFrame", true, ImGuiWindowFlags.NoTitleBar + ImGuiWindowFlags.NoResize +
		ImGuiWindowFlags.NoMove + ImGuiInputTextFlags.NoHorizontalScroll |
		ImGuiWindowFlags.NoSavedSettings + ImGuiColorEditFlags.NoInputs + ImGuiWindowFlags.NoBringToFrontOnFocus) then

		local stats = engine.Stats

		ImGui.SetWindowPos(0, 24)

		ImGui.TextColored(1.0, 1.0, 1.0, 0.5, string.format("FPS: %d", stats.systemFPS))

		ImGui.SetWindowSize(400, 120)
		
		local viewPosition = camera.MainView.position

		ImGui.TextColored(1.0, 1.0, 0.25, 1.0, string.format("Position: X: %d Y: %d Z: %d",
			math.floor(viewPosition.x * fix.ONE), math.floor(viewPosition.y * fix.ONE), math.floor(viewPosition.z * fix.ONE)))

		ImGui.TextColored(1.0, 1.0, 1.0, 0.5, string.format("Draw distance: %d", stats.cellsDrawDistance))
		ImGui.TextColored(1.0, 1.0, 1.0, 0.5, string.format("Drawn cells: %d", stats.drawnCells))
		ImGui.TextColored(1.0, 1.0, 1.0, 0.5, string.format("Drawn models: %d", stats.drawnModels))
		ImGui.TextColored(1.0, 1.0, 1.0, 0.5, string.format("Drawn polygons: %d", stats.drawnPolygons))

		ImGui.End()
	end

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

			if ImGui.MenuItem("Spool all regions") then
				SpoolRegions()
			end
			
			if ImGui.MenuItem("Unload") then
				UnloadCity()
			end

			ImGui.EndMenu()
		end

		if ImGui.BeginMenu("View") then

			local selected,activated
			
			selected,activated = ImGui.MenuItem("Disable LODs", "", levRenderProps.noLod)
			if activated then
				levRenderProps.noLod = not levRenderProps.noLod
			end

			ImGui.Separator()

			if ImGui.MenuItem("Camera properties") then
				cameraWindowShown = not cameraWindowShown
			end
			ImGui.EndMenu();
		end

		if world.IsLevelLoaded() then
			if ImGui.BeginMenu("World") then
				local selected,activated
	
				if ImGui.BeginMenu("Change sky type") then
					local num = 1
					for k,v in pairs(SkyType) do
	
						if ImGui.MenuItem(k, tostring(num)) then
							ChangeCity(nil, CurrentCityType, v)
						end
						
						num = num + 1
					end
					ImGui.EndMenu()
				end
	
				ImGui.Separator();
	
				selected,activated = ImGui.MenuItem("Display collision boxes", "", levRenderProps.displayCollisionBoxes)
				if activated then
					levRenderProps.displayCollisionBoxes = not levRenderProps.displayCollisionBoxes
				end
	
				selected,activated = ImGui.MenuItem("Display heightmap", "", levRenderProps.displayHeightMap)
				if activated then
					levRenderProps.displayHeightMap = not levRenderProps.displayHeightMap
				end
	
				if ImGui.MenuItem("Cell object list debug") then
					cellObjectWindowShown = not cellObjectWindowShown
				end
	
				ImGui.EndMenu();
			end
			
			if ImGui.BeginMenu("Game") then
				local selected,activated;
				if ImGui.MenuItem("Begin game") then
					testGame.Init()
				end
				
				selected,activated = ImGui.MenuItem("Game camera", "", testGameCamera)
				if activated then
					testGameCamera = not testGameCamera
				end
	
				selected,activated = ImGui.MenuItem("Pause", "", pauseState)
				if activated then
					pauseState = not pauseState
				end
				
				if ImGui.BeginMenu("Change car") then
					
					for i,model in ipairs(cars.carModels) do
						if ImGui.MenuItem(tostring(model.index)) then
							testGame.Init(model.index)
						end
					end
					ImGui.EndMenu()
				end

				ImGui.Separator();

				if ImGui.BeginMenu("Replay") then
					if ImGui.MenuItem("Start replay") then
						testGame.StartReplay()
					end
					ImGui.EndMenu()
				end
				
				ImGui.EndMenu()
			end
		end
		
		if ImGui.BeginMenu("Help") then
			if ImGui.MenuItem("API documentation") then
				documentationWindowShown = not documentationWindowShown
			end
			ImGui.EndMenu()
		end

		ImGui.EndMainMenuBar();
	end

	ShowCellObjectDebugListWindow()
	ShowFreeCameraWindow()
	ShowDocumentationWindow()
end

-------------------------------------------------
-- Game engine host.
-- Don't touch this code 
--    unless you know what it is
-------------------------------------------------

Input = {
	ButtonState = {},
	JustPressed = {}
}

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
		Input.JustPressed = {}
	end,
	
	MouseMove = function( x, y, xrel, yrel)
		xpcall(function() 
			if IsGameCameraEnabled() == false then
				FreeCamera.MouseMove(xrel, yrel)
			end
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
			if IsGameCameraEnabled() == false then
				FreeCamera.KeyPress(num, down)
			end
			
			if num == SDL.Scancode.Escape and down then
				forceShowUI = not forceShowUI
			end

			if num == SDL.Scancode.P and down then
				pauseState = not pauseState
			end
			
			Input.JustPressed[num] = down
			Input.ButtonState[num] = down
		end, errorHandler)

	end
}

-- TEST ROUTINE
StartTest()
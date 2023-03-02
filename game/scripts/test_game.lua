local cars = engine.Cars
local world = engine.World
local players = engine.Players

-- TODO: move to worldUtils.lua
local function EnableModel(name, value)
	local modelRef = world.GetModelByName(name)
	if modelRef ~= nil then
		modelRef.enabled = value
	else
		MsgError("Model '" .. name .. "' not found!")
	end
end

local PlayerStartInfo = {
	----------- Driver 2 freeride startpos -------------
	["Chicago"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 6216, z = -222456, direction = 0},
			playerCarId = 1,
			musicType = {2, 6},
		}
	},
	["Havana"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = -238668, z = -235595, direction = 1024},
			playerCarId = 1,
			musicType = {1, 5},
		}
	},
	["LasVegas"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 143285, z = 621193, direction = -2046},
			playerCarId = 1,
			musicType = {3, 0},
		}
	},
	["RioDeJaneiro"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 24453, z = -497793, direction = 1024},
			playerCarId = 1,
			musicType = {4, 7},
		}
	},
	----------- Driver 1 freeride startpos -------------
	["Interview"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -10415, z = -22025, direction = 0},
			playerCarId = 5,
			musicType = {1}
		}
	},
	["Training"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -6785, z = 75179, direction = 2048},
			playerCarId = 5,
			musicType = {0}
		}
	},
	["NewCastle"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 19687, z = -15361, direction = 0},
			playerCarId = 5,
			musicType = {2}
		}
	},
	["Miami"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 134000, z = 420740, direction = -1024},
			playerCarId = 5,
			musicType = {3, 4}
		}
	},
	["SanFrancisco"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -389600, z = 178500, direction = 0},
			playerCarId = 5,
			musicType = {5, 6}
		}
	},
	["LosAngeles"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 376500, z = -212560, direction = 0},
			playerCarId = 5,
			musicType = {7}
		}
	},
	["NewYork"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -138000, z = -358815, direction = -1024},
			playerCarId = 5,
			musicType = {0, 1}
		}
	},
}

-------------------------------------------------------

local TestGame = {
	CameraNumber = 0,
}

TestGame.UpdateCamera = function(dt)

	if Input.JustPressed[SDL.Scancode.C] == true then
		TestGame.CameraNumber = (TestGame.CameraNumber + 1) & 1
	end

	CameraFuncs[TestGame.CameraNumber + 1](dt)
end

TestGame.Terminate = function()
	-- all other players except local player are removed
	players.RemoveAllPlayers()

	-- destroy old car
	local car = players.localPlayer.currentCar
	if car ~= nil then
		players.localPlayer.currentCar = nil
		car:Destroy()
	end
end

TestGame.StartReplay = function()

	local cityCosmetics = dofile(CurrentCityInfo.cosmetics)
	local replayStream = players.localPlayer.recordStream
	local replaySourceParams = replayStream.sourceParams

	local positionInfo = POSITION_INFO(replaySourceParams.position, replaySourceParams.rotation)
	local residentModel = replaySourceParams.model
	local modelIdx = cars:LoadModel(residentModel)

	-- create new car
	local palette = replaySourceParams.palette
	local cosmetics = cityCosmetics[residentModel + 1]
	local plcar = cars:Create(CarCosmetics(cosmetics), CarControlType.Cutscene, modelIdx, palette, positionInfo)

	local evtCb = plcar.eventCallback
	plcar.eventCallback = function(self, eventType, parameters)
		--MsgInfo("Got player event ", eventType)
		
		-- this will scale up collision verocity and response is modified
		if eventType == "CarsCollision" then
			parameters.strikeVel.value = parameters.strikeVel.value // 2
		end
		
		-- call base event function
		evtCb(self, eventType, parameters)
	end

	-- re-init stream for replay
	local newPlayer = players.CreatePlayer()

	-- replay stream has to be clonned.
	-- as soon as player is going to be removed, the replay stream will be freed.
	newPlayer:InitReplay(replayStream:Clone())
	newPlayer.currentCar = plcar
end


TestGame.Init = function(residentModel)

	-- terminate old game
	--TestGame.Terminate()
	
	--Disable ALL cones and posts
	EnableModel("Trainingcones1", false)
	EnableModel("Trainingcones2", false)
	EnableModel("Trainingcones3", false)
	EnableModel("pole1", false)
	EnableModel("pole2", false)
	EnableModel("pole3", false)
	EnableModel("pole4", false)
	EnableModel("cone_tastic1", false)
	EnableModel("cone_tastic2", false)
	EnableModel("cone_tastic3", false)
	EnableModel("cone_tastic4", false)

	world.ResetStep()

	-- add test car
	-- create car cosmetics from table
	local cityCosmetics = dofile(CurrentCityInfo.cosmetics)
	local cityStart = PlayerStartInfo[CurrentCityName][1]
	
	residentModel = residentModel or cityStart.playerCarId
	local positionInfo = cityStart.startPos
	
	-- load current city cars
	local modelIdx = cars:LoadModel(residentModel)
	
	if modelIdx ~= -1 then
		SpoolRegions(positionInfo.position)

		local palette = math.random(0, 5)
		local cosmetics = cityCosmetics[residentModel + 1]
		local plcar = cars:Create(CarCosmetics(cosmetics), CarControlType.Player, modelIdx, palette, positionInfo)

		local evtCb = plcar.eventCallback
		plcar.eventCallback = function(self, eventType, parameters)
			--MsgInfo("Got player event ", eventType)
			
			-- this will scale up collision verocity and response is modified
			if eventType == "CarsCollision" then
				parameters.strikeVel.value = parameters.strikeVel.value // 2
			end
			
			-- call base event function
			evtCb(self, eventType, parameters)
		end
		
		players.localPlayer:InitReplay(nil)
		players.localPlayer.currentCar = plcar

		--players.localPlayer.rubberbandMode = RubberbandMode.Chaser
		--players.localPlayer.rubberbandPoint = positionInfo.position
		--players.localPlayer.rubberbandPowerRatio = 4096

		local maxMusic = #cityStart.musicType
		Music.Init(CurrentCityInfo.gameId, cityStart.musicType[(CurrentSkyType % maxMusic) + 1] + 1)
		Music.FunkUpDaBGMTunez(false)
	end
end

local copState = false

TestGame.UpdateCarPads = function()
	local input = {
		accel = Input.ButtonState[SDL.Scancode.Up],
		brake = Input.ButtonState[SDL.Scancode.Down],
		wheelspin = Input.ButtonState[SDL.Scancode.LeftShift],
		handbrake = Input.ButtonState[SDL.Scancode.Space],
		steering = 0,
		fastSteer = false,
		horn = 0
	}
	
	if Input.ButtonState[SDL.Scancode.Left] then
		input.steering = -1
	end

	if Input.ButtonState[SDL.Scancode.Right] then
		input.steering = 1
	end
	
	if Input.ButtonState[SDL.Scancode.LeftControl] then
		input.fastSteer = true
	end
	
	if Input.ButtonState[SDL.Scancode.H] then
		input.horn = input.horn + 1
	end

	if Input.JustPressed[SDL.Scancode.T] then
		copState = not copState
		Music.FunkUpDaBGMTunez(copState)
	end
	
	-- update local player inputs
	players.localPlayer.input = PlayerInputData(input)
end

TestGame.IsRunning = function()
	return (players.localPlayer.currentCar ~= nil)
end

return TestGame
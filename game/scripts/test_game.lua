local cars = engine.Cars
local world = engine.World
local players = engine.Players

local PlayerStartInfo = {
	----------- Driver 2 freeride startpos -------------
	["Chicago"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 6216, z = -222456, direction = 0},
			playerCarId = 1,
		}
	},
	["Havana"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = -238668, z = -235595, direction = 1024},
			playerCarId = 1,
		}
	},
	["LasVegas"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 143285, z = 621193, direction = -2046},
			playerCarId = 1,
		}
	},
	["RioDeJaneiro"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 24453, z = -497793, direction = 1024},
			playerCarId = 1,
		}
	},
	----------- Driver 1 freeride startpos -------------
	["Interview"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -10415, z = -22025, direction = 0},
			playerCarId = 5,
		}
	},
	["Training"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -6785, z = 75179, direction = 2048},
			playerCarId = 5,
		}
	},
	["NewCastle"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 19687, z = -15361, direction = 0},
			playerCarId = 5,
		}
	},
	["Miami"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 129853, z = 421288, direction = -1024},
			playerCarId = 5,
		}
	},
	["SanFrancisco"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -454890, z = 202343, direction = 1024},
			playerCarId = 5,
		}
	},
	["LosAngeles"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = 376500, z = -212560, direction = 0},
			playerCarId = 5,
		}
	},
	["NewYork"] = {
		-- D1 default
		{
			startPos = POSITION_INFO {x = -138000, z = -358815, direction = -1024},
			playerCarId = 5,
		}
	},
}

-------------------------------------------------------

TestGame = {
	CameraNumber = 0,
}

TestGame.UpdateCamera = function(dt)

	if JustPressed[SDL.Scancode.C] == true then
		TestGame.CameraNumber = (TestGame.CameraNumber + 1) & 1
	end

	CameraFuncs[TestGame.CameraNumber + 1](dt)
end

TestGame.Terminate = function()
	-- destroy old car
	local car = players.localPlayer.currentCar
	if car ~= nil then
		car:Destroy()
		players.localPlayer.currentCar = nil
	end
end

TestGame.Init = function(residentModel)

	-- terminate old game
	--TestGame.Terminate()

	-- add test car
	-- create car cosmetics from table
	local cityCosmetics = dofile(CurrentCityInfo.cosmetics)
	local cityStart = PlayerStartInfo[CurrentCityName][1]
	
	residentModel = residentModel or cityStart.playerCarId
	local positionInfo = cityStart.startPos
	
	-- load current city cars
	local modelIdx = cars:LoadModel(residentModel)
	
	if modelIdx ~= -1 then
		local palette = math.random(0, 5)
		
		world.SpoolRegions(positionInfo.position, 1)
		
		local cosmetics = cityCosmetics[residentModel + 1]

		local plcar = cars:Create(CarCosmetics(cosmetics), 1 --[[ CONTROL_TYPE_PLAYER ]], modelIdx, palette, positionInfo)

		players.localPlayer.currentCar = plcar
	end
end

TestGame.UpdateCarPads = function()
	local input = {
		accel = ButtonState[SDL.Scancode.Up],
		brake = ButtonState[SDL.Scancode.Down],
		wheelspin = ButtonState[SDL.Scancode.LeftShift],
		handbrake = ButtonState[SDL.Scancode.Space],
		steering = 0,
		fastSteer = false,
		horn = 0
	}
	
	if ButtonState[SDL.Scancode.Left] then
		input.steering = -1
	end

	if ButtonState[SDL.Scancode.Right] then
		input.steering = 1
	end
	
	-- update local player inputs
	players.localPlayer.input = PlayerInputData(input)
end

TestGame.IsRunning = function()
	return (players.localPlayer.currentCar ~= nil)
end

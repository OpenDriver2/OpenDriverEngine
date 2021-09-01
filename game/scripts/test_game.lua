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

-- car controls
local buttonState = {
	[SDL.Scancode.Up] = false,
	[SDL.Scancode.Down] = false,
	[SDL.Scancode.Left] = false,
	[SDL.Scancode.Right] = false,
	[SDL.Scancode.Space] = false,
	[SDL.Scancode.LeftShift] = false
}

-- TODO: move to Camera.lua
local cameraAngle = 0
local headAngle = 0

local function PlaceCameraFollowCar(dt)
	local car = players.localPlayer.currentCar

	if car == nil then
		return
	end
	
	local input = {
		lookLeft = buttonState[SDL.Scancode.A],
		lookRight = buttonState[SDL.Scancode.D],
		lookBack = buttonState[SDL.Scancode.S],
	}
	
	local carCos = car.cosmetics

	-- take the base car position
	local basePos = car.i_cog_position
	local baseDir = car.i_direction
	
	local cameraPos = fix.VECTOR(0,0,0)
	
	local addDist = math.max(248, carCos.colBox.vy * 3)
	
	local maxCameraDist = carCos.colBox.vz * 2 + carCos.colBox.vy + addDist
	local carHeight = carCos.colBox.vy * -3 + 85
	
	local cameraDist = maxCameraDist
	
	local headTarget = 0
	
	if input.lookLeft then
		headTarget = -1024
	elseif input.lookRight then
		headTarget = 1024
	end
	
	-- turn head
	headAngle = headAngle + (headTarget - headAngle) * dt * 15
	
	-- smooth follow
	local angleDelta = fix.DIFF_ANGLES(cameraAngle, baseDir)
	cameraAngle = math.floor(cameraAngle + (angleDelta / 8) * dt * 25)
	
	local cameraAngleCurrent = cameraAngle + headAngle
	
	if input.lookBack then
		cameraAngle = baseDir
		cameraAngleCurrent = cameraAngle + 2048
	end
	
	local sn = math.sin(cameraAngleCurrent * fix.toRadian)
	local cs = math.cos(cameraAngleCurrent * fix.toRadian)
	
	cameraPos.vx = math.floor(basePos.vx - (cameraDist * sn))
	cameraPos.vy = math.floor(basePos.vy)
	cameraPos.vz = math.floor(basePos.vz - (cameraDist * cs))

	local camPosVy = world.MapHeight(cameraPos)
	cameraPos.vy = carHeight - basePos.vy	
	
	local cammapht = (carHeight - camPosVy) - 100 -- + gCameraOffset.vy;
	local camera_angle = vec.vec3(25, -cameraAngleCurrent, 0)

	if cameraPos.vy > cammapht then
		local height = world.MapHeight(basePos);

		if math.abs(height - camPosVy) < 900 then
			camera_angle.x = (cameraPos.vy - cammapht) / 2 + 25
			cameraPos.vy = cammapht;
		end
	end
	
	cameraPos.vy = -cameraPos.vy
	
	InitCamera({
		position = fix.FromFixedVector(cameraPos),
		angles = vec.vec3(camera_angle.x / fix.ONE * 360.0,camera_angle.y / fix.ONE * 360.0,0),
		fov = 50,
		velocity = fix.FromFixedVector(car.linearVelocity)
	})
end

local CameraFuncs = {
	PlaceCameraFollowCar,
}

-------------------------------------------------------

TestGame = {
	CameraNumber = 1,
}

TestGame.UpdateCamera = function(dt)
	CameraFuncs[TestGame.CameraNumber](dt)
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
		accel = buttonState[SDL.Scancode.Up],
		brake = buttonState[SDL.Scancode.Down],
		wheelspin = buttonState[SDL.Scancode.LeftShift],
		handbrake = buttonState[SDL.Scancode.Space],
		steering = 0,
		fastSteer = false,
		horn = 0
	}
	
	if buttonState[SDL.Scancode.Left] then
		input.steering = -1
	end

	if buttonState[SDL.Scancode.Right] then
		input.steering = 1
	end
	
	-- update local player inputs
	players.localPlayer.input = PlayerInputData(input)
end

TestGame.IsRunning = function()
	return (players.localPlayer.currentCar ~= nil)
end

TestGame.UpdateCarControls = function(num, down)
	buttonState[num] = down
end
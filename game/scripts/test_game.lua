local cars = engine.Cars
local world = engine.World

TestGame = {}

local PlayerStartInfo = {
	----------- Driver 2 freeride startpos -------------
	["Chicago"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 6216, z = -222456, direction = 0},
		}
	},
	["Havana"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = -238668, z = -235595, direction = 1024},
		}
	},
	["LasVegas"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 143285, z = 621193, direction = -2046},
		}
	},
	["RioDeJaneiro"] = {
		-- D2 default
		{
			startPos = POSITION_INFO {x = 24453, z = -497793, direction = 1024},
		}
	},
	----------- Driver 1 freeride startpos -------------
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

local car = nil

-- car controls
local buttonState = {
	[SDL.Scancode.Up] = false,
	[SDL.Scancode.Down] = false,
	[SDL.Scancode.Left] = false,
	[SDL.Scancode.Right] = false,
	[SDL.Scancode.Space] = false,
	[SDL.Scancode.LeftShift] = false
}

-- Fixes car cosmetics in REDRIVER2 way
-- TODO: move to CarCosmetics lua
local function FixCarCos(carCos)
	local delta = fix.SVECTOR(0,0,0)
	delta.vz = -(carCos.wheelDisp[1].vz + carCos.wheelDisp[2].vz - 14) / 2

	local function UpdateCarPoints()
		for i=1,12 do
			carCos.cPoints[i].vx = carCos.cPoints[i].vx + delta.vx
			carCos.cPoints[i].vy = carCos.cPoints[i].vy - delta.vy
			carCos.cPoints[i].vz = carCos.cPoints[i].vz + delta.vz
		end

		for i=1,4 do
			carCos.wheelDisp[i].vx = carCos.wheelDisp[i].vx + delta.vx
			carCos.wheelDisp[i].vy = carCos.wheelDisp[i].vy - delta.vy
			carCos.wheelDisp[i].vz = carCos.wheelDisp[i].vz + delta.vz
		end
		carCos.cog.vx = carCos.cog.vx + delta.vx;
		carCos.cog.vy = carCos.cog.vy + delta.vy;
		carCos.cog.vz = carCos.cog.vz - delta.vz;
	end
	
	UpdateCarPoints()
end

-- TODO: move to Camera.lua
local cameraAngle = 1024

local function PlaceCameraFollowCar(dt)
	if car == nil then
		return
	end
	
	local carCos = car.cosmetics

	-- take the base car position
	local basePos = car.i_cog_position
	local baseDir = car.i_direction
	
	local cameraPos = fix.VECTOR(0,0,0)
	
	local maxCameraDist = carCos.colBox.vz * 2 + carCos.colBox.vy + 248
	local carHeight = carCos.colBox.vy * -3 + 85
	
	local cameraDist = maxCameraDist
	
	-- smooth follow
	local angleDelta = fix.DIFF_ANGLES(cameraAngle, baseDir)
	cameraAngle = cameraAngle + (angleDelta / 8) * dt * 30
	
	local sn = gte.isin(cameraAngle)
	local cs = gte.icos(cameraAngle)
	
	cameraPos.vx = basePos.vx - (cameraDist * sn) / fix.ONE;
	cameraPos.vy = basePos.vy
	cameraPos.vz = basePos.vz - (cameraDist * cs) / fix.ONE;

	local camPosVy = world.MapHeight(cameraPos)
	cameraPos.vy = carHeight - basePos.vy	
	
	local cammapht = (carHeight - camPosVy) - 100 -- + gCameraOffset.vy;
	local camera_angle = fix.VECTOR(25, -cameraAngle, 0)

	if cameraPos.vy > cammapht then
		local height = world.MapHeight(basePos);

		if math.abs(height - camPosVy) < 900 then
			camera_angle.vx = (cameraPos.vy - cammapht) / 2 + 25
			cameraPos.vy = cammapht;
		end
	end
	
	cameraPos.vy = -cameraPos.vy
	
	InitCamera({
		position = fix.FromFixedVector(cameraPos),
		angles = vec.vec3(camera_angle.vx / fix.ONE * 360.0,camera_angle.vy / fix.ONE * 360.0,0),
		fov = 50
	})
end

-------------------------------------------------------

TestGame.UpdateCamera = function(dt)
	PlaceCameraFollowCar(dt)
end

TestGame.Terminate = function()
	-- destroy old car
	if car ~= nil then
		car:Destroy()
		car = nil
	end
end

TestGame.Init = function()

	-- terminate old game
	TestGame.Terminate()

	local cityStart = PlayerStartInfo[CurrentCityName][1]

	-- add test car
	-- create car cosmetics from table
	local cityCosmetics = dofile(CurrentCityInfo.cosmetics)

	local positionInfo = cityStart.startPos
	local residentModel = cityStart.playerCarId or 0
	local palette = math.random(0, 5)
	
	world.SpoolRegions(positionInfo.position, 1)
	
	-- load current city cars
	local modelIdx = cars:LoadModel(residentModel)

	car = cars:Create(CAR_COSMETICS(cityCosmetics[residentModel + 1]), 1 --[[ CONTROL_TYPE_PLAYER ]], modelIdx, palette, positionInfo)
end

TestGame.UpdateCarPads = function()
	if car == nil then
		return
	end
	
	local input = {
		accel = buttonState[SDL.Scancode.Up],
		brake = buttonState[SDL.Scancode.Down],
		wheelspin = buttonState[SDL.Scancode.LeftShift],
		handbrake = buttonState[SDL.Scancode.Space],
		steering = 0,
	}
	

	if buttonState[SDL.Scancode.Left] then
		input.steering = -1
	end

	if buttonState[SDL.Scancode.Right] then
		input.steering = 1
	end
	
	local carCos = car.cosmetics
	
	-- update steering
	
	if input.steering < 0 then
		car.wheel_angle = car.wheel_angle - 32
		car.autobrake = car.autobrake + 1
	end
	
	if input.steering > 0 then
		car.wheel_angle = car.wheel_angle + 32
		car.autobrake = car.autobrake + 1
	end

	if car.wheel_angle < -352 then
		car.wheel_angle = -352
	end
	
	if car.wheel_angle > 352 then
		car.wheel_angle = 352
	end
	
	if car.autobrake > 90 then
		car.autobrake = 90
	end
	
	if input.steering == 0 then
		if car.wheel_angle < -64 then
			car.wheel_angle = car.wheel_angle + 64
		elseif car.wheel_angle < 65 then
			car.wheel_angle = 0
		else
			car.wheel_angle = car.wheel_angle - 64
		end
		car.autobrake = 0
	end
	
	-- update acceleration
	if input.handbrake == true then
		car.handbrake = 1
	else
		car.handbrake = 0
		
		if input.wheelspin == true then
			car.wheelspin = 1
		else
			car.wheelspin = 0
		end
		
		-- continue without burnout
		if car.wheelspin ~= 0 and car.wheel_speed > 452952 then
			car.wheelspin = 0
			input.accel = true
		end
	end
	
	if input.accel then
		car.thrust = (4915 * carCos.powerRatio) + 2048 >> 12
	end

	if input.brake then
		local rws = (car.wheel_speed * 1500 // 1024) + 2048 >> 12
		if -rws < 23 then
			rws = -5000;
		else
			rws = ((rws + 278) * -4778) >> 8
		end
				
		car.thrust = -4096
	end
	
	if input.accel ~= true and input.brake ~= true then
		car.thrust = 0
	end
	
	if car.changingGear then
		car.thrust = 1;
	end
end

TestGame.UpdateCarControls = function(num, down)
	buttonState[num] = down
end
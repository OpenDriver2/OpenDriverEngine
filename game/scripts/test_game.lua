local cars = engine.Cars
local world = engine.World

TestGame = {

}

local car = nil

-- car controls
local buttonState = {
	[SDL.Scancode.Up] = false,
	[SDL.Scancode.Down] = false,
	[SDL.Scancode.Left] = false,
	[SDL.Scancode.Right] = false,
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

TestGame.Init = function()

	-- destroy old car
	if car ~= nil then
		car:Destroy()
		car = nil
	end

	-- add test car
	-- create car cosmetics from table
	--local miamiCos = dofile("scripts/driver1_miami_cosmetics.lua")
	local chicagoCos = dofile("scripts/driver2_chicago_cosmetics.lua")
	FixCarCos(chicagoCos[1])		

	--local positionInfo = POSITION_INFO {x = 6216, z = -222456, direction = 0}
	local positionInfo = POSITION_INFO {x = -59057, z = -63321, direction = 0}
	local residentModel = 1
	local palette = math.random(0, 5)
	
	-- load current city cars
	local modelIdx = cars:LoadModel(residentModel)

	car = cars:Create(CAR_COSMETICS(chicagoCos[1]), 1 --[[ CONTROL_TYPE_PLAYER ]], modelIdx, palette, positionInfo)
end

TestGame.UpdateCarPads = function()
	if car == nil then
		return
	end
	
	local carCos = car.cosmetics
	
	-- update steering
	
	if buttonState[SDL.Scancode.Left] then
		car.wheel_angle = car.wheel_angle - 32
		car.autobrake = car.autobrake + 1
	end
	
	if buttonState[SDL.Scancode.Right] then
		car.wheel_angle = car.wheel_angle + 32
		car.autobrake = car.autobrake + 1
	end
	
	car.handbrake = buttonState[SDL.Scancode.Space] == true
	
	if car.wheel_angle < -352 then
		car.wheel_angle = -352
	end
	
	if car.wheel_angle > 352 then
		car.wheel_angle = 352
	end
	
	if car.autobrake > 90 then
		car.autobrake = 90
	end
	
	if buttonState[SDL.Scancode.Left] ~= true and buttonState[SDL.Scancode.Right] ~= true then
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
	if buttonState[SDL.Scancode.Up] then
		car.thrust = (4915 * carCos.powerRatio) / fix.ONE
	end

	if buttonState[SDL.Scancode.Down] then
		car.thrust = -4096
	end
	
	if buttonState[SDL.Scancode.Up] ~= true and buttonState[SDL.Scancode.Down] ~= true then
		car.thrust = 0
	end
	
	if car.changingGear then
		car.thrust = 1;
	end
end

TestGame.UpdateCarControls = function(num, down)
	buttonState[num] = down
end
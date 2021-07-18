local cars = engine.Cars
local world = engine.World

local car
local carCos

function FixCarCos(carCos)
	local delta = fix.SVECTOR(0,0,0)
	delta.vy = -(carCos:wheelDisp(1).vz + carCos:wheelDisp(2).vz - 14) / 2

	local function UpdateCarPoints()
		for i=1,12 do
			carCos:cPoints(i).vx = carCos:cPoints(i).vx + delta.vx
			carCos:cPoints(i).vy = carCos:cPoints(i).vy - delta.vy
			carCos:cPoints(i).vz = carCos:cPoints(i).vz + delta.vz
		end

		for i=1,4 do
			carCos:wheelDisp(i).vx = carCos:wheelDisp(i).vx + delta.vx
			carCos:wheelDisp(i).vy = carCos:wheelDisp(i).vy - delta.vy
			carCos:wheelDisp(i).vz = carCos:wheelDisp(i).vz + delta.vz
		end
		carCos.cog.vx = carCos.cog.vx + delta.vx;
		carCos.cog.vy = carCos.cog.vy + delta.vy;
		carCos.cog.vz = carCos.cog.vz - delta.vz;
	end
	
	UpdateCarPoints()
end

function InitTestGame()

	-- load current city cars
	cars:LoadModel(1)
	
	-- add test car
	carCos = CAR_COSMETICS.new()
	carCos.headLight = fix.SVECTOR(93,14,-351)
	carCos.frontInd = fix.SVECTOR(110,16,-340)
	carCos.backInd = fix.SVECTOR(55,32,368)
	carCos.brakeLight = fix.SVECTOR(68,33,368)
	carCos.revLight = fix.SVECTOR(57,35,359)
	carCos.policeLight = fix.SVECTOR(34,0,0)
	carCos.exhaust = fix.SVECTOR(68,68,359)
	carCos.smoke = fix.SVECTOR(0,-4,-335)
	carCos.fire = fix.SVECTOR(0,-4,-341)
	
	carCos:setWheelDisp(1, fix.SVECTOR(128,-30,217))
	carCos:setWheelDisp(2, fix.SVECTOR(128,-30,-181))
	carCos:setWheelDisp(3, fix.SVECTOR(-129,-30,217))
	carCos:setWheelDisp(4, fix.SVECTOR(-129,-30,-181))
	
	carCos.extraInfo = -31720 -- FLAGS
	carCos.powerRatio = 4096
	carCos.cbYoffset = 0
	carCos.susCoeff = 4096
	carCos.traction = 4096
	carCos.wheelSize = 52
	
	carCos:setcPoints(1, fix.SVECTOR(-126, 11, -370))
	carCos:setcPoints(2, fix.SVECTOR(125, 11, -370))
	carCos:setcPoints(3, fix.SVECTOR(-126, 2, 370))
	carCos:setcPoints(4, fix.SVECTOR(125, 2, 370))
	carCos:setcPoints(5, fix.SVECTOR(-126, 89, -367))
	carCos:setcPoints(6, fix.SVECTOR(125, 89, -367))
	carCos:setcPoints(7, fix.SVECTOR(-83, 163, -124))
	carCos:setcPoints(8, fix.SVECTOR(82, 163, -124))
	carCos:setcPoints(9, fix.SVECTOR(-83, 164, 52))
	carCos:setcPoints(10, fix.SVECTOR(82, 164, 52))
	carCos:setcPoints(11, fix.SVECTOR(-126, 91, 371))
	carCos:setcPoints(12, fix.SVECTOR(125, 91, 371))
	
	carCos.colBox = fix.SVECTOR(129,84,375)
	carCos.cog = fix.SVECTOR(0,-85,11)
	carCos.twistRateX = 224
	carCos.twistRateY = 224
	carCos.twistRateZ = 1120
	carCos.mass = 4096
	
	FixCarCos(carCos)

	local positionInfo = POSITION_INFO {x = 3166, z = -11787, direction = 0}
	
	local model = 0
	local palette = 1
	
	car = cars:Create(carCos, 1 --[[ CONTROL_TYPE_PLAYER ]], model, palette, positionInfo)
end

local cameraAngle = 1024

local buttonState = {
	[SDL.Scancode.Up] = false,
	[SDL.Scancode.Down] = false,
	[SDL.Scancode.Left] = false,
	[SDL.Scancode.Right] = false,
}

function UpdateCarPads()

	-- update steering
	
	if buttonState[SDL.Scancode.Left] then
		car.wheel_angle = car.wheel_angle - 32
		car.autobrake = car.autobrake + 1
	end
	
	if buttonState[SDL.Scancode.Right] then
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

function PlaceCameraFollowCar()
	local basePos = car.cog_position
	local direction = car.direction
	
	local cameraPos = fix.VECTOR(0,0,0)
	
	local maxCameraDist = carCos.colBox.vz * 2 + carCos.colBox.vy + 248
	local carHeight = carCos.colBox.vy * -3 + 85
	
	local cameraDist = maxCameraDist
	
	-- smooth follow
	local angleDelta = fix.DIFF_ANGLES(cameraAngle, direction)
	cameraAngle = cameraAngle + (angleDelta / 8)
	
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

function UpdateCarControls(num, down)
	buttonState[num] = down
end
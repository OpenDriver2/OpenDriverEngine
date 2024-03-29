local players = engine.Players					-- local players
local world = engine.World						-- collision and rendering world
local camera = engine.Camera

local cameraAngle = 0
local headAngle = 0

local function GetCameraInput()
	local input = {
		lookLeft = Input.ButtonState[SDL.Scancode.A],
		lookRight = Input.ButtonState[SDL.Scancode.D],
		lookBack = Input.ButtonState[SDL.Scancode.S],
	}
	return input
end

local function TurnHead(input, dt)
	local headTarget = 0
	
	if input.lookLeft then
		headTarget = -1024
	elseif input.lookRight then
		headTarget = 1024
	end
	
	-- turn head
	headAngle = headAngle + (headTarget - headAngle) * dt * 15
end

local function PlaceCameraInCar(dt)
	local car = players.localPlayer.currentCar

	if car == nil then
		return
	end
	
	local input = GetCameraInput()
	TurnHead(input, dt)
	
	local carCos = car.cosmetics

	-- take the base car position
	local basePos = car.i_cogPosition
	local baseDir = car.i_direction
	local baseRot = car.i_drawMatrix
	
	local colBox = fix.FromFixedVector(carCos.colBox)
	local _headAngle = headAngle
	
	if input.lookBack then
		_headAngle = 2048
		colBox.z = -colBox.z
	end
	
	local rotationY = vec.mat3.rotationY(math.rad(-_headAngle / fix.ONE * 360.0))
	local camRotation = (baseRot * rotationY)
	local camEulerAngles = camRotation:eulersZXY()
	
	local camOffset = baseRot:transformVecInv(vec.vec3(0,colBox.y * 0.5, colBox.z - 0.01))
	
	local cameraPos = fix.FromFixedVector(basePos) + camOffset
	local camera_angle = vec.vec3(math.deg(camEulerAngles.x), math.deg(camEulerAngles.y), math.deg(camEulerAngles.z))
	
	InitCamera({
		position = cameraPos,
		angles = camera_angle,
		fov = 50,
		velocity = fix.FromFixedVector(car.linearVelocity)
	})
end

local function PlaceCameraFollowCar(dt)
	local car = players.localPlayer.currentCar

	if car == nil then
		return
	end
	
	local input = GetCameraInput()
	TurnHead(input, dt)
	
	local carCos = car.cosmetics

	-- take the base car position
	local basePos = car.i_cogPosition
	local baseDir = car.i_direction
	
	local cameraPos = fix.VECTOR(0,0,0)
	
	local addDist = math.max(248, carCos.colBox.vy * 3)
	local carHeight = math.max(-375, -carCos.colBox.vy * 3 + 85);

	-- TEMP
	local gCameraDefaultScrZ = 256

	-- determine aspect ratio of the car
	local aspectY = (carCos.colBox.vy * 256) / fix.ONE;
	local aspectZ = (carCos.colBox.vz * 256) / fix.ONE;

	local A = (aspectY * (aspectZ / 2));
		
	-- Fireboyd78 camera code from ReD2
	-- adjust for especially taller/longer cars
	if A >= carCos.colBox.vx then
		local B = aspectZ / (aspectY * 2);
		local C = A * B

		local camDist = (aspectY * aspectZ)
		local camAdjust = 0;

		if C > addDist then
			-- long vehicle but not very tall;
			-- move the camera closer and lower it a bit
			addDist = addDist - (C - addDist) * 2;
			carHeight = carHeight - (A / 2);
		elseif (C < carCos.colBox.vy) then
			-- tall vehicle but not very long;
			-- move up closer in respect to camera aspect
			addDist = addDist - (camDist - gCameraDefaultScrZ);
		else
			-- deal with very large vehicles (like buses)
			if camDist < -carHeight + 100 then
				-- long and tall, but still fits within view
				-- move up closer in respect to camera/vehicle aspect
				addDist = addDist - (camDist - (gCameraDefaultScrZ / B));
			else
				-- absolutely massive, doesn't fit within view!
				-- move up closer; adjust height in respect to camera aspect
				addDist = addDist - (camDist / (1 + B));
				carHeight = carHeight - (A - gCameraDefaultScrZ) / 2;
			end
		end
	end

	local carSize = carCos.colBox.vz * 2 + carCos.colBox.vy;
	local maxCameraDist = carSize + addDist;
	
	-- TODO: check collision
	local cameraDist = maxCameraDist

	-- smooth follow
	local angleDelta = fix.DIFF_ANGLES(cameraAngle, baseDir)
	cameraAngle = (cameraAngle + (angleDelta / 8) * dt * 25)
	
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

CameraFuncs = {
	PlaceCameraFollowCar,
	PlaceCameraInCar
}

--
-- InitCamera : updates the game engine main camera with specified parameters
-- 		@params: 	a table of { position: vec.vec3, angles: vec.vec3, fov: float}
--
function InitCamera( params )
	-- you can't replace MainView but we can copy all parameters
	camera.MainView.position = params.position
	camera.MainView.angles = params.angles
	camera.MainView.fov = params.fov

	if params.velocity ~= nil then
		camera.MainViewVelocity:set(params.velocity)
	else
		camera.MainViewVelocity:set(0.0)
	end
end

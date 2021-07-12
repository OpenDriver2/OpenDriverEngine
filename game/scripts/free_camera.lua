local world = engine.World						-- collision and rendering world

FreeCamera = {
	MoveDir = vec.vec3(0),
	Position = vec.vec3(0),
	Velocity = vec.vec3(0),
	Angles = vec.vec3(0),
	FOV = 75,
	Boost = false,
	Look = false
}

local CAMERA_MOVEMENT_SPEED_FACTOR = 5.0	-- in-game units per second (one unit is 4096)
local CAMERA_MOVEMENT_ACCELERATION = 10
local CAMERA_MOVEMENT_DECELERATION = 40

---------------------------------------------------------
-- Updates camera movement for level viewer
---------------------------------------------------------
FreeCamera.UpdateCameraMovement = function(deltaTime)

	local speedModifier = iif(FreeCamera.Boost, 5.0, 1.0)

	forward, right = vec.AngleVectors(FreeCamera.Angles)

	local moveDir = FreeCamera.MoveDir
	local maxSpeed = CAMERA_MOVEMENT_SPEED_FACTOR * speedModifier;
	local speed = FreeCamera.Velocity:length()

	if moveDir:lengthSqr() > 0.1 and speed < maxSpeed then
		FreeCamera.Velocity = FreeCamera.Velocity + right * vec.vec3(moveDir.x * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier)
		FreeCamera.Velocity = FreeCamera.Velocity + forward * vec.vec3(moveDir.z * deltaTime * CAMERA_MOVEMENT_ACCELERATION * speedModifier)
	else
		if speed < 1.0 then
			speed = 1.0
		end

		FreeCamera.Velocity = FreeCamera.Velocity - ((FreeCamera.Velocity / vec.vec3(speed)) * vec.vec3(CAMERA_MOVEMENT_DECELERATION * deltaTime))
	end

	FreeCamera.Position = FreeCamera.Position + FreeCamera.Velocity * vec.vec3(deltaTime)

	local cameraPosition = fix.ToFixedVector(FreeCamera.Position)

	local height = world.MapHeight(cameraPosition)

	if cameraPosition.vy < height then
		cameraPosition.vy = height;
		FreeCamera.Position.y = height / fix.ONE;
	end
end

FreeCamera.MouseMove = function(xrel, yrel)
	if FreeCamera.Look then
		FreeCamera.Angles.x = FreeCamera.Angles.x + yrel * 0.25;
		FreeCamera.Angles.y = FreeCamera.Angles.y - xrel * 0.25;
	end
end

FreeCamera.KeyPress = function(num, down)
	if num == SDL.Scancode.W then
		FreeCamera.MoveDir.z = iif(down, 1, 0)
	end
	if num == SDL.Scancode.S then
		FreeCamera.MoveDir.z = iif(down, -1, 0)
	end
	if num == SDL.Scancode.D then
		FreeCamera.MoveDir.x = iif(down, 1, 0)
	end
	if num == SDL.Scancode.A then
		FreeCamera.MoveDir.x = iif(down, -1, 0)
	end
	
	if num == SDL.Scancode.LeftShift then
		FreeCamera.Boost = down
	end
end
local world = engine.World						-- collision and rendering world

------------------------------------------------------------------------
-- LIFTING BRIDGES

local LiftingBridges = {
	-- 1
    {
        cellList = {0, 1},
        direction = 0,
        positionX = { -182784, -175616, -168448},
        positionZ = { -227328, -162304, -141824, -121344, -100864, -80384, -59904},
    },

	-- goose island start

	-- 2
    {
        cellList = {2, 3},
        direction = 0,
        positionX = { -312832, -305664, -298496 },
        positionZ = { 324096 },
    },

	-- 3
    {
        cellList = {4, 5},
        direction = 0,
        positionX = { -311808, -304640, -297472 },
        positionZ = { 247296 },
    },

	-- 4
    {
        cellList = {6, 7},
        direction = 0,
        positionX = { -256512, -249344, -242176 },
        positionZ = { 247296 },
    },

	-- 5
    {
        cellList = {8, 9},
        direction = 0,
        positionX = { -262656, -255488, -248320 },
        positionZ = { 324096, }
    },

	-- 6
    {
        cellList = {10, 11},
        direction = 1,
        positionZ = { 170496, 177664, 184832 },
        positionX = { -271360 }
    },

    -- goose island end

	-- 7
    {
        cellList = {12, 13},
        direction = 1,
        positionZ = { -12800, -5632, 1536 },
        positionX = { -162304, -102912, -82432, -61952, -39936 }
    },

	-- 8
    {
        cellList = {14, 15},
        direction = 1,
        positionZ = { -6656, 512, 7680 },
        positionX = { 4137, 27648, 128000 }
    }
}

local CainesCompoundBridges = {
    2,3,4,5
}

local function BridgeTime(n)
    local rnd = 28403 -- Random2(0); this is how D2 random works!!!
    local timeOffset = (rnd >> (n & 31) & 255) * 32
    return timeOffset % 8000
end

local function GetBridgeRotation(timer)
	if timer > 2600 then
		return 0
    end

	if timer > 1600 then
		timer = 2600 - timer
	elseif timer > 1000 then
		timer = 1000
    end

	return 800 * (4096 - gte.icos((timer * 2048) // 1000)) // 8192
end

local function MakeBridges()
    for bridgeNum,bridgesInfo in ipairs(LiftingBridges) do
        bridgesInfo.alwaysOn = true
        bridgesInfo.lists = {}
        bridgesInfo.timer = BridgeTime(bridgeNum-1) % 8000
        for i,cellList in ipairs(bridgesInfo.cellList) do

            --local eventPlaneHandler = world.CreateEventPlane(cellList)
            local cellListHandler = world.CreateCellList(cellList)

            if bridgesInfo.direction == 0 then
                cellListHandler.position = fix.VECTOR(bridgesInfo.positionX[1 + (i-1)*2], 0, bridgesInfo.positionZ[1])
            else
                cellListHandler.position = fix.VECTOR(bridgesInfo.positionX[1], 0, bridgesInfo.positionZ[1 + (i-1)*2])
            end
            cellListHandler.pivotMatrix = vec.mat4.translate(fix.FromFixedVector(cellListHandler.position))

            table.insert( bridgesInfo.lists, cellListHandler )
        end
    end
end

local function UpdateBridges()
    --  rotate!
    for _,bridgesInfo in ipairs(LiftingBridges) do
        for i,cellList in ipairs(bridgesInfo.lists) do

            local sign = if_then_else(i > 1, -1, 1) * if_then_else(bridgesInfo.direction == 0, -1, 1)
            local rotation = sign * GetBridgeRotation(bridgesInfo.timer)

            if bridgesInfo.direction == 0 then
                cellList.rotation = fix.VECTOR(0, 0, rotation)
            else
                cellList.rotation = fix.VECTOR(rotation, 0, 0)
            end

            if bridgesInfo.alwaysOn or bridgesInfo.timer <= 1000 then
                bridgesInfo.timer = (bridgesInfo.timer + 1) % 8000
            end

            -- TODO: add plane computations
        end
    end
end

------------------------------------------------------------------------
-- FERRIS WHEEL

local FerrisWheel = {
    position = fix.VECTOR(195264, 3728, 74752),
    hubModel = nil,
    carModel = nil,
    frameModel = nil,

    rotation = 0,
}

local function InitFerrisWheel()
    FerrisWheel.hubModel = world.GetModelByName("HUB")
    FerrisWheel.carModel = world.GetModelByName("CAR")
    FerrisWheel.frameModel = world.GetModelByName("FRAME")


end

local function UpdateFerrisWheel()

    
end

local function DrawFerrisWheel(dt)
    FerrisWheel.rotation = FerrisWheel.rotation + 32 * dt

    local spoke = {
		vec.vec3(0, 0, 2677 / fix.ONE),
		vec.vec3(0, 2677 / fix.ONE, 0)
	}

    -- draw ferris cars
    for i=0,4 do
        local angle = FerrisWheel.rotation + i * 410

		local sn = vec.vec3(math.sin(angle * fix.toRadian))
		local cs = vec.vec3(math.cos(angle * fix.toRadian))
        
        local offset = vec.vec3(0)
		offset = spoke[1] * sn + spoke[2] * cs;

        world.AddDrawable(DRAWABLE({
            position = fix.FromFixedVector(FerrisWheel.position) + offset,
            angles = vec.vec3(0.0,0.0,0.0),
            model = FerrisWheel.carModel.index,
        }))

        -- draw car on other side
        world.AddDrawable(DRAWABLE({
            position = fix.FromFixedVector(FerrisWheel.position) - offset,
            angles = vec.vec3(0.0,0.0,0.0),
            model = FerrisWheel.carModel.index,
        }))
    end

    -- draw hub
    world.AddDrawable(DRAWABLE({
        position = fix.FromFixedVector(FerrisWheel.position),
        angles = vec.vec3(0.0,0.0,0.0),
        model = FerrisWheel.hubModel.index,
    }))
    
    local rotationRad = -FerrisWheel.rotation * fix.toRadian

    -- draw wheel
    world.AddDrawable(DRAWABLE({
        position = fix.FromFixedVector(FerrisWheel.position),
        angles = vec.vec3(rotationRad,0.0,0.0),
        model = FerrisWheel.frameModel.index,
    }))

    -- add another wheel instance
    world.AddDrawable(DRAWABLE({
        position = fix.FromFixedVector(FerrisWheel.position),
        angles = vec.vec3(rotationRad,0.0,0.0),
        model = FerrisWheel.frameModel.index,
        scale = vec.vec3(-1, 1, -1)
    }))
end

------------------------------------------------------------------------

local function Terminate()
end

local function TriggerEvent(name)
end

return {
    Initialize = function()
        MakeBridges()
        InitFerrisWheel()
    end,
    Terminate = Terminate,

    Trigger = TriggerEvent,
    Step = function()
        UpdateBridges()
        UpdateFerrisWheel()
    end,
    Draw = DrawFerrisWheel
}
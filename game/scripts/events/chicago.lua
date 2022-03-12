local world = engine.World						-- collision and rendering world

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
    local rnd = 28403 -- Random2(0);
    local timeOffset = (rnd >> (n & 31) & 255) * 32;
    return timeOffset % 8000
end

local function MakeBridges()
    for _,bridgesInfo in ipairs(LiftingBridges) do
        bridgesInfo.lists = {}
        for i,cellList in ipairs(bridgesInfo.cellList) do

            local cellListHandler = world.CreateCellList(cellList)
            cellListHandler.rotation = fix.VECTOR(0, 0, 0)

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
            local sign = if_then_else(i > 1, -1, 1)
            if bridgesInfo.direction == 0 then
                cellList.rotation = fix.VECTOR(0, 0, sign * gte.isin(world.StepCount() * 4) // 4)
            else
                cellList.rotation = fix.VECTOR(sign * gte.isin(world.StepCount() * 4) // 4, 0, 0)
            end
        end
    end
end

local function Terminate()
end

local function TriggerEvent(name)
end

return {
    Initialize = MakeBridges,
    Terminate = Terminate,

    Trigger = TriggerEvent,
    Step = UpdateBridges
}
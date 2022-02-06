local world = engine.World						-- collision and rendering world

local trainModel
local truckModel

--
-- InitEventModels: initializes models
--
local function InitEventModels()
	trainModel = world.GetModelByName("ELTRAIN")
	truckModel = world.GetModelByName("LORRY")
	
	-- model properties
	--[[
	truckModel.animateInfo = ANIMATED_OBJECT {
		glowType = 1,
		glowSize = 125 / fix.ONE,
		lightRadius = 512 / fix.ONE,
		color = vec.vec3(1,1,1),
		position = fix.VECTOR(e,t,c),
	}]]
end

--
-- AddEventObjects: adding event object to rendering system and collision
--
local function AddEventObjects()
	-- delete old event objects
	world.PurgeCellObjects()
	
	-- Chicago ONLY
	if trainModel then
	
		-- push event cell object
		-- any collision checks afterwards will have an effect with it
		world.PushCellObject(CELL_OBJECT { position = fix.VECTOR(4749, -250, -11082), yang = 512 // 64, type = trainModel.index })
	
	end
	
	if truckModel then
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2579, -300, -14235), 1024 // 64, truckModel.index))
		world.PushCellObject(CELL_OBJECT(fix.VECTOR(2434, -300, -9641), 128 // 64, truckModel.index))
	end
end

return {
	InitEventModels = InitEventModels,
	AddEventObjects = AddEventObjects
}
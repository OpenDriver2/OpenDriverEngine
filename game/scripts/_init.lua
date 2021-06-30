-- state machine setup

dofile "city.lua"
dofile "updates.lua"

-- "engine" namespace has everything dynamically updated
-- "driver" & "driver2" namespace has every class to be used with "engine"

local players = engine.Players				-- local players
local cars = engine.Cars					-- cars, handling
local peds = engine.Pedestrians				-- pedestrians and motion capture system
local world = engine.World					-- collision and rendering world
local sky = engine.Sky						-- Sky renderer

function StepSim(dt)
	-- TODO: direct port from MAIN.C
	-- 
	-- events:Update()
	-- mission:Update()
	-- civAI:PingIn()
	cars:UpdateControl()	-- controls and replay
	
	-- cops.Update()
	-- peds.Update()

	cars:GlobalTimeStep( dt )
	--players:Update()
	cars:DoScenaryCollisions()
	--players:CheckMiscFelonies()
end

-- called from C++ to test everything
function StartTestGame()
	-- We are going to load Driver 2 level TODO: factory
	local level = driver2.Level.new()

	CurrentCityInfo = CityInfo["Chicago"]
	CurrentCityType = CityType.Day

	-- set level to be used for rendering, collision etc
	world:SetLevel(level)

	-- load a level file
	if level:Load( CurrentCityInfo.levPath[CurrentCityType] ) then
		-- load sky
		sky:Load( CurrentCityInfo.skyPath, 0 )
		cars:Load( CurrentCityInfo, level )
	end
	
	-- create a car and a player
	local model = 0
	local palette = 2
		
	positionInfo = {X = 200000, Z = 300000, dir = 2048} -- X, Z, direction. Not specifying Y will place car at level.MapHeight(XZ)
		
	local car = cars:Create(model, palette, CONTROL_TYPE_PLAYER, positionInfo)

	-- create player at slot 0
	local localPlayer = players:InitWithCar(0, car)
	
	players:SetLocalPlayer(localPlayer)
	
	SetUpdateFunc("StepSim", StepSim)
end

--------------------------------------------------

-- This function is called every frame
function Sys_Frame( dt )

	DoUpdateFuncs(dt)
	
end
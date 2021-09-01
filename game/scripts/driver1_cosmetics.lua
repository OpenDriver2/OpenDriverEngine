local carsCommon = dofile("scripts/cars_common.lua")

local cars = engine.Cars						-- cars, handling

local bank_index = 0

local bank = LoadSoundbank("CarBankD1_"..bank_index, {
	revSample = "voices/Bank_"..bank_index.."/1.wav",
	idleSample = "voices/Bank_"..bank_index.."/2.wav",
	hornSample = "voices/Bank_"..bank_index.."/8.wav",
})

-- they are all equal in D1 PSX
local D1_collPoints = {                      
	fix.SVECTOR(185, -80, -520),
	fix.SVECTOR(-185, -80, -520),
	fix.SVECTOR(185, -80, 520),
	fix.SVECTOR(-185, -80, 520),
	fix.SVECTOR(185, 40, -520),
	fix.SVECTOR(-185, 40, -520),
	fix.SVECTOR(150, 130, -180),
	fix.SVECTOR(-150, 130, -180),
	fix.SVECTOR(150, 130, 70),
	fix.SVECTOR(-150, 130, 70),
	fix.SVECTOR(185, 40, 520),
	fix.SVECTOR(-185, 40, 520),
}

local cityCosmeticStart = 0
		
if CurrentCityInfo.number == 1 then
	cityCosmeticStart = 8
elseif CurrentCityInfo.number == 2 then
	cityCosmeticStart = 16
elseif CurrentCityInfo.number == 3 then
	cityCosmeticStart = 24
elseif CurrentCityInfo.number == 5 then
	cityCosmeticStart = 32
elseif CurrentCityInfo.number == 7 then
	cityCosmeticStart = 40
end

-- load 8 cosmetics into lua
local allCosmetics = {}

for i=0,7 do
	local cosmetic = cars:LoadCosmeticsFileD1(CurrentCityInfo.lcf, cityCosmeticStart + i)
	if cosmetic ~= nil then
		local tabl = cosmetic:ToTable()
		
		tabl.gears = carsCommon.GearDesc[1]
		tabl.handlingType = carsCommon.HandlingType[1]
		tabl.gravity = carsCommon.Gravity[1]
		
		local fix_wheels = 95
		
		tabl.cPoints = D1_collPoints
		tabl.revSample = bank.revSample
		tabl.idleSample = bank.idleSample
		tabl.hornSample = bank.hornSample
		tabl.cog = fix.SVECTOR(0,-fix_wheels,0)
		tabl.colBox = fix.SVECTOR(120*1.5,82*1.5,360*1.5)
		
		-- Hardcoded Miami superfly
		if CurrentCityInfo.number == 0 and i == 3 then
			tabl.powerRatio = 5833 * 19 + 8 >> 4;
		else
			tabl.powerRatio = 5833
		end
		
		for i=1,12 do
			tabl.cPoints[i].vx = D1_collPoints[i].vx
			tabl.cPoints[i].vy = D1_collPoints[i].vy + fix_wheels // 10
			tabl.cPoints[i].vz = D1_collPoints[i].vz
		end
		
		-- D1 has messed wheels
		tabl.wheelDisp[1].vz = tabl.wheelDisp[3].vz
		tabl.wheelDisp[2].vz = tabl.wheelDisp[4].vz
		
		for i=1,4 do
			tabl.wheelDisp[i].vy = tabl.wheelDisp[i].vy + fix_wheels -- Game code: -165 + fix_wheels
		end
		
		if i > 4 then
			allCosmetics[i - 4] = tabl
		else
			allCosmetics[i + 6] = tabl
		end
		
		--table.insert(allCosmetics, tabl)
	end
end

return allCosmetics

--[[

-- D1 PSX has 48 car cosmetics in total for 5 cities
local playerCar0Cosmetics = {
	gears = carsCommon.GearDesc[1],
	handlingType = carsCommon.HandlingType[1],
	gravity = carsCommon.Gravity[1],
	headLight = fix.SVECTOR(93,14,-351),
	frontInd = fix.SVECTOR(110,16,-340),
	backInd = fix.SVECTOR(55,32,368),
	brakeLight = fix.SVECTOR(68,33,368),
	revLight = fix.SVECTOR(57,35,359),
	policeLight = fix.SVECTOR(34,0,0),
	exhaust = fix.SVECTOR(68,68,359),
	smoke = fix.SVECTOR(0,-4,-335),
	fire = fix.SVECTOR(0,-4,-341),
	wheelDisp = {
		fix.SVECTOR(120,-30,230),
		fix.SVECTOR(120,-30,-165),
		fix.SVECTOR(-120,-30,230),
		fix.SVECTOR(-120,-30,-165),
	},
	wheelspinMaxSpeed = 663552,
	extraInfo = -31720, -- FLAGS
	powerRatio = 5833, -- or 6000 for superfly
	cbYoffset = 0,
	susCoeff = 4096,
	susCompressionLimit = 60,	-- 42 in Driver 2
	susTopLimit = 32767,		-- unlimited, 800 in Driver 2
	traction = 4096,
	wheelSize = 70,
	cPoints = D1_collPoints,
	colBox = fix.SVECTOR(120,82,360),
	cog = fix.SVECTOR(0,-125,-45),
	twistRateX = 200,	-- not used
	twistRateY = 110,
	twistRateZ = 550,
	mass = 4096,
	revSample = bank.revSample,
	idleSample = bank.idleSample,
	hornSample = bank.hornSample
}

-- temporary
local function ScaleUp(carCos)

	for i=1,12 do
		carCos.cPoints[i].vx = carCos.cPoints[i].vx * 1.25
		carCos.cPoints[i].vy = carCos.cPoints[i].vy * 1.25
		carCos.cPoints[i].vz = carCos.cPoints[i].vz * 1.25
	end

	for i=1,4 do
		carCos.wheelDisp[i].vx = carCos.wheelDisp[i].vx * 1.5
		carCos.wheelDisp[i].vy = carCos.wheelDisp[i].vy * 1.5
		carCos.wheelDisp[i].vz = carCos.wheelDisp[i].vz * 1.5
	end
	
	carCos.colBox.vx = carCos.colBox.vx * 1.5
	carCos.colBox.vy = carCos.colBox.vy * 1.5
	carCos.colBox.vz = carCos.colBox.vz * 1.5
end

ScaleUp(playerCar0Cosmetics)

-- basicmcnally, all cosmetics in D1 are EQUAL!
return {
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
	playerCar0Cosmetics,
}]]
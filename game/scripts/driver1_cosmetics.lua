local carsCommon = dofile("scripts/cars_common.lua")

local bank_index = 0

local bank = LoadSoundbank("CarBankD1_"..bank_index, {
	revSample = "voices/Bank_"..bank_index.."/1.wav",
	idleSample = "voices/Bank_"..bank_index.."/2.wav",
	hornSample = "voices/Bank_"..bank_index.."/8.wav",
})

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
		fix.SVECTOR(120,-30,217),
		fix.SVECTOR(120,-30,-181),
		fix.SVECTOR(-120,-30,217),
		fix.SVECTOR(-120,-30,-181),
	},
	wheelspinMaxSpeed = 663552,
	extraInfo = -31720, -- FLAGS
	powerRatio = 6000,
	cbYoffset = 0,
	susCoeff = 4096,
	susCompressionLimit = 60,	-- 42 in Driver 2
	susTopLimit = 32767,		-- unlimited, 800 in Driver 2
	traction = 4096,
	wheelSize = 70,
	cPoints = {
		fix.SVECTOR(-126, 11, -370),
		fix.SVECTOR(125, 11, -370),
		fix.SVECTOR(-126, 2, 370),
		fix.SVECTOR(125, 2, 370),
		fix.SVECTOR(-126, 89, -367),
		fix.SVECTOR(125, 89, -367),
		fix.SVECTOR(-83, 163, -124),
		fix.SVECTOR(82, 163, -124),
		fix.SVECTOR(-83, 164, 52),
		fix.SVECTOR(82, 164, 52),
		fix.SVECTOR(-126, 91, 371),
		fix.SVECTOR(125, 91, 371),
	},
	colBox = fix.SVECTOR(120,76,355),
	cog = fix.SVECTOR(0,-120,-5),
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
		carCos.cPoints[i].vx = carCos.cPoints[i].vx * 1.5
		carCos.cPoints[i].vy = carCos.cPoints[i].vy * 1.5
		carCos.cPoints[i].vz = carCos.cPoints[i].vz * 1.5
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
}
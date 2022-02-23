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
		
		local fix_wheels = 0 --95
		
		tabl.cPoints = D1_collPoints
		tabl.revSample = bank.revSample
		tabl.idleSample = bank.idleSample
		tabl.hornSample = bank.hornSample
		tabl.cog = fix.SVECTOR(0,-fix_wheels,0)
		tabl.colBox = fix.SVECTOR(185,130,520)
		
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
		
		local wheelsHeight = -165 -- -245 with stilts cheat

		for i=1,4 do
			--tabl.wheelDisp[i].vy = tabl.wheelDisp[i].vy + fix_wheels
			tabl.wheelDisp[i].vy = wheelsHeight + fix_wheels
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

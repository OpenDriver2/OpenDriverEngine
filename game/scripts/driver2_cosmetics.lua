local carsCommon = dofile("scripts/cars_common.lua")

local cars = engine.Cars						-- cars, handling

local function MapCarIndexToBank(model)

	if model > 4 then
		model = model - 3
	end
	
	local car_banks = {
		-- customized by SoapyMan
		{	13,	12,	4, 	13, 5,	16,	18,	2,	15, 10	},
		{	7,	10,	11, 7, 	6,	17,	17,	2,	5,	4	},
		{	8,	14,	3, 	4, 	6,	11,	9,	15,	5,	13	},
		{	13,	8, 	7, 	13, 9,	2,	17,	17,	11, 15	},
		
		--[[{13,12, 4, 13, 5, 16, 9, 2, 15, 10},
        {7,	10, 11, 7, 6, 17, 17, 2, 5, 4},
        {8,	14, 3, 4, 6, 11, 9, 2, 17, 10},
        {13,8, 7, 13, 9, 2, 17, 17, 11, 16},]]
	}
	
	return car_banks[CurrentCityInfo.number+1][model+1]
end

-- Fixes car cosmetics in REDRIVER2 way
-- TODO: move to CarCosmetics lua
local function FixCarCos(carCos)
	local delta = fix.SVECTOR(0,0,0)
	delta.vz = -(carCos.wheelDisp[1].vz + carCos.wheelDisp[2].vz - 14) / 2

	local function UpdateCarPoints()
		for i=1,12 do
			carCos.cPoints[i].vx = carCos.cPoints[i].vx + delta.vx
			carCos.cPoints[i].vy = carCos.cPoints[i].vy - delta.vy
			carCos.cPoints[i].vz = carCos.cPoints[i].vz + delta.vz
		end

		for i=1,4 do
			carCos.wheelDisp[i].vx = carCos.wheelDisp[i].vx + delta.vx
			carCos.wheelDisp[i].vy = carCos.wheelDisp[i].vy - delta.vy
			carCos.wheelDisp[i].vz = carCos.wheelDisp[i].vz + delta.vz
		end
		carCos.cog.vx = carCos.cog.vx + delta.vx;
		carCos.cog.vy = carCos.cog.vy + delta.vy;
		carCos.cog.vz = carCos.cog.vz - delta.vz;
	end
	
	UpdateCarPoints()
end

-- load all 13 cosmetics into lua
local allCosmetics = {}
for i=0,12 do
	local cosmetic = cars:LoadCosmeticsFileD2(CurrentCityInfo.lcf, i)
	if cosmetic ~= nil then
		local tabl = cosmetic:ToTable()
		
		local bank_index = MapCarIndexToBank(i)
		local bank_key = "CarBankD2_"..bank_index
		local bank_path = "voices2/Bank_"..bank_index.."/"
		
		-- try loading sound bank
		local bank = SoundBanks[bank_key]
		
		if bank == nil then
			bank = LoadSoundbank(bank_key, {
				revSample 	= bank_path.."0.wav",
				idleSample 	= bank_path.."1.wav",
				hornSample 	= bank_path.."2.wav",
			})
		end
		
		tabl.revSample = bank.revSample
		tabl.idleSample = bank.idleSample
		tabl.hornSample = bank.hornSample
		
		FixCarCos(tabl)
		
		table.insert(allCosmetics, tabl)
	end
end

return allCosmetics
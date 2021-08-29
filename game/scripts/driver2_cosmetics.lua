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

-- load all 13 cosmetics into lua
local allCosmetics = {}
for i=0,12 do
	local cosmetic = cars:LoadCosmeticsFile(CurrentCityInfo.lcf, i)
	if cosmetic ~= nil then
		local tabl = cosmetic:ToTable()
		
		local bank_index = MapCarIndexToBank(i)
		local bank_path = "voices2/Bank_"..bank_index.."/"
		
		local bank = LoadSoundbank("CarBankD2_"..bank_index, {
			revSample 	= bank_path.."0.wav",
			idleSample 	= bank_path.."1.wav",
			hornSample 	= bank_path.."2.wav",
		})
		
		tabl.revSample = bank.revSample
		tabl.idleSample = bank.idleSample
		tabl.hornSample = bank.hornSample
		
		table.insert(allCosmetics, tabl)
	end
end

return allCosmetics
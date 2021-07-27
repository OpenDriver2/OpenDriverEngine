local carsCommon = dofile("scripts/cars_common.lua")

local cars = engine.Cars						-- cars, handling

-- load all 13 cosmetics into lua
local allCosmetics = {}
for i=0,12 do
	local cosmetic = cars:LoadCosmeticsFile(CurrentCityInfo.lcf, i)
	if cosmetic ~= nil then
		local tabl = cosmetic:ToTable()
		table.insert(allCosmetics, tabl)
	end
end

return allCosmetics
local cars = engine.Cars

-- Driver 1 cars
local GearDesc_D1 = {
	GEAR_DESC( { lowidl_ws = 0, 	low_ws = 0, 	hi_ws = 245, 	ratio_ac = 96, ratio_id = 90 }),
	GEAR_DESC( { lowidl_ws = 130, 	low_ws = 200, 	hi_ws = 390, 	ratio_ac = 60, ratio_id = 57 }),
	GEAR_DESC( { lowidl_ws = 130, 	low_ws = 350, 	hi_ws = 540, 	ratio_ac = 40, ratio_id = 38 }),
	GEAR_DESC( { lowidl_ws = 130, 	low_ws = 490, 	hi_ws = 9999, 	ratio_ac = 32, ratio_id = 30})
}

-- Driver 2 player car
local GearDesc_D2 = {
	GEAR_DESC( { lowidl_ws = 0, 	low_ws = 0, 	hi_ws = 163, 	ratio_ac = 144, ratio_id = 135 }),
	GEAR_DESC( { lowidl_ws = 86, 	low_ws = 133, 	hi_ws = 260, 	ratio_ac = 90, 	ratio_id = 85 }),
	GEAR_DESC( { lowidl_ws = 186, 	low_ws = 233, 	hi_ws = 360, 	ratio_ac = 60, 	ratio_id = 57 }),
	GEAR_DESC( { lowidl_ws = 286, 	low_ws = 326, 	hi_ws = 9999, 	ratio_ac = 48, 	ratio_id = 45 })
}

-- Driver 2 civ car
local GearDesc_D2_civ = {
	GEAR_DESC( { lowidl_ws = 0, 	low_ws = 0, 	hi_ws = 50, 	ratio_ac = 144, ratio_id = 135 }),
	GEAR_DESC( { lowidl_ws = 43, 	low_ws = 66, 	hi_ws = 100, 	ratio_ac = 90, 	ratio_id = 85 }),
	GEAR_DESC( { lowidl_ws = 93, 	low_ws = 116, 	hi_ws = 150, 	ratio_ac = 60, 	ratio_id = 57 }),
	GEAR_DESC( { lowidl_ws = 143, 	low_ws = 163, 	hi_ws = 9999, 	ratio_ac = 48, 	ratio_id = 45 })
}

local defaultHandling = HANDLING_TYPE {
	frictionScaleRatio = 32, 
	aggressiveBraking = true, 
	fourWheelDrive = false, 
	autoBrakeOn = 1
}

-- setup car sound handler
-- FIXME this is bad a bit
cars.soundSourceGetCallback = function(soundType)

	local sbk_perm = SoundBanks["permanent"]
	
	if soundType == "HitCurb" then
		return sbk_perm.Hit_Cone
	elseif soundType == "Hit_Car_3" then
		local sounds = {
			sbk_perm.Hit_Car_3a,
			sbk_perm.Hit_Car_3b
		}
	
		return sounds[math.random(1,2)]
	end
	
	return sbk_perm[soundType]
end

-- attach to event sink TODO: event manager with subscribers etc
cars.eventCallback = function(caller, eventType, parameters)
	Msg("Got event '", eventType, "'")
end

return {
	HandlingType = {
		defaultHandling,
	},
	GearDesc = {
		GearDesc_D1,
		GearDesc_D2,
		GearDesc_D2_civ
	},
	Gravity = {
		-10922,		-- Driver 1
		-7456,		-- Driver 2
	}
}
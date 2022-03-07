local world = engine.World						-- collision and rendering world

-- port of D2 smashables to lua
local KnownSmashables = {
    -- name, sound, volume, pitch
	{ "", 1, 0, 800 },		-- default
	{ "CONE_TASTIC", 1, 0, 1400 },
	{ "CONE1_TASTIC", 1, 0, 1400 },
	{ "CONE2_TASTIC", 1, 0, 1400 },
	{ "BOX_TASTIC", 0, -6000, 4096 },
	{ "BOX2_TASTIC", 0, -6000, 4096 },
	{ "BOX3_TASTIC", 0, -6000, 4096 },
	{ "BIN_TASTIC", 3, -6500, 3200 },
	{ "BARRIER_TASTIC", 3, -6500, 3200 },
	{ "BARRIER1_TASTIC", 3, -6500, 3200 },
	{ "BARREL_TASTIC", 2, -3300, 2400 },
	{ "BARREL1_TASTIC", 2, -3300, 2400 },
	{ "BARREL2_TASTIC", 2, -3300, 2400 },
	{ "TABLE_TASTIC", 3, -6500, 3200 },
	{ "BENCH_TASTIC", 3, -6500, 3200 },
	{ "CHAIR_TASTIC", 3, -6500, 3200 },
	{ "CHAIR1_TASTIC", 3, -6500, 3200 },
	{ "CHAIR2_TASTIC", 3, -6500, 3200 },
	{ "UMBRELLA_TASTIC", 3, -6500, 3200 },
	{ "UMBRELLA1_TASTIC", 3, -6500, 3200 },
	{ "PHONE_TASTIC", 2, -3300, 3600 },
	{ "PHONE1_TASTIC", 2, -3300, 3600 },
	{ "SIGN00_TASTIC", 3, -6500, 3200 },
	{ "NEWS_TASTIC", 3, -6500, 3200 },
	{ "PARKING_TASTIC", 2, -7500, 4096 },
	{ "DRINKS_TASTIC", 2, -3300, 3600 },
	{ "FENCE_TASTIC", 3, -6500, 3200 },
	{ "FENCE00_TASTIC", 3, -6500, 3200 },
	{ "FENCE01_TASTIC", 3, -6500, 3200 },
	{ "FENCE02_TASTIC", 3, -6500, 3200 },
	{ "FENCE03_TASTIC", 3, -6500, 3200 },
	{ "FENCE04_TASTIC", 3, -6500, 3200 },
	{ "CONE_TASTIC1", 1, 0, 800 },
	{ "CONE_TASTIC2", 1, 0, 800 },
	{ "CONE_TASTIC3", 1, 0, 800 },
	{ "CONE_TASTIC4", 1, 0, 800 },
	{ 0, 0, 0, 0 }
}

local SmashableSounds = {
    [0] = "Hit_Box",
	[1] = "Hit_Cone",
	[2] = "Hit_Barrel",
	[3] = "Hit_Fence",
}

local Smashables = {}

---------------------------------------------------------

local function InitSmashables()
    Smashables = {}
    for i,v in ipairs(KnownSmashables) do
        local modelRef = world.GetModelByName(v[1])
        if modelRef ~= nil then
            Smashables[modelRef.index] = v
        end
    end
end
AddCallback(InitSmashables, CityEvents.OnLoaded)

--
-- OnHitSmashable : callback for smashable hit
-- 		@eventData: 	hit event data
--                .model :          modelRef
--                .cellObject :     CELL_OBJECT - hit object
--                .position :       ref(VECTOR) - collision position
--                .normal :         ref(VECTOR) - collision normal
--                .velocity :       ref(VECTOR) - direction vector of hit
--                .strikeVel :      ref(int) - the hit velocity
--

CarEventCallbacks["HitSmashable"] = function(car, eventData)
    local sobj = Smashables[eventData.cellObject.type]
    if sobj == nil then
        sobj = KnownSmashables[1]
    end

    --MsgInfo("Smashable ", sobj[1])

    local position = eventData.position.value
    local velocity = eventData.velocity.value
    
    local sbk_perm = SoundBanks["permanent"]
    local soundName = SmashableSounds[sobj[2]]
    local volume = sobj[3]
    local pitch = sobj[4] + (((velocity.vx ~ velocity.vz) * (position.vx ~ position.vz) & 1023) - 512)

    StartStaticSound3D(sbk_perm[soundName], fix.FromFixedVector(position), VolumeDbToPerc(volume), pitch / fix.ONE)
end

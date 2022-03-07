local world = engine.World						-- collision and rendering world

-- most of D1 and all D2 smashables are here
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

--------------------------------------------------------------------

-- TODO: register them in city Lua file!!!
local KnownCyclingPals = {
    -- name, vx, vy, start1, stop1, speed1, start2, stop2, speed2
    Chicago = {
        { "REDRVR", 0, 0, 6, 10, 2, 11, 15, 2 },
        { "NAVPIR34", 0, 0, 0, 6, 0, 7, 13, 0 },
    },
    LasVegas = {
        { "DTSYN01", 0, 0, 0, 3, 0, 4, 14, 0 },
        { "DTSYN02", 0, 0, 0, 15, 0, -1, -1, 0 },
        { "F-MTSYN2", 0, 0, 0, 3, 0, 10, 14, 0 },
        { "DTSYN03", 0, 0, 0, 3, 0, -1, -1, 0 },
        { "SYN-CAS1", 0, 0, 0, 3, 0, 4, 14, 0 },
        { "SYNSLOT", 0, 0, 0, 3, 0, 4, 7, 0 },
        { "ENT1B", 0, 0, 0, 15, 0, -1, -1, 0 },
        { "FLAMINGO", 0, 0, 0, 3, 0, 9, 12, 0 },
        { "CYCLE-01", 0, 0, 0, 5, 1, 6, 11, 1 },
        { "CYCLE-02", 0, 0, 0, 3, 3, -1, -1, 0 },
        { "CYCLE-03", 0, 0, 0, 6, 7, 7, 13, 7 },
        { "CYCLE-04", 0, 0, 0, 6, 15, 7, 13, 15 }
    },
}
local CyclingPals = {}
local CycleTimer = 0

local function InitCyclingPals()
    CyclingPals = {}
    if IsNightCity(CurrentCityType) == false then
        return
    end
    local cyclingPals = KnownCyclingPals[CurrentCityName]
    if cyclingPals == nil then
        return
    end
    for i,v in ipairs(cyclingPals) do
        local texDetail = world.FindTextureDetail(v[1])
        if texDetail ~= nil then
            CyclingPals[texDetail] = v
        end
    end
end
AddCallback(InitCyclingPals, CityEvents.OnLoaded)

local function ColourCycle()
    local step = world.StepCount()
    if (step & 1) == 0 then
        return
    end
    for k,v in pairs(CyclingPals) do
        if (CycleTimer & v[6]) == 0 then
            world.StepTextureDetailPalette(k, v[4], v[5])
        end
        if (CycleTimer & v[9]) == 0 then
            world.StepTextureDetailPalette(k, v[7], v[8])
        end
    end
    CycleTimer = CycleTimer + 1
end
AddCallback(ColourCycle, CityEvents.OnStep)
-------------------------------------------------------------------

KnownAnimatedObjects = {
    -- internal_id, model_num, name, LitPoly
    Chicago = {
        
    }    
}

-------------------------------------------------------------------

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

local SmashedObjects = {}

CarEventCallbacks["HitSmashable"] = function(car, eventData)

    local cellObjType = eventData.cellObject.type
    local cellObjPos = fix.FromFixedVector(eventData.cellObject.pos)

    local doSound = ((eventData.model.shapeFlags & ShapeFlags.Trans) == 0)
    if doSound then
        local sobj = Smashables[cellObjType]
        if sobj == nil then
            sobj = KnownSmashables[1]
        end
    
        local position = eventData.position.value
        local velocity = eventData.velocity.value
        
        local sbk_perm = SoundBanks["permanent"]
        local soundName = SmashableSounds[sobj[2]]
        local volume = sobj[3]
        local pitch = sobj[4] + (((velocity.vx ~ velocity.vz) * (position.vx ~ position.vz) & 1023) - 512)
    
        StartStaticSound3D(sbk_perm[soundName], fix.FromFixedVector(position), VolumeDbToPerc(volume), pitch / fix.ONE)
    end

    local yang = eventData.cellObject.yang
    local cellRotationRad = -yang / 64.0 * math.pi * 2.0;
    cellObjPos.y = -cellObjPos.y

    -- compute smashable velocity
    local velocity = fix.FromFixedVector(eventData.velocity.value)

    velocity.y = math.abs(velocity.x) + math.abs(velocity.z)

    local rot_speed = 0
	if (math.random(0,3) & 1) == 0 then
		rot_speed = -velocity.y * 65.0
	else
		rot_speed = velocity.y * 65.0
    end

    if velocity.y > 0.045 then
        velocity.y = 0.045
    end

    velocity.y = velocity.y * 11

    -- add smashed object
    table.insert(SmashedObjects, {
        position = cellObjPos,
        angles = vec.vec3(0.0,cellRotationRad,0.0),
        model = cellObjType,

        origPosY = cellObjPos.y,
        velocity = velocity,
        rot_speed = rot_speed,
    })
end

function MoveSmashables(dt)
    local newSmashedObjects = {}
    for i,dam in ipairs(SmashedObjects) do
        -- move
        if dam.position.y > dam.origPosY-0.015 then
            dam.position = dam.position + dam.velocity * vec.vec3(dt)
            dam.velocity.y = dam.velocity.y - 3.0 * dt;
            dam.angles = dam.angles + vec.vec3(dam.rot_speed * dt) * vec.vec3(0.35, 3.0, 0.35)
            table.insert(newSmashedObjects, dam)
        end

        -- add to renderer
        world.AddDrawable(DRAWABLE(dam))
    end
    SmashedObjects = newSmashedObjects
end

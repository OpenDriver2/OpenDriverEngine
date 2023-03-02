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
    -- start1, stop1, speed1, start2, stop2, speed2
    Chicago = {
        ["REDRVR"]      = { start1 = 6, stop1 = 10, speed1 = 2,  start2 = 11, stop2 = 15, speed2 = 2 },
        ["NAVPIR34"]    = { start1 = 0, stop1 = 6,  speed1 = 0,  start2 = 7,  stop2 = 13, speed2 = 0 },
    },
    LasVegas = {
        ["DTSYN01"]     = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = 4,  stop2 = 14, speed2 = 0 },
        ["DTSYN02"]     = { start1 = 0, stop1 = 15, speed1 = 0,  start2 = -1, stop2 = -1, speed2 = 0 },
        ["F-MTSYN2"]    = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = 10, stop2 = 14, speed2 = 0 },
        ["DTSYN03"]     = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = -1, stop2 = -1, speed2 = 0 },
        ["SYN-CAS1"]    = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = 4,  stop2 = 14, speed2 = 0 },
        ["SYNSLOT"]     = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = 4,  stop2 = 7,  speed2 = 0 },
        ["ENT1B"]       = { start1 = 0, stop1 = 15, speed1 = 0,  start2 = -1, stop2 = -1, speed2 = 0 },
        ["FLAMINGO"]    = { start1 = 0, stop1 = 3,  speed1 = 0,  start2 = 9,  stop2 = 12, speed2 = 0 },
        ["CYCLE-01"]    = { start1 = 0, stop1 = 5,  speed1 = 1,  start2 = 6,  stop2 = 11, speed2 = 1 },
        ["CYCLE-02"]    = { start1 = 0, stop1 = 3,  speed1 = 3,  start2 = -1, stop2 = -1, speed2 = 0 },
        ["CYCLE-03"]    = { start1 = 0, stop1 = 6,  speed1 = 7,  start2 = 7,  stop2 = 13, speed2 = 7 },
        ["CYCLE-04"]    = { start1 = 0, stop1 = 6,  speed1 = 15, start2 = 7,  stop2 = 13, speed2 = 15 }
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
    CyclingPals = {}
    for name, v in pairs(cyclingPals) do
        local texDetail = world.FindTextureDetail(name)
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
    for detail, v in pairs(CyclingPals) do
        if (CycleTimer & v.speed1) == 0 then
            world.StepTextureDetailPalette(detail, v.start1, v.stop1)
        end
        if (CycleTimer & v.speed2) == 0 then
            world.StepTextureDetailPalette(detail, v.start2, v.stop2)
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

    velocity.y = velocity.y * 14

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
            dam.angles = dam.angles + vec.vec3(dam.rot_speed * dt) * vec.vec3(0, 3.14, 1)
            table.insert(newSmashedObjects, dam)
        end
    end
    SmashedObjects = newSmashedObjects
end

function DrawSmashables()
    for i,dam in ipairs(SmashedObjects) do
        world.AddDrawable(DRAWABLE(dam))
    end
end
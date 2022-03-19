local world = engine.World						-- collision and rendering world

local function MakeDreaAndStevesSecretLair()
    -- make secret lair visible
    for i=2,15 do
        world.CreateCellList(i)
    end
end

local function UpdateEvents()
end

local function Terminate()
end

local function TriggerEvent(name)
end

return {
    Initialize = MakeDreaAndStevesSecretLair,
    Terminate = Terminate,

    Trigger = TriggerEvent,
    Step = UpdateEvents
}
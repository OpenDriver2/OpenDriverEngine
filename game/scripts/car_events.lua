local cars = engine.Cars

CarEventCallbacks = {}

-- attach to event sink TODO: event manager with subscribers etc
cars.eventCallback = function(caller, eventType, parameters)
	local carEventCallbacks = CarEventCallbacks[eventType]
	if carEventCallbacks ~= nil then
		carEventCallbacks(caller, parameters)
	end
	--Msg("Got event '", eventType, "' from ", tostring(caller))
	--util.printObject(parameters)
end
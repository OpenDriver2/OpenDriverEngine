local updateFuncs = {}

local function errorHandler ( errobj )
	print("ERROR - "..errobj)
	print(debug.traceback())
	return false
end

function DoUpdateFuncs(dt)
	for k,v in pairs(updateFuncs) do
		v.time = v.time - dt
		if v.time <= 0 then
			xpcall(function() v.fn(v.freq + dt) end, errorHandler)			
			v.time = v.freq
		end
	end
end

-- this will set or remove the update function
function SetUpdateFunc(name, fn, freq, delay)
	if fn == nil then
		updateFuncs[name] = nil
	else
		updateFuncs[name] = { fn = fn, freq = freq or 0, time = delay or 0 }
	end
end
local updateFuncs = {}

function DoUpdateFuncs(dt)
	for k,v in pairs(updateFuncs) do
		v.time = v.time - dt
		if v.time <= 0 then
			local ran, errorMsg = pcall( v.fn, v.freq + dt )
			if not ran then
				MsgError("Lua error: " .. errorMsg)
			end
			
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
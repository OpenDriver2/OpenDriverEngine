-- cond ? a : b
function if_then_else(cond, a, b)

	local condResult = false
	
	if type(cond) == "function" then
		condResult = cond()
	else
		condResult = cond
	end

	if condResult then
		if type(a) == "function" then
			return a()
		end
	
		return a
	else
		if type(b) == "function" then
			return b()
		end
	
		return b
	end
end

iif = if_then_else
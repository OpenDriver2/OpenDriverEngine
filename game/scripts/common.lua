-------------------------------------------------------------
--	script utility functions
-------------------------------------------------------------

util = {}

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

-- splits the string
function string:split( inSplitPattern, outResults )
	if not outResults then
		outResults = { }
	end
	local theStart = 1
	local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	while theSplitStart do
		table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
		theStart = theSplitEnd + 1
		theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	end
	table.insert( outResults, string.sub( self, theStart ) )
	return outResults
end

-- prints object to the console output
function util.printObject( tt )

	local splr = ""
	if type(tt) == "userdata" then
		local meta = getmetatable(tt)
		splr = table.inspect(meta)
	else
		splr = table.inspect(tt)
	end

	local strings = splr:split('\n')
	
	for k,v in ipairs(strings) do
		print(v)
	end
end

-- Print anything - including nested tables
-- returns string
function table.inspect(tt, indent, done, inspector)
	done = done or {}
	
	if inspector == nil then
		inspector = {}
		inspector.buffer = ""
		inspector.Concat = function( self, v )
			self.buffer = self.buffer..v
		end
	end

	indent = indent or 0
	if type(tt) == "table" then
		for key, value in pairs (tt) do
			inspector:Concat(string.rep (" ", indent)) -- indent it
			if type(value) == "table" and not done [value] then
				done [value] = true
				inspector:Concat(string.format("[%s] => table\n", tostring(key)))
				inspector:Concat(string.rep(" ", indent+4)) -- indent it
				inspector:Concat("(\n");
				table.inspect(value, indent + 7, done, inspector)
				inspector:Concat(string.rep(" ", indent+4)) -- indent it
				inspector:Concat(")\n");
			else
				inspector:Concat(string.format("[%s] => (%s) %s\n",tostring (key), type(value), tostring(value)))
			end
		end
	elseif type(tt) == "function" then
		inspector:Concat(string.format("[%s] => (%s) %s\n",tostring (tt), type(tt), tostring(tt)))
	else
		inspector:Concat(tostring(tt) .."\n")
	end

	return inspector.buffer
end
-------------------------------------------------------------
--	script utility functions
-------------------------------------------------------------

dofile "scripts/orderedPairs.lua"

local callbackCallableMeta = {
	__call = function(self, ...)
		local chain = self
		while chain ~= nil do
			chain.func(...)
			chain = chain.next
		end
	end
}

-- usage:
--		cb = AddCallback(func1)
--		cb()	-- call order: func1()
--		cb = AddCallback(func2, cb)
--		cb()	-- call order: func1() func2()
--		...
--		cb = AddCallback(funcN, cb)
--		cb()	-- call order: func1() func2() ... funcN()
function AddCallback(callback, addTo)
	local link = { func = callback, next = nil }
	if addTo == nil then
		setmetatable(link, callbackCallableMeta)
		return link
	end

	-- go to tail, add new link
	local chain = addTo
	while chain.next ~= nil do
		chain = chain.next
	end 
	chain.next = link

	-- add quick lookup of parent link
	addTo[callback] = chain
	return addTo
end

-- removes callback from link.
function RemoveCallback(callback, removeFrom)
	if removeFrom == nil then
		return
	end

	-- if head is removed, re-create chain
	if removeFrom.func == callback then
		local newChain = AddCallback(removeFrom.next.func)

		local chain = removeFrom.next
		while chain ~= nil do
			AddCallback(chain.func, newChain)
			chain = chain.next
		end
		return newChain
	end

	-- get the parent chain link
	local parent = removeFrom[callback]
	-- unlink
	parent.next = parent.next.next

	return removeFrom
end

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
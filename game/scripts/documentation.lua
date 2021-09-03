function PrintDocumentation()
	for namespace in orderedPairs(docs) do
		local tbl = docs[namespace]
		MsgInfo("namespace ", namespace)
		
		for typeName in orderedPairs(tbl) do
			local typeTbl = tbl[typeName]
			if string.len(typeTbl.description) > 0 then
				MsgWarning("\t", typeName, "\t - ", typeTbl.description)
			else
				MsgWarning("\t", typeName)
			end
			
			for name in orderedPairs(typeTbl.members) do
				local desc = typeTbl.members[name]
				if string.len(desc) > 0 then
					Msg("\t\t", name, "\t\t - ", desc, "")
				else
					Msg("\t\t", name)
				end
			end
		end
	end
end
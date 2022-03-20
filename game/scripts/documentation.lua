-- PrintDocumentation - prints documentation to console
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
				local propTbl = typeTbl.members[name]
				if string.len(propTbl.desc) > 0 then
					Msg("\t\t", DocPropType[propTbl.type], " ", name, "\t\t - ", propTbl.desc, "")
				else
					Msg("\t\t", DocPropType[propTbl.type], " ", name)
				end
			end
		end
	end
end

documentationWindowShown = false
local DocPropType = {
	[0] = "function",
	[1] = "member",
	[2] = "enum"
}

function ShowDocumentationWindow()

	local open, draw = ImGui.Begin("Lua documentation", documentationWindowShown)
	documentationWindowShown = open
	
	if draw then
		ImGui.SetNextWindowSize(500, 400, ImGuiCond.FirstUseEver)

		for namespace in orderedPairs(docs) do
			local docTbl = docs[namespace]
			local tbl = {}
			if namespace ~= "_G" then
				tbl = _G[namespace]
			end
			if ImGui.CollapsingHeader(namespace) then
			
				for typeName in orderedPairs(docTbl) do
					local typedocTbl = docTbl[typeName]
					local typeStr = ""
					if typedocTbl.description ~= nil and string.len(typedocTbl.description) > 0 then
						typeStr = typeName .. " - " .. typedocTbl.description
					else
						typeStr = typeName
					end
						
					if ImGui.TreeNode(typeStr) then
						if ImGui.BeginTable("table1", 3) then

							ImGui.TableSetupColumn("Type", 0, 0.35)
							ImGui.TableSetupColumn("Name", 0, 0.7)
							ImGui.TableSetupColumn("Description", 0, 1.25)
							ImGui.TableHeadersRow()
						
							for name in orderedPairs(typedocTbl.members) do
								local propdocTbl = typedocTbl.members[name]
								ImGui.TableNextRow();

								ImGui.TableSetColumnIndex(0)
								ImGui.Text(DocPropType[propdocTbl.type])
								
								ImGui.TableSetColumnIndex(1)
								ImGui.Text(name)

								ImGui.TableSetColumnIndex(2)
								ImGui.Text(propdocTbl.desc)
							end
							ImGui.EndTable()
						end
					
						ImGui.TreePop()
						ImGui.Separator()
					end
				end
				-- constants table
				if ImGui.BeginTable("constTable", 3) then

					ImGui.TableSetupColumn("Type", 0, 0.35)
					ImGui.TableSetupColumn("Name", 0, 0.7)
					ImGui.TableHeadersRow()
				
					for name in orderedPairs(tbl) do
						local prop = tbl[name]
						
						if type(prop) ~= "table" then
							ImGui.TableNextRow();

							ImGui.TableSetColumnIndex(0)
							ImGui.Text(type(prop))
							
							ImGui.TableSetColumnIndex(1)
							ImGui.Text(name)
						end
					end
					ImGui.EndTable()
				end
			end
		end
		ImGui.End()
	end
end
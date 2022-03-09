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

ShowDocumentation = false
local DocPropType = {
	[0] = "func",
	[1] = "prop",
	[2] = "enum"
}

function DisplayDocumentationWindow()

	if ShowDocumentation == false then
		return
	end

	ImGui.SetNextWindowSize(500, 400, ImGuiCond.FirstUseEver)
	if ImGui.Begin("Lua documentation") then
		for namespace in orderedPairs(docs) do
			local tbl = docs[namespace]
			if ImGui.CollapsingHeader(namespace) then
			
				for typeName in orderedPairs(tbl) do
					local typeTbl = tbl[typeName]
					local typeStr = ""
					if typeTbl.description ~= nil and string.len(typeTbl.description) > 0 then
						typeStr = typeName .. " - " .. typeTbl.description
					else
						typeStr = typeName
					end
						
					if ImGui.TreeNode(typeStr) then
						if ImGui.BeginTable("table1", 3) then

							ImGui.TableSetupColumn("Type", 0, 0.35)
							ImGui.TableSetupColumn("Name", 0, 0.7)
							ImGui.TableSetupColumn("Description", 0, 1.25)
							ImGui.TableHeadersRow()
						
							for name in orderedPairs(typeTbl.members) do
								local propTbl = typeTbl.members[name]
								ImGui.TableNextRow();

								ImGui.TableSetColumnIndex(0)
								ImGui.Text(DocPropType[propTbl.type])
								
								ImGui.TableSetColumnIndex(1)
								ImGui.Text(name)

								ImGui.TableSetColumnIndex(2)
								ImGui.Text(propTbl.desc)
							end
							ImGui.EndTable()
						end
					
						ImGui.TreePop()
						ImGui.Separator()
					end
				end
			end
		end
		ImGui.End()
	end
end
require('vstudio')

premake.api.register {
	name = "unitybuild",
	scope = "config",
	kind = "boolean",
	default_value = "off",
}

premake.api.register {
	name = "dontincludeinunity",
	scope = "config",
	kind = "boolean",
	default_value = "off",
}

local function enable_unitybuild(cfg) 
	if cfg.unitybuild then
		--print("Unity build enabled!")
		premake.w('<PropertyGroup><EnableUnitySupport>true</EnableUnitySupport></PropertyGroup>')
	end
end 

local function include_in_unitybuild(cfg) 
	if cfg.unitybuild then
		--print("Included in unity build!")
		premake.w('<IncludeInUnityFile>true</IncludeInUnityFile>')
	end
end 

local function dont_include_in_unitybuild(cfg) 
	if cfg.dontincludeinunity then
		--print("Not included in unity build!")
		--print(cfg.name)
		premake.w('<IncludeInUnityFile>false</IncludeInUnityFile>') --todo use premake.element 
		premake.w('<PrecompiledHeader>NotUsing</PrecompiledHeader>')
	end
end 

premake.override(premake.vstudio.vc2010.elements, "project", function(base, cfg)
	local calls = base(cfg)
	table.insertafter(calls, premake.vstudio.vc2010.project, enable_unitybuild)
	return calls
end)

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
	local calls = base(cfg)
	table.insertafter(calls, premake.vstudio.vc2010.precompilerHeaders, include_in_unitybuild)
	return calls
end) 

premake.override(premake.vstudio.vc2010, "excludedFromBuild", function(base, cfg)
	local calls = base(cfg)
	dont_include_in_unitybuild(cfg)
	
	--table.insertafter(calls, premake.vstudio.vc2010.excludedFromBuild, dont_include_in_unitybuild)
	
	return calls
end) 
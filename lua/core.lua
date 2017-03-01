
local libs = { }

_G.require = function(moduleName)

	if not libs[moduleName] then
		libs[moduleName] = C_ExecuteFile(moduleName .. ".lua")
	end

	return libs[moduleName]

end

local _formatscalar = function(v)
	if type(v) == "string" then
		return v .. ":stirng"
	elseif type(v) == "number" then
		return "" .. v .. ":number"
	elseif type(v) == "boolean" then
		if v then
			return "true:boolean"
		end
		return "false:boolean"
	end
	return v
end

_G.dump = function(v, level)

	if level == nil then level = 0 end
	local align = ""
	for i = 1, level do
		align = align .. "  "
	end


	if type(v) == "table" then
		for key, val in pairs(v) do
			if type(val) == "table" then
				lprint(align .. key .. " = table")
				dump(val, level + 1)
			else
				lprint(align .. key .. " = " .. _formatscalar(val))
			end
		end
	else
		lprint(align .. _formatscalar(v))
	end
end

_G.dofile = nil
_G.loadfile = nil
_G.load = nil
_G.loadstring = nil
_G.print = nil

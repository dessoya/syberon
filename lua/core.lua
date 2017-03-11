
local libs = { }

_G.bn = { [true] = "true", [false] = "false" }

_G.require = function(moduleName)

	if not libs[moduleName] then
		local status, r1 = pcall(C_ExecuteFile, moduleName .. ".lua")
		if status then
			libs[moduleName] = r1
		else
			libs[moduleName] = nil
			eprint(r1)
		end
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


local function split(str, sep)
   local result = {}
   local regex = ("([^%s]+)"):format(sep)
   for each in str:gmatch(regex) do
      table.insert(result, each)
   end
   return result
end

local debugOption = require("debugOption")

C_InstallPreProcessor(function(filename, text)

	local f = filename:sub(1, filename:len() - 4)
	if debugOption[f] == nil then return text end

	local opt = debugOption[f]
	if type(opt) == "boolean" then
		if opt == false then return text end
		opt = { }
	else
		if opt.all ~= nil and opt.all == false then return text end
	end

	local lines = split(text, "\n\r")

	text = ""
	for i, line in ipairs(lines) do

		for l in line:gmatch("%s-[%-]+%s-(lprint[^\n\r]+)") do
			line = l
			break
		end

		for l in line:gmatch("%s-[%-]+%s-(dump[^\n\r]+)") do
			line = l
			break
		end

		for g, l in line:gmatch("%s-[%-]+%s-%[(%S+)%]%s-(lprint[^\n\r]+)") do
			if opt[g] ~= nil and opt[g] == true then
				line = l
			end
			break
		end

		text = text .. "\n" .. line
	end

	return text
end)
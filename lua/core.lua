C_InstallModule("file")

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
	elseif type(v) == "function" then
		return "function"
	else
		return "unk:" .. type(v)
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
	local start, last = 1, str:len() + 1

	while start < last do
		local s, e = str:find(sep, start)
		if s ~= nil then
			table.insert(result, str:sub(start, s - 1))
			start = e + 1
		else
			break
		end
	end

	if start < last then
		table.insert(result, str:sub(start))
	end

   -- dump(result)

   return result
end



local debugOption = require("debugOption")

local consts, includes = { }, { }
--[[
usage:
#include file
#const name value
lprint("#name")
]]

function _parse_consts(lines)
	for i, line in ipairs(lines) do

		for name in line:gmatch("#include%s+([%a%d%-_\\]+)") do
			-- lprint("#include '" .. name .. "'")
			line = "  "
			lines[i] = line

			if includes[name] == nil then
				local inc = C_File_Read("lua\\" .. name .. ".lua")
				local inc_lines = split(inc, "\r\n")
				_parse_consts(inc_lines)
				includes[name] = true
			end

		end

		for name, value in line:gmatch("#const%s+([%a%d%-_]+)%s+([%S]+)") do
			-- lprint("found const '" .. name .. "' = '" .. value .."'")
			line = "  "
			lines[i] = line
			consts[name] = value
			break
		end

		while true do

			local count = 0
			for name in line:gmatch("#([%a%d%-_]+)") do
				if consts[name] == nil then
					-- lprint("const '" .. name .. "' not found")
				else
					count = count + 1
					-- lprint("found using const '" .. name .. "'")
					while true do
						local s,e = line:find("#" .. name)
						if s ~= nil then
							line = line:sub(1, s - 1) .. consts[name] .. line:sub(e + 1)
							lines[i] = line
						else
							break
						end
					end
				end						
				break
			end

			if count == 0 then break end
		end

	end	
end

C_InstallPreProcessor(function(filename, text)

	-- lprint(filename)
	local f = filename:sub(1, filename:len() - 4)
	local checkForDebugOutput = true
	if debugOption[f] == nil then
		checkForDebugOutput = false
	else

		local opt = debugOption[f]
		if type(opt) == "boolean" then
			if opt == false then
				checkForDebugOutput = false
			else
				opt = { }
			end
		else
			if opt.all ~= nil and opt.all == false then
				checkForDebugOutput = false
			end
		end

	end

	local lines = split(text, "\r\n")
	_parse_consts(lines)

	text = ""
	for i, line in ipairs(lines) do

		if checkForDebugOutput then
			-- debug printing
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
		end

		if text:len() == 0 then
			text = line
		else
			text = text .. "\n" .. line
		end
	end

	return text
end)
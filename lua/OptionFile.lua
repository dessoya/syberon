C_InstallModule("file")

local Object = require("Object")
local OptionFile = Object:extend()

local function split(str, sep)
   local result = {}
   local regex = ("([^%s]+)"):format(sep)
   for each in str:gmatch(regex) do
      table.insert(result, each)
   end
   return result
end

function OptionFile:initialize(filename)
	self.filename = filename
	self.groups = { }
	local text = C_File_Read(filename)

	
	local lines = split(text, "\n")
	local group
	for i, line in ipairs(lines) do
   		-- lprint("" .. i .. ": " .. line)
   		for p in line:gmatch("[[](%w+)[]]") do
      		-- lprint("group '" .. p .. "'")
      		self.groups[p] = { }
      		group = self.groups[p]
      		break
   		end

   		for k,v in line:gmatch("%s*(%w+)%s*[=]%s*(%w+)%s*") do
      		-- lprint("option '" .. k .. "' = '"..v.."'")
      		if v == "true" then
      			v = true
      		elseif v == "false" then
      			v = false
      		else
      			local n = tonumber(v)
      			if n ~= nil then
      				v = n
      			end
      		end
      		group[k] = v
   			break
   		end

	end

end

function OptionFile:getGroup(groupName)
	return self.groups[groupName]
end

function OptionFile:setGroup(groupName, group)
	self.groups[groupName] = group
end

function OptionFile:save()
	local text = ""
	for key, group in pairs(self.groups) do
		text = text .. "[" .. key .. "]\n"
		for option, value in pairs(group) do
			local v = ""
			if type(value) == "boolean" then
				if value then
					v = v .. "true"
				else
					v = v .. "false"
				end
			else
				v = v .. value
			end
			text = text .. option .. " = " .. v .. "\n"
		end
	end
	C_File_Write(self.filename, text)
end

return OptionFile 
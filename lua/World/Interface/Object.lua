local Object = require("Object")
local IObject = Object:extend()
local Const = require("World\\Const")

local worldThreadId = nil
function IObject.init(_worldThreadId)
	worldThreadId = _worldThreadId
end

function IObject.make(data)

	local type = data.type
	if IObject.map[type] == nil then return nil end
	local object = IObject.map[type]:new(data)
	return object

end

function IObject:initialize(data)	
	self.id = data.id
	self.type = data.type
	self.x = data.x
	self.y = data.y
end

function IObject:updateObject(prop)
	if worldThreadId == nil then 
		lprint("Error: IObject:updateProp worldThreadId == nil")
		return
	end
	prop.id = self.id
	C_Thread_PostMessage(worldThreadId, Const.CMD_UpdateObject, C_PackTable(prop))
end

IObject.map = { }

return IObject
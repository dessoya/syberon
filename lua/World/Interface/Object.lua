local Object = require("Object")
local IObject = Object:extend()
local Const = require("World\\Const")
local scale = -1

local Cursor = require("Windows\\Cursor")
local WindowsConst = require("Windows\\Const")

local worldQueue = nil
function IObject.init(_worldQueue)
	worldQueue = _worldQueue
end

function IObject.setScale(_scale)
	scale = _scale
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
	self.hover = false
end

function IObject:updateObject(prop)
	if worldQueue == nil then 
		lprint("Error: IObject:updateProp worldThreadId == nil")
		return
	end
	prop.id = self.id
	-- worldQueue
	C_CQueue_send(worldQueue, Const.CMD_UpdateObject, C_PackTable(prop))
end

function IObject:pointOnObject(x, y)

	local w, h = self:getSize(scale)
	local x1, y1 = self.x - w, self.y - h
	local x2, y2 = self.x + w, self.y + h

	if x >= x1 and x <= x2 and y >= y1 and y <= y2 then return true end

	return false

end

function IObject:onHover() end
function IObject:onHoverLost() end

IObject.map = { }

return IObject
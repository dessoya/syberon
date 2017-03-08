local Object = require("Object")

local WorldObject = Object:extend()

local idGenerator = 1
local f = C_Timer_QueryPerformanceFrequency()
local period = math.floor(f / 10000)

function WorldObject:initialize(x, y)
	self.x = x
	self.y = y
	self.id = idGenerator
	self.counter = C_Timer_QueryPerformanceCounter()
	idGenerator = idGenerator + 1
end

function WorldObject:getCounter()

	local c = C_Timer_QueryPerformanceCounter()
	local t = c - self.counter
	--[[
	local e = t * 1000000 / f
	self.counter = self.counter + t
	return e
	]]

	if t < period then return -1 end

	t = math.floor(t / period) * period
	local e = (0.0 + t) * 1000000.0 / (f + 0.0)

	self.counter = self.counter + t
	return e

end

function WorldObject:serialize()
	local data = { type = self.type, id = self.id, x = self.x, y = self.y }
	self:_serialize(data)
	return data
end


return WorldObject
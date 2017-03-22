C_InstallModule("cqueue")

local Object = require("Object")
local Child = Object:extend()

function Child:initialize(index, ownerQueue, poolQueue, childQueue, ...)
	self.index = index
	self.poolQueue = poolQueue
	self.ownerQueue = ownerQueue
	self.childQueue = childQueue
	if self.init ~= nil then
		self:init(...)
	else
		self.args = { ... }
	end
end

function Child:send(...)
	C_CQueue_send(self.ownerQueue, ...)
end

function Child:empty()
	return C_CQueue_empty(self.childQueue)
end

function Child:get(c)
	if c == nil then
		return C_CQueue_get(self.childQueue)
	end

	local t = { C_CQueue_get(self.childQueue) }
	
	local a1 = table.remove(t, 1)
	if c == 1 then return a1, t end

	local a2 = table.remove(t, 1)
	if c == 2 then return a1, a2, t end

	local a3 = table.remove(t, 1)
	if c == 3 then return a1, a2, a3, t end

	return nil
end

function Child:poolEmpty()
	return C_CQueue_empty(self.poolQueue)
end

function Child:poolGet(c)
	if c == nil then
		return C_CQueue_get(self.poolQueue)
	end

	local t = { C_CQueue_get(self.poolQueue) }
	
	local a1 = table.remove(t, 1)
	if c == 1 then return a1, t end

	local a2 = table.remove(t, 1)
	if c == 2 then return a1, a2, t end

	local a3 = table.remove(t, 1)
	if c == 3 then return a1, a2, a3, t end

	return nil
end


return Child
C_InstallModule("thread")
C_InstallModule("cqueue")

local Object = require("Object")
local Thread = Object:extend()

function Thread:initialize(filename, ownerQueue, size, ...)
	self.pool = { }
	self.poolQueue = C_CQueue_new()
	self.size = size
	for i = 1, size do
		local info = { }

		info.childQueue = C_CQueue_new()
		info.id = C_Thread_NewArgs("Thread\\PoolChildWrapper.lua", filename, i, ownerQueue, self.poolQueue, info.childQueue, ...)

		self.pool[i] = info
	end
end

function Thread:send(...)
	C_CQueue_send(self.poolQueue, ...)
end

function Thread:sendAll(...)
	for i = 1, self.size do
		local info = self.pool[i]
		C_CQueue_send(info.childQueue, ...)
	end
end

return Thread
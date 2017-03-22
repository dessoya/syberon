C_InstallModule("thread")
C_InstallModule("cqueue")

local Object = require("Object")
local Thread = Object:extend()

function Thread:initialize(filename, ownerQueue, ...)
	self.childQueue = C_CQueue_new()
	self.id = C_Thread_NewArgs("Thread\\ChildWrapper.lua", filename, ownerQueue, self.childQueue, ...)
end

function Thread:send(...)
	C_CQueue_send(self.childQueue, ...)
end

return Thread
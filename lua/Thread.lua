C_InstallModule("thread")

local Object = require("Object")
local Thread = Object:extend()

function Thread:initialize(filename, ...)
	self.id = C_Thread_New(filename, ...)
end

function Thread:send(message, lparam, wparam)
	while C_Thread_PostMessage(self.id, message, lparam, wparam) do
		C_Timer_Sleep(1)
	end
end

--[[
function thread(hwnd, name)

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_Thread_SetName(name)

	while true do

		while C_Thread_PeekMessage() do
			local message = C_Thread_GetMessageId()
			local lparam = C_Thread_GetLParam()
			local wparam = C_Thread_GetWParam()
			lprint("Thread messageId " .. message .. " " .. lparam .. " " .. wparam)
			C_Thread_PostMessage(hwnd, 0x401, 1, C_PackTable({ asd = 1, qwe = 2}))
		end

		C_Timer_Sleep(1)
	end

end
]]

return Thread
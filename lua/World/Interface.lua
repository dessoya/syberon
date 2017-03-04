local Const = require("World\\Const")
local MessagePump = require("MessagePump")

local Object = require("Object")

local Interface = Object:extend()

function Interface:initialize(hwnd)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit"
	})

end

function Interface:start()

	self.work = true
	while self.work do

		self:beforePeekMessage()

		while C_Thread_PeekMessage() do			
			local message = C_Thread_GetMessageId()
			self.pump:onWindowMessage(C_Thread_GetMessageId(), C_Thread_GetLParam(), 0, 0, C_Thread_GetWParam())	
		end

		C_Timer_Sleep(35)
	end
end

function Interface:beforePeekMessage()
end

function Interface:onQuit()
	self.work = false
end

return Interface

local Object = require("Object")
local Const = require("ImageLoader\\Const")
local ThreadChild = require("Thread\\Child")

local Unpacker = ThreadChild:extend()

function Unpacker:start()

	C_Thread_SetName("Unpacker")

	C_InstallModule("timer")
	C_InstallModule("pack")
	C_InstallModule("image")	

	local _unpackTime = 0

	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
			
				if messageId == Const.CMD_Unpack then


					local _start = C_Timer_QueryPerformanceCounter()

					local name, packData = a1, a2
					local imageData = C_Unpack(packData)
					local image = C_Image_NewFromData(imageData)

					local _end = C_Timer_QueryPerformanceCounter()
					local one = _end - _start

					_unpackTime = _unpackTime + one

					self:send(Const.CMD_ImageData, name, image)

				elseif messageId == Const.CMD_Quit then
					lprint("_unpackTime " .. _unpackTime)
					work = false
				end
			end
		end
		C_Thread_yield()

	end

end

return Unpacker
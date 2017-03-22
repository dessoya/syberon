local Object = require("Object")
local Const = require("ImageLoader\\Const")
local ThreadChild = require("Thread\\Child")

local Reader = ThreadChild:extend()

function Reader:start()

	C_Thread_SetName("Reader:" .. self.index)

	C_InstallModule("timer")
	C_InstallModule("image")	

	local readTime, count = 0, 0

	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
					
				if messageId == Const.CMD_Quit then
					-- lprint("readTime " .. count .. " " .. readTime)
					work = false
				end
			end
		end

		read = true
		while read do
			read = false
			if not self:poolEmpty() then
				read = true
				local messageId, a1, a2 = self:poolGet()
			
				if messageId == Const.CMD_ReadImage then


					local _start = C_Timer_QueryPerformanceCounter()

					local name = a1

					-- lprint("resource\\" .. name)
					local image = C_Image_New("resource\\" .. name)
					-- lprint("resource\\" .. name .. " done")

					local _end = C_Timer_QueryPerformanceCounter()
					local one = _end - _start

					readTime = readTime + one
					count = count + 1

					self:send(Const.CMD_ImageData, name, image)

				end
			end
		end

		C_Thread_yield()

	end

end

return Reader
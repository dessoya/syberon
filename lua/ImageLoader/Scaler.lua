local Object = require("Object")
local Const = require("ImageLoader\\Const")
local ThreadChild = require("Thread\\Child")

local Scaler = ThreadChild:extend()

function Scaler:start()

	C_Thread_SetName("Scaler:" .. self.index)

	C_InstallModule("timer")
	C_InstallModule("image")	

	local scaleTime, count = 0, 0

	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
					
				if messageId == Const.CMD_Quit then
					-- lprint("scaleTime " .. count .. " " .. scaleTime)
					work = false
				end
			end
		end

		read = true
		while read do
			read = false
			if not self:poolEmpty() then
				read = true
				local messageId, a1, a2, a3, a4, a5, a6 = self:poolGet()
			
				if messageId == Const.CMD_ScaleImage then

					local _start = C_Timer_QueryPerformanceCounter()

					local src, dst, m, d = a1, a2, a3, a4

					local image = C_Image_get(src)
					local scaleImage = C_Image_scale(image, m, d)

					local _end = C_Timer_QueryPerformanceCounter()
					local one = _end - _start

					scaleTime = scaleTime + one
					count = count + 1

					self:send(Const.CMD_ImageData, dst, scaleImage)

				elseif messageId == Const.CMD_SplitImage then

					local _start = C_Timer_QueryPerformanceCounter()

					local src, dst, w, h, x, y = a1, a2, a3, a4, a5, a6

					local image = C_Image_get(src)
					local splitImage = C_Image_scaleEx(image, 1, 1, x, y, w, h)

					local _end = C_Timer_QueryPerformanceCounter()
					local one = _end - _start

					scaleTime = scaleTime + one
					count = count + 1

					self:send(Const.CMD_ImageData, dst, splitImage)

				end
			end
		end

		C_Thread_yield()

	end

end

return Scaler
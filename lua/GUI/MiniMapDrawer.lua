#include World\Const
#include Map

local ThreadChild = require("Thread\\Child")
local Const = require("World\\Const")
local Map = require("Map")

local Drawer = ThreadChild:extend()

local maxW, maxH = 600, 600


function Drawer:start()

	local img = { self.args[1], self.args[2] }
	local index = 2
	local px, py = _tointeger(self.args[3] / 1024), _tointeger(self.args[4] / 1024)
	local m = Map:new(self.args[5])
	local rcList = self.args[6]


	C_Thread_SetName("Drawer")
	lprint("start")

	C_InstallModule("timer")
	C_InstallModule("image")	

	local f = C_Timer_QueryPerformanceFrequency()

	local readTime, count = 0, 0

	local rcImage = C_Image_get("png_02\\Kristal_01.png_1d10")

	local colors = {
		{_color(0,150,0),_color(150,230,50),_color(50,50,200),_color(70,70,70),_color(1,1,1)},
		{_color(0,75,0), _color(75,115,25),_color(25,25,100),_color(35,35,35),_color(1,1,1)}
	}
	-- dump(colors)

	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
					
				if messageId == Const.CMD_Quit then

					C_Image_Delete(img[1])
					C_Image_Delete(img[2])
					work = false

				elseif messageId == Const.CMD_Rescale then

					-- lprint("Const.CMD_Rescale")
					-- local _start = C_Timer_QueryPerformanceCounter()


					local scale = a1
					index = a2

					local w = maxW/2/scale
					local h = maxH/2/scale
					local w_ = maxW/2
					local h_ = maxH/2

					local x1, y1 = px - w, py - h
					local x2, y2 = px + w, py + h

					--[[
					lprint("scale " .. scale .. " index " .. index)
					lprint("player coords " .. px .. "x" .. py)
					lprint("minimap box " .. x1 .. "x" .. y1 .. " " .. x2 .. "x" .. y2)
					]]
					local image = img[index]

					local mp = C_Map_getPointer(m._ptr, px - w, py - h)

					local ix, iy = w_ + (px - w - px) * scale, h_ + (py - h - py) * scale

					for y = py - h, py + h do

						-- local mp = m:getPointer(px - w, y)
						C_MapPointer_moveTo(mp, px - w, y)

						ix = w_ + (px - w - px) * scale

						for x = px - w, px + w do

							-- local id,f = m:get(x, y)
							-- local id,f = mp:get(#MOVE_RIGHT)							
							-- mp:move(#MOVE_RIGHT)
							local id,f = C_MapPointer_get(mp, #MOVE_RIGHT)

							if id > 4 or id < 1 then id = 5 end

							-- lprint("id " .. id .. " f " .. (f + 1))
							-- if f > 1 then f = 1 end

							local color = colors[f + 1][id]							

							if scale >= 1 then
								C_Image_setPixel(image, ix, iy, color)
							end

							if scale >= 2 then
								C_Image_setPixel(image, ix+1, iy, color)
								C_Image_setPixel(image, ix, iy+1, color)
								C_Image_setPixel(image, ix+1, iy+1, color)
							end

							if scale >= 3 then
								C_Image_setPixel(image, ix+2, iy, color)
								C_Image_setPixel(image, ix+2, iy+1, color)
								C_Image_setPixel(image, ix, iy+2, color)
								C_Image_setPixel(image, ix+1, iy+2, color)
								C_Image_setPixel(image, ix+2, iy+2, color)
							end

							ix = ix + scale

						end
						iy = iy + scale
					end		

					-- local _end = C_Timer_QueryPerformanceCounter()
					-- local one = _end - _start

					-- lprint("draw time " .. _tointeger(one * 1000000.0 / f))


					local i, l = 0, rcList:len()
					while i < l do
						local rc = rcList:get(i)
						local rx, ry = rc:get(#ODP_X, 2) 
						rx, ry = rx / 1024, ry / 1024

						if rx >= x1 and rx <= x2 and ry >= y1 and ry <= y2 then

							-- 20x20 image
							C_Image_drawImage(image, rcImage, (rx - x1) * scale - 10, (ry - y1) * scale - 10)

						end

						i = i + 1
					end

					--_end = C_Timer_QueryPerformanceCounter()
					--one = _end - _start

					--lprint("draw time + rc " .. _tointeger(one * 1000000.0 / f))

					C_Image_updateDDS(image)

					self:send(Const.CMD_RescaleDone, scale, index)


				end
			end
		end

		C_Thread_yield()
		C_Timer_Sleep(15)
		C_Thread_yield()

	end

	lprint("done")

end

return Drawer
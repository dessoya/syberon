
local Const = require("ImageLoaderConst")

local Images = {
	  "controls.png"
	, "ter1.png"
	, "player-basic-idle.png"
	, "player-basic-run.png"
--	, "font_v1.png"
}
local TestImages = { }

if 0 == 1 then
	for i = 1, 200 do
		table.insert(TestImages, i)
	end
end

function thread(hwnd)

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_InstallModule("image")
	C_Thread_SetName("ImageLoader")

	local work = true
	while work do

		while C_Thread_PeekMessage() do			
			local message = C_Thread_GetMessageId()
			
			if message == Const.CMD_StartLoad then
				local imageCount = table.getn(Images) + table.getn(TestImages)
				C_Thread_PostMessage(hwnd, Const.CMD_ImageCount, imageCount)

				for k, v in pairs(Images) do
					local image = C_Image_New("resource\\" .. v)
					C_Thread_PostMessage(hwnd, Const.CMD_Image, C_PackTable({ name = v, image = image }))		
				end

				for k, v in pairs(TestImages) do

					C_Timer_Sleep(10)
					C_Thread_PostMessage(hwnd, Const.CMD_Image, C_PackTable({ name = v, image = v }))		
				end

			elseif message == Const.CMD_Quit then
				work = false
			end

		end

		C_Timer_Sleep(100)
	end

	lprint("done")
end

local Const = require("World\\Const")
local Object = require("Object")

local ImageGetter = Object:extend()

function ImageGetter:initialize(hwnd, imageForLoad, pump, cb)

	self.hwnd = hwnd
	pump:addNames({		
		[Const.CMD_Image]			= "onImage",
		[Const.CMD_ImageAbsent]		= "onImageAbsent"
	})
	pump:registerReciever(self)

	self.imageForLoad = imageForLoad
	self.images = { }

	self.cb = cb

	self:makeImageRequest()

end

function ImageGetter:makeImageRequest()

	if table.getn(self.imageForLoad) == 0 then
		self.cb(self.images)
		return
	end

	self.gettingImageName = table.remove(self.imageForLoad)
	-- lprint("GetImage " .. self.gettingImageName)
	C_Thread_PostMessage(self.hwnd, Const.CMD_GetImage, C_PackTable({ name = self.gettingImageName }))

end

function ImageGetter:onImage(lparam)
	--lprint("recieve image " .. self.gettingImageName)
	local data = C_UnpackTable(lparam)
	self.images[self.gettingImageName] = data.image
	self:makeImageRequest()
end

function ImageGetter:onImageAbsent()
	--lprint("image " .. self.gettingImageName .. " absent")
	C_Timer_Sleep(135)
	--lprint("try GetImage " .. self.gettingImageName)
	C_Thread_PostMessage(self.hwnd, Const.CMD_GetImage, C_PackTable({ name = self.gettingImageName }))
end

return ImageGetter
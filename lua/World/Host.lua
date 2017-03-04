local Object = require("Object")
local Thread = require("Thread")
local Const = require("World\\Const")
local WConst = require("Windows\\Const")

local World = Object:extend()

function World:initialize(pump, images, hwnd, renderer)

	self.images = images

	pump:addNames({
		[Const.CMD_GetImage]		= "onGetImage",
		[WConst.WM.Size]			= "onWindowSize"
	})

	pump:registerReciever(self)

	self.thread = Thread:new("World\\WorldThread", hwnd, renderer)
	self.interfaceThread = Thread:new("World\\InterfaceThread", hwnd)

end

function World:create()

	self.thread:send(Const.CMD_Create)

end

function World:onGetImage(lparam)
	local data = C_UnpackTable(lparam)
	lprint("World:onGetImage " .. data.name)
	if self.images[data.name] ~= nil then
		lprint("image exists")
		self.thread:send(Const.CMD_Image, C_PackTable({ image = self.images[data.name] }))
	else
		lprint("image absent")
		self.thread:send(Const.CMD_ImageAbsent)
	end
end

function World:onWindowSize(lparam, lparam1, lparam2, wparam)
	self.thread:send(WConst.WM.Size, lparam1, lparam2)
end

return World
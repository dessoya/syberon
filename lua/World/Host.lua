local Object = require("Object")
local Thread = require("Thread")
local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local Timer = require("Timer")

local World = Object:extend()

function World:initialize(pump, images, hwnd, renderer, optionFile)

	self.images = images

	pump:addNames({
		[Const.CMD_GetImage]		= "onGetImage",
		[WConst.WM.Size]			= "onWindowSize",

		[WConst.WM.SYSKeyDown]		= "onSysKeyDown",
		[WConst.WM.SYSKeyUp]		= "onSysKeyUp",
		[WConst.WM.KeyDown]			= "onKeyDown",
		[WConst.WM.KeyUp]			= "onKeyUp",
		[WConst.WM.Active]			= "onWindowActive"

	})

	pump:registerReciever(self)

	self.thread = Thread:new("World\\WorldThread", hwnd, renderer)
	self.interfaceThread = Thread:new("World\\InterfaceThread", hwnd)

	self.interfaceThread:send(Const.CMD_ThreadId, C_PackTable({ world = self.thread.id}))
	self.thread:send(Const.CMD_ThreadId, C_PackTable({ interface = self.interfaceThread.id}))

	Timer:new(15, function()
		self.thread:send(Const.CMD_Timer)
		self.interfaceThread:send(Const.CMD_Timer)
	end)

	self:sendKeys(optionFile:getGroup("keys"))

end

function World:sendKeys(keys)
	self.interfaceThread:send(Const.CMD_Keys, C_PackTable(keys))
end

function World:onSysKeyDown(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.SYSKeyDown, lparam, wparam)
end

function World:onSysKeyUp(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.SYSKeyUp, lparam, wparam)
end

function World:onKeyDown(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.KeyDown, lparam, wparam)
end

function World:onKeyUp(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.KeyUp, lparam, wparam)
end

function World:onWindowActive()
	self.interfaceThread:send(WConst.WM.Active, lparam, wparam)
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
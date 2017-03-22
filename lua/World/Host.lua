local Cursor = require("Windows\\Cursor")
local Object = require("Object")
local ThreadHost = require("Thread\\Host")
local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local Timer = require("Timer")

local World = Object:extend()

function World:initialize(ownerQueue, pump, images, hwnd, renderer, optionFile)

	self.images = images

	pump:addNames({
		[WConst.WM.Size]			= "onWindowSize",

		[WConst.WM.MouseMove]		= "onMouseMove",
		[WConst.WM.SYSKeyDown]		= "onSysKeyDown",
		[WConst.WM.SYSKeyUp]		= "onSysKeyUp",
		[WConst.WM.KeyDown]			= "onKeyDown",
		[WConst.WM.KeyUp]			= "onKeyUp",
		[WConst.WM.Active]			= "onWindowActive",
		[WConst.WM.MouseWheel]		= "onMouseWheel",

		[Const.CMD_UpdateOptions]	= "onUpdateOptions",	
		[Const.CMD_SetCursor]		= "onSetCursor"	
	})

	pump:registerReciever(self)

	self.thread = ThreadHost:new("World\\World", ownerQueue, hwnd, renderer)
	self.interfaceThread = ThreadHost:new("World\\Interface", ownerQueue, hwnd, renderer)

	self.interfaceThread:send(Const.CMD_Queue, Const.Q_World, self.thread.childQueue)	
	self.thread:send(Const.CMD_Queue, Const.Q_Interface, self.interfaceThread.childQueue)

	local g = optionFile:getGroup("interface")
	self.interfaceThread:send(Const.CMD_UpdateOptions, Const.Options.Interface, C_PackTable(g))
	self.thread:send(Const.CMD_UpdateOptions, Const.Options.Interface, C_PackTable(g))



	--[[
	Timer:new(5, function()
		self.thread:send(Const.CMD_Timer)
		self.interfaceThread:send(Const.CMD_Timer)
	end)
	]]

	self:sendKeys(optionFile:getGroup("keys"))

	--[[
	local a = Array:new(30)
	a:set(0, "qwe", "asd", 1, 2.2, 3, Array:new(), 111, 123, 333)
	a:push("world")
	a:dump()
	-- lprint("__index " .. a["aasd"])
	-- lprint("__tostring " .. tostring(a))
	self.thread:send(Const.CMD_Data, a)
	]]

end

function World:onMouseWheel(l, l1, l2, w, w1)

	-- lprint("World:onMouseWheel " .. w1)
	if self.thread ~= nil then
		self.thread:send(WConst.WM.MouseWheel, w1)
	end
	if self.interfaceThread ~= nil then
		self.interfaceThread:send(WConst.WM.MouseWheel, w1)
	end

end

function World:onSetCursor(id)
	C_Cursor_set(Cursor.get(id))
end

function World:onUpdateOptions(group, l1, l2, data)
	-- lprint("World:onUpdateOptions")
	local g = C_UnpackTable(data)
	if self.interfaceThread ~= nil then
		self.interfaceThread:send(Const.CMD_UpdateOptions, group, C_PackTable(g))
	end
	if self.thread ~= nil then
		self.thread:send(Const.CMD_UpdateOptions, group, C_PackTable(g))
	end
end


function World:onMouseMove(lparam, lparam1, lparam2, wparam)
	if self.interfaceThread ~= nil then
		self.interfaceThread:send(WConst.WM.MouseMove, lparam, lparam1, lparam2, wparam)
	end
end

function World:sendKeys(keys)
	self.interfaceThread:send(Const.CMD_Keys, C_PackTable(keys))
end

function World:onSysKeyDown(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.SYSKeyDown, lparam, l1, l2, wparam)
end

function World:onSysKeyUp(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.SYSKeyUp, lparam, l1, l2, wparam)
end

function World:onKeyDown(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.KeyDown, lparam, l1, l2, wparam)
end

function World:onKeyUp(lparam, l1, l2, wparam)
	self.interfaceThread:send(WConst.WM.KeyUp, lparam, l1, l2, wparam)
end

function World:onWindowActive()
	lprint("World:onWindowActive")
	self.interfaceThread:send(WConst.WM.Active, lparam, l1, l2, wparam)
end


function World:create(backgroundPtr)

	self.thread:send(Const.CMD_Create, C_PackTable({ backgroundPtr = backgroundPtr }))

end

function World:onGetImage(lparam)
	local data = C_UnpackTable(lparam)
	-- lprint("World:onGetImage " .. data.name)
	if self.images[data.name] ~= nil then
		-- lprint("image exists")
		self.thread:send(Const.CMD_Image, C_PackTable({ image = self.images[data.name] }))
	else
		-- lprint("image absent")
		self.thread:send(Const.CMD_ImageAbsent)
	end
end

function World:onWindowSize(lparam, lparam1, lparam2, wparam)
	-- lprint("World:onWindowSize")
	self.thread:send(WConst.WM.Size, lparam1, lparam2)
	self.interfaceThread:send(WConst.WM.Size, lparam, lparam1, lparam2)
end

return World
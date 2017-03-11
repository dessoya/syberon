C_InstallModule("renderer")

local GUIObject = require("GUI\\Object")
local WindowsConst = require("Windows\\Const")
local Cursor = require("Windows\\Cursor")

local Renderer = GUIObject:extend() 

function Renderer:initialize(hwnd, pump, _ptr)

	self.id = "renderer"
	self.hwnd = hwnd
	self.lockCount = 0

	if pump ~= nil then
		pump:addNames({
			[WindowsConst.WM.Size]			= "onWindowSize",
			[WindowsConst.WM.MouseMove]		= "onMouseMove",
			[WindowsConst.WM.LButtonDown]	= "onLButtonDown",
			[WindowsConst.WM.LButtonUp]		= "onLButtonUp",
			[WindowsConst.WM.Active]		= "onWindowActive"
		})
		pump:registerReciever(self)
	end

	if _ptr ~= nil then
		self._ptr = _ptr
	else
		self._ptr = C_Renderer_New(hwnd)
	end

end

function Renderer:refresh()
	C_Cursor_set(Cursor.get(WindowsConst.Cursor.Arrow))
	if self.ww ~= nil then
		self:onWindowSize(0, self.ww, self.wh)
	end
	if self.mx ~= nil then
		self:onMouseMove(0, self.mx, self.my, self.mflags)
	end
end

function Renderer:name() return "renderer" end

function Renderer:add(object, layer)
	self:addChild(object)
	object:pushToRenderer(self, layer)
	return object
end

function Renderer:getFPS()
	return C_Renderer_getFPS(self._ptr)
end

function Renderer:lockObjectList()
	if self.lockCount == 0 then
		--lprint("lock")
		C_Renderer_lockObjectList(self._ptr)
	end
	self.lockCount = self.lockCount + 1
end

function Renderer:unlockObjectList()
	if self.lockCount == 1 then
		--lprint("unlock")
		C_Renderer_unlockObjectList(self._ptr)
	end
	self.lockCount = self.lockCount - 1
end

function Renderer:del(object)
	self:delChild(object)
	object:delFromRenderer(self)
end

function Renderer:enableFullScreen(w, h)
	C_Renderer_enableFullScreen(self._ptr, w, h)
end

function Renderer:disableFullScreen()
	C_Renderer_disableFullScreen(self._ptr)
end

function testMode(w, h, rw, rh)
	if  math.floor(h / (w / rw)) == rh then return true end
	return false
end

function Renderer:getModeList()
	local t = C_Renderer_getModeList(self._ptr)
	local r = {	["4:3"] = { }, ["16:9"] = { }, ["16:10"] = { }, ["unk"] = { } }
	local l = table.getn(t)
	for i = 1, l, 2 do

		local w = t[i]
		local h = t[i + 1]
		local caption = "" .. w .. "x" .. h
		local item = { w = w, h = h, caption = caption }

		local sec = "unk"
		if testMode(w, h, 4, 3) then
			sec = "4:3"
		elseif testMode(w, h, 16, 9) then
			sec = "16:9"
		elseif testMode(w, h, 16, 10) then
			sec = "16:10"
		end			

		table.insert(r[sec], item)

	end
	return r
end

function Renderer:getCurrentWidth()
	return C_Renderer_getCurrentWidth(self._ptr)
end

function Renderer:getCurrentHeight()
	return C_Renderer_getCurrentHeight(self._ptr)
end

function Renderer:getCurrentViewWidth()
	return C_Renderer_getCurrentViewWidth(self._ptr)
end

function Renderer:getCurrentViewHeight()
	return C_Renderer_getCurrentViewHeight(self._ptr)
end

function Renderer:modify(f)

	self:lockObjectList()

	f()

	self:refresh()
	self:unlockObjectList()

end


-- messages


function Renderer:onWindowActive(lparam, lparam1, lparam2, wparam)
	--lprint("onWindowActive " .. wparam)
	if wparam == 0 then
		C_Renderer_setFPS(self._ptr, 10)
	else
		C_Renderer_setFPS(self._ptr, 80)
	end
end

function Renderer:onWindowSize(lparam, lparam1, lparam2, wparam)
	-- lprint("onWindowSize " .. lparam1 .. " " .. lparam2)
	self.ww = lparam1
	self.wh = lparam2
	self:invokeChilds("onWindowSize", lparam1, lparam2)
end


function Renderer:onMouseMove(lparam, lparam1, lparam2, wparam)
	self.mx = lparam1
	self.my = lparam2
	self.mflags = wparam
	--[mm] lprint("onMouseMove " .. lparam1 .. " " .. lparam2 .. " " .. wparam)
	self:invokeChilds("onMouseMove", lparam1, lparam2, wparam)
end

function Renderer:onLButtonDown()
	self:invokeChilds("onLButtonDown")
end

function Renderer:onLButtonUp()
	self:invokeChilds("onLButtonUp")
end


function Renderer:onWindowMessage(message, lparam, lparam1, lparam2, wparam)
	return C_Renderer_onWindowMessage(self._ptr, message, lparam, wparam)
end


return Renderer
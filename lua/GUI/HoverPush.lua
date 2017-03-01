
local Hover = require("GUI\\Hover")
local WindowsConst = require("Windows\\Const")
local Cursor = require("Windows\\Cursor")

local HoverPush = Hover:extend()

function HoverPush:initialize(x,y,w,h,cb)

	Hover.initialize(self,x,y,w,h)
	self.HoverPush_cb = cb
	self.pushState = false
end


function HoverPush:onHover()
	C_Cursor_set(Cursor.get(WindowsConst.Cursor.Hand))
end

function HoverPush:onHoverLost()
	C_Cursor_set(Cursor.get(WindowsConst.Cursor.Arrow))
	if self.pushState then
		self.pushState = false
		self:onPushStateChange()
	end
end

function hasbit(x, p)
  return x % (p + p) >= p       
end

function HoverPush:onMouseMove(x, y, flags, ctx)

	local s = Hover.onMouseMove(self, x, y, flags, ctx)

	if hasbit(flags, WindowsConst.Mouse.LeftButton) then
		self:onLButtonDown()
	end

	return s
end

function HoverPush:onLButtonDown()

	if self.hover and not self.underShadow then
		self.pushState = true
		self:onPushStateChange()
	end

	if self.hover and self.pushState and self.underShadow then
		self.pushState = false
		self:onPushStateChange()
	end

end

function HoverPush:onLButtonUp()
	if self.hover and not self.underShadow then
		self.pushState = false
		self:onPushStateChange()
		local cb = self.HoverPush_cb
		if type(cb) == "function" then
			cb(self)
		end
	end
end


return HoverPush   
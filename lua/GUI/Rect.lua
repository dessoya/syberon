local GUIObject = require("GUI\\Object")
local Rect = GUIObject:extend()

function Rect:initialize(x,y,w,h,r,g,b)
	self.x = x
	self.y = y
	self.w = w
	self.h = h
	self.r = r
	self.g = g
	self.b = b
	self._ptr = C_GUI_Rect_New(x,y,w,h,r,g,b)
	self.id = self:getID()
end

function Rect:setProp()
	C_GUI_Rect_setProp(self._ptr, self.x, self.y, self.w, self.h, self.r, self.g, self.b)
end


return Rect
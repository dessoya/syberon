local GUIObject = require("GUI\\Object")
local Box = GUIObject:extend()

function Box:initialize(x,y,w,h,r,g,b)
	self.x = x
	self.y = y
	self.w = w
	self.h = h
	self.r = r
	self.g = g
	self.b = b
	self._ptr = C_GUI_Box_New(x,y,w,h,r,g,b)
	self.id = self:getID()
end

function Box:setProp()
	C_GUI_Box_setProp(self._ptr, self.x, self.y, self.w, self.h, self.r, self.g, self.b)
end


return Box
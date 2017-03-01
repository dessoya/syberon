local GUIObject = require("GUI\\Object")
local ImageText = GUIObject:extend()

function ImageText:initialize(x,y,text,font)
	self.x = x
	self.y = y
	self.text = text
	self.font = font
	self.r = r
	self.g = g
	self.b = b
	self._ptr = C_GUI_ImageText_New(x,y,text,font)
	self.id = self:getID()
end

function ImageText:setProp()
	C_GUI_ImageText_setProp(self._ptr, self.x, self.y, self.text, self.font)
end


return ImageText
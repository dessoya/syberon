local GUIObject = require("GUI\\Object")
local Text = GUIObject:extend()

function Text:initialize(x,y,text,font,r,g,b)
	self.x = x
	self.y = y
	self.text = text
	self.font = font
	self.r = r
	self.g = g
	self.b = b
	self._ptr = C_GUI_Text_New(x,y,text,font,r,g,b)
	self.id = self:getID()
end

function Text:setProp()
	C_GUI_Text_setProp(self._ptr, self.x, self.y, self.text, self.font, self.r, self.g, self.b)
end


return Text
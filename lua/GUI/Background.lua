local Rect = require("GUI\\Rect")
local Background = Rect:extend()

function Background:initialize(x,y,w,h)
	Rect.initialize(self,0,0,1,1,0,0,0)
end

function Background:onWindowSize(w, h)
	self.w = w
	self.h = h
	self:setProp()
end

return Background
local Hover = require("GUI\\Hover")
local Rect = require("GUI\\Rect")

local Window = Hover:extend()

function Window:initialize(x,y,w,h)
	Hover.initialize(self,x,y,w,h)
	self.id = "window_" .. self:getIDFor("window")
	self:updateColor()
	self.rect = self:addChild(Rect:new(x,y,w,h,self.r,self.g,self.b))
end

function Window:updateColor()
	local l
	if self.hover then
		l = 120
	else
		l = 60
	end
		self.r = l
		self.g = l
		self.b = l
end

function Window:onHoverStatesChange()
	self:updateColor()
	self.rect.r = self.r
	self.rect.g = self.g
	self.rect.b = self.b
	self.rect:setProp()
	-- lprint("Window:onHoverStatesChange() " .. self.x)
end

function Window:onWindowSize(w, h)
	self.x = w / 2 - self.w / 2
	self.rect.x = self.x
	self.rect:setProp()
	self:invokeChilds("onParentChangePosition", self)

end

 return Window
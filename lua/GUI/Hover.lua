local GUIObject = require("GUI\\Object")
local Hover = GUIObject:extend()

function Hover:initialize(x,y,w,h)
	self.x = x
	self.y = y
	self.w = w
	self.h = h
	self.hover = false
	self.underShadow = false
end

function Hover:onMouseMove(x, y, flags, ctx)

	if ctx == nil then ctx = { hover = false} end
	-- lprint("Hover:onWMMouseMove " .. self.id)

	if self.childs then
		for index = self.childCount, 1, -1 do
			local child = self.childs[index]
			if child ~= nil then
				child:invokeMethod("onMouseMove",x, y, flags, ctx)
			end
		end
	end

	self:checkHover(x, y, ctx)
	return true

end

function Hover:checkHover(x, y, ctx) 

	local hover = self.hover
	local underShadow = self.underShadow

	if x >= self.x and y >= self.y and x < self.x + self.w and y < self.y + self.h then

		if not self.hover then
			if not ctx.hover then
				self:onHover()
			end
			self.hover = true
		end

		if ctx.hover then
			if not self.underShadow then
				self.underShadow = true
			end
		else
			if self.underShadow then
				self.underShadow = false
				self:onHover()
			end			
		end

		if not ctx.hover then
			ctx.hover = true
		end

	else 
		if self.hover then
			if not self.underShadow then
				self:onHoverLost()
			end
			self.hover = false
			self.underShadow = false
		end
	end

	if self.hover ~= hover or self.underShadow ~= underShadow then self:onHoverStatesChange() end
end

function Hover:onHover() end
function Hover:onHoverLost() end
function Hover:onHoverStatesChange() end

return Hover

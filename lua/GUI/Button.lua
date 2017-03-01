
local Music = require("Music")

local M_onHover = Music:new("D:\\steamGames\\steamapps\\common\\Factorio\\data\\base\\sound\\wooden-chest-open.ogg")
local M_onHoverLost = Music:new("D:\\steamGames\\steamapps\\common\\Factorio\\data\\base\\sound\\wooden-chest-close.ogg")
 
 local HoverPush = require("GUI\\HoverPush")
 local Rect = require("GUI\\Rect")
 local Text = require("GUI\\Text")
 local Fonts = require("GUI\\Fonts")

 local Button = HoverPush:extend()

function Button:initialize(x,y,w,h,caption,cb)
	HoverPush.initialize(self,x,y,w,h,cb)
	self.bx = x
	self.by = y
	self.id = "button_" .. self:getIDFor("button")
	self:updateColor()
	self.rect = self:addChild(Rect:new(x,y,w,h,self.r,self.g,self.b))
	self.caption = self:addChild(Text:new(x + 3, y + 3, caption, Fonts.basic, 0, 0, 0))
end

function Button:updateColor()
	if self.hover then
		if self.underShadow then
			self.r = 200; self.g = 0; self.b = 200
		else
			self.r = 255; self.g = 0; self.b = 255
		end		
	else
		self.r = 150; self.g = 0; self.b = 150
	end
end

function Button:onHoverStatesChange()
	self:updateColor()
	self.rect.r = self.r
	self.rect.g = self.g
	self.rect.b = self.b
	self.rect:setProp()
	-- lprint("Window:onHoverStatesChange() " .. self.x)
end

function Button:onPushStateChange()

	self.rect.x = self.x
	self.rect.y = self.y

	self.caption.x = self.x + 3
	self.caption.y = self.y + 3

	if self.pushState then

		self.rect.x = self.x + 1
		self.rect.y = self.y + 1

		self.caption.x = self.x + 4
		self.caption.y = self.y + 4
	end

	self.rect:setProp()
	self.caption:setProp()


	-- lprint("Window:onHoverStatesChange() " .. self.x)
end

function Button:onParentChangePosition(parent)

	self.x = parent.x + self.bx
	self.y = parent.y + self.by

	self.rect.x = self.x
	self.rect.y = self.y
	self.rect:setProp()

	self.caption.x = self.x + 3
	self.caption.y = self.y + 3
	self.caption:setProp()

	return true

end

function Button:onHover()
	HoverPush.onHover(self)
	M_onHover:play()
end
function Button:onHoverLost()
	HoverPush.onHoverLost(self)
	M_onHoverLost:play()
end

return Button
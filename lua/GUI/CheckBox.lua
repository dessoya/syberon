local HoverPush = require("GUI\\HoverPush")
local Text = require("GUI\\Text")
local Image = require("GUI\\Image")
local Fonts = require("GUI\\Fonts")

local CheckBox = HoverPush:extend()
local image

function CheckBox:initialize(x, y, caption, cb)

	self.cb = cb
	local f = function()

		self:setCheckState(not self.CheckBox_state)

		local cb = self.cb
		if type(cb) == "function" then cb(self.CheckBox_state) end
	end

	HoverPush.initialize(self, x, y, 35, 40, f)
	
	self.bx = x
	self.by = y
	self.id = "checkbox_" .. self:getIDFor("checkbox")

	self.image = self:addChild(Image:new(x - 11, y, image, 11, 0, 30, 27, true))
	self:setCheckState(false)

	self.caption = self:addChild(Text:new(x + 37, y + 6, caption, Fonts.basic, 255, 255, 255))
end

function CheckBox:onParentChangePosition(parent)

	self.x = parent.x + self.bx
	self.y = parent.y + self.by

	self.caption.x = self.x + 37
	self.caption.y = self.y + 6
	self.caption:setProp()

	self:setCheckState(self.CheckBox_state)

	return true

end

function CheckBox:onPushStateChange()
end

function CheckBox:setCheckState(state)
	self.CheckBox_state = state
	if self.CheckBox_state then
		self.image.x = self.x - 6
		self.image.y = self.y - 5
		self.image.sx = 41
		self.image.sy = 176
		self.image.sw = 42
		self.image.sh = 42
	else
		self.image.x = self.x
		self.image.y = self.y
		self.image.sx = 2
		self.image.sy = 181
		self.image.sw = 30
		self.image.sh = 30
	end
	self.image:setProp()
end

function CheckBox.init(_image)
	image = _image
end

return CheckBox
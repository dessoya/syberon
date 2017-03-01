local HoverPush = require("GUI\\HoverPush")
local Text = require("GUI\\Text")
local Image = require("GUI\\Image")
local Fonts = require("GUI\\Fonts")

local RadioBox = HoverPush:extend()

local image

function RadioBox:initialize(x, y, caption, cb)

	self.cb = cb
	local f = function()

		if not self.CheckBox_state then
			local cb = self.cb
			if type(cb) == "function" then cb(self.caption.text, self.CheckBox_state) end
		end
		
		self:setCheckState(true)

	end

	HoverPush.initialize(self, x, y, 35, 40, f)
	
	self.bx = x
	self.by = y
	self.id = "radiobox_" .. self:getIDFor("radiobox")

	self.image = self:addChild(Image:new(x - 11, y, image, 11, 0, 30, 27, true))
	self:setCheckState(false)

	self.caption = self:addChild(Text:new(x + 37, y + 3, caption, Fonts.basic, 255, 255, 255))
end

function RadioBox:onParentChangePosition(parent)

	self.x = parent.x + self.bx
	self.y = parent.y + self.by

	self.caption.x = self.x + 37
	self.caption.y = self.y + 3
	self.caption:setProp()

	self:setCheckState(self.CheckBox_state)

	return true

end

function RadioBox:onPushStateChange()
end

function RadioBox:setCheckState(state)
	self.CheckBox_state = state
	if self.CheckBox_state then
		self.image.x = self.x - 7
		self.image.y = self.y - 6
		self.image.sx = 38
		self.image.sy = 126
		self.image.sw = 45
		self.image.sh = 45
	else
		self.image.x = self.x
		self.image.y = self.y
		self.image.sx = 0
		self.image.sy = 132
		self.image.sw = 30
		self.image.sh = 30
	end
	self.image:setProp()
end

function RadioBox.init(_image)
	image = _image
end


return RadioBox
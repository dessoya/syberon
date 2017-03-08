local Object = require("GUI\\Object")
local Box = require("GUI\\Box")
local Rect = require("GUI\\Rect")
local Text = require("GUI\\Text")
local Fonts = require("GUI\\Fonts")
local Timer = require("Timer")

local ProgressBar = Object:extend()

function ProgressBar:initialize(caption, count)

	self.x = 0
	self.y = 0
	self.ry = 0
	self.w = 400
	self.h = 40
	self.yoffset = 0

	self.count = count
	self.current = 0

	self.caption = caption
	self.id = "progressbar " .. self:getIDFor("progressbar")
	self.stage = 0

	self.timer = Timer:new(15, function()

		if self.stage > 90 then
			self.stage = -1
			self.timer:kill()
		else
			self.stage = self.stage + 1		
		end
		self:updateYOffset()
		self:updateProp()
	end)

	self.p = 0

	self.box = self:addChild(Box:new(self.x, self.y, self.w, self.h, 255,255,255))
	self.rect = self:addChild(Rect:new(self.x + 3, self.y + 3, 1, self.h - 6, 255,255,255))
	self.text = self:addChild(Text:new(self.x, self.y + 3 + self.h, self.caption .. " " .. self.p .. "%", Fonts.s14, 255,255,255))

	self:updateYOffset()
	self:updateProp()

end

function ProgressBar:updateYOffset()
	if self.stage == -1 then
		self.yoffset = 0
	else
		self.yoffset = -1 * math.floor(math.abs(math.cos(self.stage / 12)) * ( 400 - (self.stage * 3) ))
	end
end

function ProgressBar:onWindowSize(w, h)
	self.x = w / 2 - self.w / 2
	self.ry = h / 2 - self.h / 2
	self:updateProp()
end

function ProgressBar:updateProp()	

	self.y = self.ry + self.yoffset
	self.box.x = self.x
	self.box.y = self.y

	self.box:setProp()

	self.rect.x = self.x + 3
	self.rect.y = self.y + 3

	self.rect:setProp()

	self.text.x = self.x
	self.text.y = self.y + 3 + self.h

	self.text:setProp()
end

function ProgressBar:inc()

	self.current = self.current + 1

	self.p = self.current / self.count * 100
	-- lprint("ProgressBar:inc " .. self.current .. " " .. self.p)

	self.rect.w = math.floor((self.p / 100) * (self.w - 6))
	self.rect:setProp()

	self.text.text = self.caption .. " " .. math.ceil(self.p) .. "%"
	self.text:setProp()

end

return ProgressBar 
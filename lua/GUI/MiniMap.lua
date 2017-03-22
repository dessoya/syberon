
local GUIObject = require("GUI\\Object")
local Rect = require("GUI\\Rect")
local Image = require("GUI\\Image")
local ThreadHost = require("Thread\\Host")
local Const = require("World\\Const")

local MiniMap = GUIObject:extend()

local maxW, maxH = 600, 600
local scale = 1

function MiniMap:initialize(r, m, px, py, ownerQueue, rcList)

	self.id = "MiniMap_" .. self:getIDFor("MiniMap")

	self:setSize(r:getCurrentViewWidth(), r:getCurrentViewHeight())
	-- self.rect = self:addChild(Rect:new(self.x, self.y, self.w, self.h, 100, 100, 100))

	self.img = { C_Image_NewEmpty(maxW, maxH), C_Image_NewEmpty(maxW, maxH) }
	self.imgIndex = 1

	self.rescaleInProgress = false

	self.drawer = ThreadHost:new("GUI\\MiniMapDrawer", ownerQueue, self.img[1], self.img[2], px, py, m._ptr, rcList)

	self:rescale(scale, true)


	self.image = self:addChild(Image:new(
		self.x + 5, self.y + 5,
		self.img[self.imgIndex],
		maxW/2 - (self.w - 10) / 2,
		maxH/2 - (self.h - 10) / 2,
		self.w - 10,
		self.h - 10,
		false)
	)
end                                                                         

function MiniMap:onDeleteFromRenderer()
	self.drawer:send(Const.CMD_Quit)
end

function MiniMap:send()

	self.rescaleInProgress = true
	local newIndex
	if self.imgIndex == 1 then newIndex = 2
	else newIndex = 1 end
	self.drawer:send(Const.CMD_Rescale, scale, newIndex)

end

function MiniMap:rescale(_scale, force)

	if force == nil then force = false end

	if force or scale ~= _scale then
		scale = _scale
		if not self.rescaleInProgress then
			self:send()
		end
	end
end

function MiniMap:rescaleDone(_scale, _index)
	self.rescaleInProgress = false
	self.image.image = self.img[_index]
	self.image:setProp()
	self.imgIndex = _index
	if scale ~= _scale then
		self:send()
	end
end

function MiniMap:onMouseWheel(w)

	local delta = 0

	if w < 0 and scale > 1 then
		delta = - 1
	elseif w > 0 and scale < 3 then
		delta = 1
	end

	if delta ~= 0 then
		self:rescale(scale + delta)
	end

end

function MiniMap:setSize(w, h)

	if w > maxW + 40 then
		self.w = maxW
		self.x = (w - maxW) / 2
	else
		self.w = w - 40
		self.x = 20
	end

	if h > maxH + 40 then
		self.h = maxH
		self.y = (h - maxH) / 2
	else
		self.h = h - 40
		self.y = 20
	end
end

function MiniMap:onWindowSize(w, h)

	self:setSize(w, h)

--[[
	self.rect.x = self.x
	self.rect.y = self.y
	self.rect.w = self.w
	self.rect.h = self.h

	self.rect:setProp()
	]]

	self.image.x = self.x + 5
	self.image.y = self.y + 5
	self.image.sw = self.w - 10
	self.image.sh = self.h - 10

	self.image.sx = (maxW - (self.w - 10)) / 2
	self.image.sy = (maxH - (self.h - 10)) / 2

	self.image:setProp()

	self:invokeChilds("onParentChangePosition", self)

end

return MiniMap
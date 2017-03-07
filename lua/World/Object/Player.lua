local Object = require("World\\Object")
local Image = require("GUI\\Image")
local Rect = require("GUI\\Rect")
local GUIConst = require("GUI\\Const")

local Player = Object:extend()
local images = { }

function Player.init(idle, run)
	images.idle = idle
	images.run = run
end

function Player:initialize(x, y)
	Object.initialize(self, x, y)
	self.type = "player"
	self.dir = -1

	self.phase = 0
end

function Player:addToRenderer(renderer)
	-- self.image = Image:new(self.x, self.y, images.idle, 0, 0, 53, 73, true)
	self.image = Image:new(self.x, self.y, images.run, 0, 0, 48, 71, true)
	-- self.rect = Rect:new(self.x, self.y + 80, 50, 50, 255, 255, 255)
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Objects)
		-- renderer:add(self.rect, GUIConst.Layers.Objects)
	end)
end

function Player:_serialize(data)
	data.dir = self.dir
end

function Player:update(data)

	if self.dir ~= data.dir then
		self.dir = data.dir

		if self.dir ~= -1 then
			self.image.sy = self.dir * 71
			self.image:setProp()
		end
	end

end

local k = math.pi / 4
local k2 = math.pi + math.pi / 2
local v = 160
local vm = v / 1000000

function Player:treat()

	-- take counter
	local c = self:getCounter()
	if c == -1 then return end

	if self.dir ~= -1 then

		local v = c * vm

		local dx = ( math.cos(self.dir * k + k2) * v )
		local dy = ( math.sin(self.dir * k + k2) * v )
		self.x = self.x + dx
		self.y = self.y + dy

		self.image.x = self.x
		self.image.y = self.y

		--[[
		self.rect.x = self.x
		self.rect.y = self.y + 80
		self.rect:setProp()
		]]

		self.phase = self.phase + ( c / 25000 )
		if self.phase >= 22 then
			self.phase = self.phase - math.floor(self.phase / 22) * 22
		end

		self.image.sx = math.floor(self.phase) * 48

		self.image:setProp()
	end

end

return Player
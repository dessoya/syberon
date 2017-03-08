local Object = require("World\\Object")
local Image = require("GUI\\Image")
local Rect = require("GUI\\Rect")
local GUIConst = require("GUI\\Const")

local Player = Object:extend()
local images = { }


function Player.init(idle, run, _options)
	Player.options = options
	images.idle = idle
	images.run = run
end

function Player:initialize(x, y, rmap, interfaceOption)
	Object.initialize(self, x, y)
	self.interfaceOption = interfaceOption
	self.rmap = rmap
	self.type = "player"
	self.dir = -1
	self.idleDir = 4

	self.phase = 0
end

function Player:onInterfaceUpdate(interfaceOption)
	self.interfaceOption = interfaceOption
	self:afterChangeCoords()
end

function Player:addToRenderer(renderer)
	self.image = Image:new(self.x, self.y, images.idle, 0, self.idleDir * 73, 53, 73, true)
	-- self.image = Image:new(self.x, self.y, images.run, 0, 0, 48, 71, true)

	self:afterChangeCoords()

	-- self.rect = Rect:new(self.x, self.y + 80, 50, 50, 255, 255, 255)
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Objects)
		-- renderer:add(self.rect, GUIConst.Layers.Objects)
	end)
end

function Player:afterChangeCoords()

	if self.interfaceOption ~= nil then

		if self.interfaceOption.centerCamera then

			if self.dir ~= -1 then
				self.image.x = math.floor(self.rmap.vw / 2 - 48 / 2)
				self.image.y = math.floor(self.rmap.vh / 2 - 71 / 2)
			else
				self.image.x = math.floor(self.rmap.vw / 2 - 53 / 2)
				self.image.y = math.floor(self.rmap.vh / 2 - 73 / 2)
			end
			self.image:setProp()

			self.rmap.nx = math.floor(self.x - self.rmap.vw / 2)
			self.rmap.ny = math.floor(self.y - self.rmap.vh / 2)

			self.rmap:setCoords()
		else
			if self.dir ~= -1 then
				self.image.x = self.x - self.rmap.x - 48 / 2
				self.image.y = self.y - self.rmap.y - 71 / 2
			else
				self.image.x = self.x - self.rmap.x - 53 / 2
				self.image.y = self.y - self.rmap.y - 73 / 2
			end
			self.image:setProp()

			if self.x - self.rmap.x < 100 then
				self.rmap.nx = math.floor(self.x - 100)
			end

			if self.x - self.rmap.x > self.rmap.vw - 100 then
				self.rmap.nx = math.floor(self.x - (self.rmap.vw - 100))
			end

			if self.y - self.rmap.y < 100 then
				self.rmap.ny = math.floor(self.y - 100)
			end

			if self.y - self.rmap.y > self.rmap.vh - 100 then
				self.rmap.ny = math.floor(self.y - (self.rmap.vh - 100))
			end

			self.rmap:setCoords()
		end

	end
end


function Player:_serialize(data)
	data.dir = self.dir
end

function Player:update(data)

	if self.dir ~= data.dir then

		if data.dir ~= -1 then
			self.idleDir = data.dir
		end

		if self.dir == -1 then

			-- run
			self.image.image = images.run
			self.image.sw = 48
			self.image.sh = 71
			self.phase = 0

		elseif data.dir == -1 then
			-- idle
			self.image.image = images.idle
			self.image.sw = 53
			self.image.sh = 73
			self.phase = 0

		end

		self.dir = data.dir

		if self.dir ~= -1 then
			self.image.sy = self.dir * 71
			self.image.sx = math.floor(self.phase) * 48
		else
			self.image.sy = self.idleDir * 73
			self.image.sx = math.floor(self.phase) * 53
		end

		self:afterChangeCoords()
		-- self.image:setProp()

	end

end

local k = math.pi / 4
local k2 = math.pi + math.pi / 2
local v = 130
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


		self.phase = self.phase + ( c / 25000 )
		if self.phase >= 22 then
			self.phase = self.phase - math.floor(self.phase / 22) * 22
		end

		self.image.sx = math.floor(self.phase) * 48

		-- self.image:setProp()
		self:afterChangeCoords()
	else

		self.phase = self.phase + ( c / 175000 )
		if self.phase >= 22 then
			self.phase = self.phase - math.floor(self.phase / 22) * 22
		end

		self.image.sx = math.floor(self.phase) * 53
		self.image:setProp()

	end

end

return Player
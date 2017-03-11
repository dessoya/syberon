local Object = require("World\\Object")
local Image = require("GUI\\Image")
local Rect = require("GUI\\Rect")
local GUIConst = require("GUI\\Const")

local Player = Object:extend()
local images = { }
local psizes = { }
local scale = require("scale")


function Player.init(idle, run, _options)
	Player.options = options
	images.idle = { }	
	images.run = { }
	psizes.idle = { }
	psizes.run = { }
	C_Image_disableDDS(idle)
	C_Image_disableDDS(run)
	for i = 1, scale.count do
		local s = scale.d[i]
		if s[1] == -1 then
			images.idle[i] = idle
			images.run[i] = run
			psizes.idle[i] = { 53, 73 }
			psizes.run[i] = { 48, 71 }
		else
			images.idle[i] = C_Image_scale(idle, s[1], s[2])
			images.run[i] = C_Image_scale(run, s[1], s[2])
			psizes.idle[i] = { 53 * s[1] / s[2], 73 * s[1] / s[2] }
			psizes.run[i] = { 48 * s[1] / s[2], 71 * s[1] / s[2] }
		end
	end
	-- dump(psizes)
end

function Player:initialize(x, y, rmap, interfaceOption)
	Object.initialize(self, x, y)
	self.interfaceOption = interfaceOption
	self.rmap = rmap
	self.type = "player"
	self.dir = -1
	self.idleDir = 4

	-- self.scale = 8

	self.phase = 0
end

function Player:onInterfaceUpdate(interfaceOption)
	self.interfaceOption = interfaceOption
	self:afterChangeCoords()
end

function Player:addToRenderer(renderer, rmap)

	local k = self.rmap:getScaleK()

	--lprint("scale " .. self.scale)
	--dump(psizes)

	self.scale = rmap.scale + 1

	self.image = Image:new(self.x / k, self.y / k, images.idle[self.scale], 0, 
		math.floor(self.idleDir * psizes.idle[self.scale][2]),
		math.floor(psizes.idle[self.scale][1]),
		math.floor(psizes.idle[self.scale][2]),
		true)
	-- self.image = Image:new(self.x, self.y, images.run, 0, 0, 48, 71, true)

	-- self.image2 = Image:new(self.x / k, self.y / k, images.idle[self.scale], 0, 0, 200, 200, true)

	self:afterChangeCoords()

	-- self.rect = Rect:new(self.x, self.y + 80, 50, 50, 255, 255, 255)
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Objects)
		-- renderer:add(self.image2, GUIConst.Layers.Objects)
		-- renderer:add(self.rect, GUIConst.Layers.Objects)
	end)
end

function Player:afterChangeCoords()

	if self.interfaceOption ~= nil then

		local k = self.rmap:getScaleK()
		if self.interfaceOption.centerCamera then

			if self.dir ~= -1 then
				self.image.x = math.floor(self.rmap.vw / 2 - psizes.run[self.scale][1] / 2)
				self.image.y = math.floor(self.rmap.vh / 2 - psizes.run[self.scale][2] / 2)
			else
				self.image.x = math.floor(self.rmap.vw / 2 - psizes.idle[self.scale][1] / 2)
				self.image.y = math.floor(self.rmap.vh / 2 - psizes.idle[self.scale][2] / 2)
			end

			self.rmap.nx = math.floor(self.x - self.rmap.vw * k / 2)
			self.rmap.ny = math.floor(self.y - self.rmap.vh * k / 2)

		else
			if self.dir ~= -1 then
				self.image.x = (self.x - self.rmap.x) / k - psizes.run[self.scale][1] / 2
				self.image.y = (self.y - self.rmap.y) / k - psizes.run[self.scale][2] / 2
			else
				self.image.x = (self.x - self.rmap.x) / k - psizes.idle[self.scale][1] / 2
				self.image.y = (self.y - self.rmap.y) / k - psizes.idle[self.scale][2] / 2
			end

			if self.x - self.rmap.x < 512 then
				self.rmap.nx = math.floor(self.x - 512)
			end

			if self.x - self.rmap.x > self.rmap.vw * k - 512 then
				self.rmap.nx = math.floor(self.x - (self.rmap.vw * k - 512))

			end

			if self.y - self.rmap.y < 512 then
				self.rmap.ny = math.floor(self.y - 512)
			end

			if self.y - self.rmap.y > self.rmap.vh * k - 512 then
				self.rmap.ny = math.floor(self.y - (self.rmap.vh * k - 512))
			end

			if self.dir ~= -1 then
				self.image.x = (self.x - self.rmap.nx) / k - psizes.run[self.scale][1] / 2
				self.image.y = (self.y - self.rmap.ny) / k - psizes.run[self.scale][2] / 2
			else
				self.image.x = (self.x - self.rmap.nx) / k - psizes.idle[self.scale][1] / 2
				self.image.y = (self.y - self.rmap.ny) / k - psizes.idle[self.scale][2] / 2
			end			

		end
		self.image:setProp()
		self.rmap:setCoords()
	else
		lprint("no interface options")
	
	end

end


function Player:_serialize(data)
	data.dir = self.dir
end

function Player:afterChangeScale(rmap)

	self.scale = rmap.scale + 1

	if self.dir ~= -1 then

		-- run
		self.image.image = images.run[self.scale]
		self.image.sw = math.floor(psizes.run[self.scale][1])
		self.image.sh = math.floor(psizes.run[self.scale][2])
		self.image.sy = math.floor(self.dir * psizes.run[self.scale][2])
		self.image.sx = math.floor(math.floor(self.phase) * psizes.run[self.scale][1])

	else
		-- idle
		self.image.image = images.idle[self.scale]
		self.image.sw = math.floor(psizes.idle[self.scale][1])
		self.image.sh = math.floor(psizes.idle[self.scale][2])
		self.image.sy = math.floor(self.idleDir * psizes.idle[self.scale][2])
		self.image.sx = math.floor(math.floor(self.phase) * psizes.idle[self.scale][1])

	end

	self:afterChangeCoords()
	-- self:afterChangeCoords()

end

function Player:update(data, rmap)

	self.scale = rmap.scale + 1

	if self.dir ~= data.dir then

		if data.dir ~= -1 then
			self.idleDir = data.dir
		end

		if self.dir == -1 then

			-- run
			self.image.image = images.run[self.scale]
			self.image.sw = math.floor(psizes.run[self.scale][1])
			self.image.sh = math.floor(psizes.run[self.scale][2])
			self.phase = 0

		elseif data.dir == -1 then
			-- idle
			self.image.image = images.idle[self.scale]
			self.image.sw = math.floor(psizes.idle[self.scale][1])
			self.image.sh = math.floor(psizes.idle[self.scale][2])
			self.phase = 0

		end

		self.dir = data.dir

		if self.dir ~= -1 then
			self.image.sy = math.floor(self.dir * psizes.run[self.scale][2])
			self.image.sx = math.floor(math.floor(self.phase) * psizes.run[self.scale][1])
		else
			self.image.sy = math.floor(self.idleDir * psizes.idle[self.scale][2])
			self.image.sx = math.floor(math.floor(self.phase) * psizes.idle[self.scale][1])
		end

		self:afterChangeCoords()
		-- self.image:setProp()

	end

end

local k = math.pi / 4
local k2 = math.pi + math.pi / 2
local v = 600
local vm = v / 1000000

function Player:treat(rmap)

	self.scale = rmap.scale + 1

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

		self.image.sx = math.floor(math.floor(self.phase) * psizes.run[self.scale][1])

		-- self.image:setProp()
		self:afterChangeCoords()
	else

		self.phase = self.phase + ( c / 175000 )
		if self.phase >= 22 then
			self.phase = self.phase - math.floor(self.phase / 22) * 22
		end

		self.image.sx = math.floor(math.floor(self.phase) * psizes.idle[self.scale][1])
		self.image:setProp()

	end

end

return Player
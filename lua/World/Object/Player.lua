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
		
		images.idle[i] = { }
		images.run[i] = { }

		psizes.idle[i] = { }
		psizes.run[i] = { }

		for dir = 1, 8 do
			images.idle[i][dir] = { }
			images.run[i][dir] = { }
			psizes.idle[i][dir] = { }
			psizes.run[i][dir] = { }
			for phase = 1, 22 do
				images.idle[i][dir][phase] = C_Image_scaleEx(idle, s[1], s[2], phase * 53 - 53, dir * 73 - 73, 53 ,73)
				psizes.idle[i][dir][phase] = { 53 * s[1] / s[2], 73 * s[1] / s[2] }

				images.run[i][dir][phase] = C_Image_scaleEx(run, s[1], s[2], phase * 48 - 48, dir * 71 - 71, 48 ,71)
				psizes.run[i][dir][phase] = { 48 * s[1] / s[2], 71 * s[1] / s[2] }
			end
		end
	end
	-- dump(psizes)
end

function Player:initialize(x, y, rmap, interfaceOption, afterChangePosition_cb)
	Object.initialize(self, x, y)
	self.afterChangePosition_cb = afterChangePosition_cb
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

	self.image = Image:new(
		self.x / k, self.y / k,
		images.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1],
		0, 0,
		_tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1]),
		_tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2]),
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
				self.image.x = _tointeger(self.rmap.vw / 2 - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2)
				self.image.y = _tointeger(self.rmap.vh / 2 - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2)
			else
				self.image.x = _tointeger(self.rmap.vw / 2 - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2)
				self.image.y = _tointeger(self.rmap.vh / 2 - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2)
			end

			self.rmap.nx = _tointeger(self.x) - _tointeger(_tointeger(self.rmap.vw * k) / 2)
			self.rmap.ny = _tointeger(self.y) - _tointeger(_tointeger(self.rmap.vh * k) / 2)

		else
			if self.dir ~= -1 then
				self.image.x = (self.x - self.rmap.x) / k - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
				self.image.y = (self.y - self.rmap.y) / k - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
			else
				self.image.x = (self.x - self.rmap.x) / k - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
				self.image.y = (self.y - self.rmap.y) / k - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
			end

			if _tointeger(self.x) - self.rmap.x < 512 then
				self.rmap.nx = _tointeger(self.x) - 512
			end

			if _tointeger(self.x) - self.rmap.x > _tointeger(self.rmap.vw * k) - 512 then
				self.rmap.nx = _tointeger(self.x) - _tointeger(self.rmap.vw * k) + 512

			end

			if _tointeger(self.y) - self.rmap.y < 512 then
				self.rmap.ny = _tointeger(self.y) - 512
			end

			if _tointeger(self.y) - self.rmap.y > _tointeger(self.rmap.vh * k) - 512 then
				self.rmap.ny = _tointeger(self.y) - _tointeger(self.rmap.vh * k) + 512
			end

			if self.dir ~= -1 then
				self.image.x = (self.x - self.rmap.nx) / k - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
				self.image.y = (self.y - self.rmap.ny) / k - psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
			else
				self.image.x = (self.x - self.rmap.nx) / k - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
				self.image.y = (self.y - self.rmap.ny) / k - psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
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

function Player:setScale(scale, scaleK)
-- function Player:afterChangeScale(rmap)

	self.scale = scale + 1

	if self.image ~= nil then
		if self.dir ~= -1 then

			-- run
			self.image.image = images.run[self.scale][self.dir + 1][math.floor(self.phase) + 1]
			self.image.sw = _tointeger(psizes.run[self.scale][self.dir + 1][math.floor(self.phase) + 1][1])
			self.image.sh = _tointeger(psizes.run[self.scale][self.dir + 1][math.floor(self.phase) + 1][2])

			-- self.image.sy = _tointeger(self.dir * psizes.run[self.scale][2])
			-- self.image.sx = _tointeger(_tointeger(self.phase) * psizes.run[self.scale][1])

		else
			-- idle
			self.image.image = images.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1]
			self.image.sw = _tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1])
			self.image.sh = _tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2])
			-- self.image.sy = _tointeger(self.idleDir * psizes.idle[self.scale][2])
			-- self.image.sx = _tointeger(_tointeger(self.phase) * psizes.idle[self.scale][1])

		end

		self:afterChangeCoords()
		-- self:afterChangeCoords()
		
	end

	if self.dir ~= -1 then
		self.w_2 = psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
		self.h_2 = psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
	else
		self.w_2 = psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1] / 2
		self.h_2 = psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2] / 2
	end

end

function Player:update(data, rmap, renderer)

	self.scale = rmap.scale + 1

	if self.dir ~= data.dir then

		if data.dir ~= -1 then
			self.idleDir = data.dir
		end

		if self.dir == -1 then

			-- run
			self.image.image = images.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1]
			self.image.sw = _tointeger(psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1])
			self.image.sh = _tointeger(psizes.run[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2])
			self.phase = 0

		elseif data.dir == -1 then
			-- idle
			self.image.image = images.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1]
			self.image.sw = _tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][1])
			self.image.sh = _tointeger(psizes.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1][2])
			self.phase = 0

		end

		self.dir = data.dir

		--[[
		if self.dir ~= -1 then
			self.image.sy = _tointeger(self.dir * psizes.run[self.scale][2])
			self.image.sx = _tointeger(_tointeger(self.phase) * psizes.run[self.scale][1])
		else
			self.image.sy = _tointeger(self.idleDir * psizes.idle[self.scale][2])
			self.image.sx = _tointeger(_tointeger(self.phase) * psizes.idle[self.scale][1])
		end
		]]

		renderer:lockObjectList()
		self:afterChangeCoords()
		self.afterChangePosition_cb(self)		
		renderer:unlockObjectList()

		-- self.image:setProp()

	end

end

local k = math.pi / 4
local k2 = math.pi + math.pi / 2
-- local v = 660
local v = 660
local vm = v / 1000000

function Player:treat(rmap, renderer)

	self.scale = rmap.scale + 1

	-- take counter
	local c = self:getCounter()
	if c == -1 then return end

	if self.dir ~= -1 then

		renderer:lockObjectList()

		local v = c * vm

		local dx = ( math.cos(self.dir * k + k2) * v )
		local dy = ( math.sin(self.dir * k + k2) * v )
		self.x = self.x + dx
		self.y = self.y + dy

		self.phase = self.phase + ( c / 25000 )
		if self.phase >= 22 then
			self.phase = self.phase - _tointeger(self.phase / 22) * 22
		end

		-- self.image.sx = _tointeger(_tointeger(self.phase) * psizes.run[self.scale][1])
		self.image.image = images.run[self.scale][self.dir + 1][math.floor(self.phase) + 1]

		-- self.image:setProp()
		self:afterChangeCoords()

		self.afterChangePosition_cb(self)		

		renderer:unlockObjectList()
	else

		self.phase = self.phase + ( c / 175000 )
		if self.phase >= 22 then
			self.phase = self.phase - _tointeger(self.phase / 22) * 22
		end

		-- self.image.sx = _tointeger(_tointeger(self.phase) * psizes.idle[self.scale][1])

		-- lprint("self.scale "..self.scale.." self.idleDir + 1 " .. (self.idleDir + 1) .. " self.phase + 1 " .. (self.phase + 1))
		-- dump(images.idle)

		self.image.image = images.idle[self.scale][self.idleDir + 1][math.floor(self.phase) + 1]

		self.image:setProp()

	end

end

return Player
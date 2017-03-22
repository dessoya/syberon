local Object = require("World\\Object")
local Image = require("GUI\\Image")
local Rect = require("GUI\\Rect")
local GUIConst = require("GUI\\Const")

local Player = Object:extend()
local images = { }
local psizes = { }

local scale, scaleK, curScale = 0, 0, { }

function Player._setScale(_scale, _scaleK)
	scale = _scale + 1
	scaleK = _scaleK
	curScale.run = psizes.run[scale]
	curScale.idle = psizes.idle[scale]
end


function Player.init()

	local scale = require("scale")

	images.idle = { }	
	images.run = { }

	psizes.idle = { }
	psizes.run = { }

	for i = 1, scale.count do
		local s = scale.d[i]
		
		images.idle[i] = { }
		images.run[i] = { }

		psizes.idle[i] = { }
		psizes.run[i] = { }

		for dir = 1, 8 do
			images.idle[i][dir - 1] = { }
			images.run[i][dir - 1] = { }
			psizes.idle[i][dir - 1] = { }
			psizes.run[i][dir - 1] = { }
			for phase = 1, 22 do
				images.idle[i][dir - 1][phase - 1] = C_Image_get("player-basic-idle.png_" .. phase .. "x" .. dir .. "_" .. i)
				psizes.idle[i][dir - 1][phase - 1] = {
					w   = _tointeger(53 * s[1] / s[2]),
					h   = _tointeger(73 * s[1] / s[2]),
					w_2 = _tointeger(53 * s[1] / s[2] / 2),
					h_2 = _tointeger(73 * s[1] / s[2] / 2)
				}

				images.run[i][dir - 1][phase - 1] = C_Image_get("player-basic-run.png_" .. phase .. "x" .. dir .. "_" .. i)
				psizes.run[i][dir - 1][phase - 1] = {
					w   = _tointeger(48 * s[1] / s[2]),
					h   = _tointeger(71 * s[1] / s[2]),
					w_2 = _tointeger(48 * s[1] / s[2] / 2),
					h_2 = _tointeger(71 * s[1] / s[2] / 2)
				}
			end
		end
	end
	-- dump(psizes)
end

function Player:initialize(x, y, rmap, renderer, interfaceOption, afterChangePosition_cb, afterChangePosition_ctx)
	Object.initialize(self, x, y)
	self.afterChangePosition_cb = afterChangePosition_cb
	self.afterChangePosition_ctx = afterChangePosition_ctx
	self.interfaceOption = interfaceOption
	self.rmap = rmap
	self.renderer = renderer
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
	
	local p = curScale.idle
	self.image = Image:new(
		self.x / scaleK,
		self.y / scaleK,
		images.idle[scale][self.idleDir][math.floor(self.phase)],
		0, 0,
		_tointeger(p[self.idleDir][math.floor(self.phase)].w),
		_tointeger(p[self.idleDir][math.floor(self.phase)].h),
		true)

	self:afterChangeCoords()

	-- self.rect = Rect:new(self.x, self.y + 80, 50, 50, 255, 255, 255)
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Objects)
	end)
end

function Player:afterChangeCoords()

	local p = math.floor(self.phase)
	local id = self.idleDir
	local idle, run = curScale.idle[id][p], curScale.run[id][p]
	local w, h = self.rmap.vw, self.rmap.vh
	local w_2, h_2 = w / 2, h / 2
	local rmap = self.rmap
	local image = self.image

	if self.interfaceOption ~= nil then

		if self.interfaceOption.centerCamera then

			local sz
			if self.dir ~= -1 then sz = run
			else sz = idle end

			image.x = _tointeger(w_2 - sz.w_2)
			image.y = _tointeger(h_2 - sz.h_2)

			self.rx = image.x + sz.w_2
			self.ry = image.y + sz.h_2			

			rmap.nx = _tointeger(self.x) - _tointeger(_tointeger(w * scaleK) / 2)
			rmap.ny = _tointeger(self.y) - _tointeger(_tointeger(h * scaleK) / 2)

		else

			local sz
			if self.dir ~= -1 then sz = run
			else sz = idle end

			image.x = (self.x - rmap.x) / scaleK - sz.w_2
			image.y = (self.y - rmap.y) / scaleK - sz.h_2

			if self.x - rmap.x < 512 then
				rmap.nx = self.x - 512
			end

			if self.x - rmap.x > w * scaleK - 512 then
				rmap.nx = self.x - w * scaleK + 512
			end

			if self.y - rmap.y < 512 then
				rmap.ny = self.y - 512
			end

			if self.y - rmap.y > h * scaleK - 512 then
				self.rmap.ny = self.y - h * scaleK + 512
			end

			image.x = (self.x - rmap.nx) / scaleK - sz.w_2
			image.y = (self.y - rmap.ny) / scaleK - sz.h_2

			self.rx = image.x + sz.w_2
			self.ry = image.y + sz.h_2

		end
		image:setProp()
		rmap:setCoords()

	else
		lprint("no interface options")
	end

end


function Player:_serialize(data)
	data.dir = self.dir
end

function Player:setScale()

	local p = math.floor(self.phase)
	local id = self.idleDir
	local idle, run = curScale.idle[id][p], curScale.run[id][p]

	local sz
	if self.dir ~= -1 then sz = run
	else sz = idle end

	if self.image ~= nil then

		if self.dir ~= -1 then

			-- run
			self.image.image = images.run[scale][self.dir][p]
			self.image.sw = curScale.run[self.dir][p].w
			self.image.sh = curScale.run[self.dir][p].h

		else

			-- idle
			self.image.image = images.idle[scale][id][p]
			self.image.sw = idle.w
			self.image.sh = idle.h

		end

		self:afterChangeCoords()
		
	end

	self.w_2 = sz.w_2
	self.h_2 = sz.h_2

end

function Player:update(data, rmap, renderer)

	if self.dir ~= data.dir then

		if data.dir ~= -1 then
			self.idleDir = data.dir
		end

		if self.dir == -1 then

			-- run
			self.phase = 0
			self.image.image = images.run[scale][self.idleDir][0]
			self.image.sw = curScale.run[self.idleDir][0].w
			self.image.sh = curScale.run[self.idleDir][0].h

		elseif data.dir == -1 then

			-- idle
			self.phase = 0
			self.image.image = images.idle[scale][self.idleDir][0]
			self.image.sw = curScale.idle[self.idleDir][0].w
			self.image.sh = curScale.idle[self.idleDir][0].h

		end

		self.dir = data.dir

		renderer:lockObjectList()
		self:afterChangeCoords()
		self.afterChangePosition_cb(self, self.afterChangePosition_ctx)		
		renderer:unlockObjectList()

	end

end

local k = math.pi / 4
local k2 = math.pi + math.pi / 2
-- local v = 660
local v = 1560
local vm = v / 1000000

function Player:treat()

	-- take counter
	local c = self:getCounter()
	if c == -1 then return end

	if self.dir ~= -1 then

		self.renderer:lockObjectList()

		local v = c * vm

		local dx = ( math.cos(self.dir * k + k2) * v )
		local dy = ( math.sin(self.dir * k + k2) * v )
		self.x = self.x + dx
		self.y = self.y + dy

		self.phase = self.phase + ( c / 25000 )
		if self.phase >= 22 then
			self.phase = self.phase - _tointeger(self.phase / 22) * 22
		end

		self.image.image = images.run[scale][self.dir][math.floor(self.phase)]

		self:afterChangeCoords()
		self.afterChangePosition_cb(self, self.afterChangePosition_ctx)		

		self.renderer:unlockObjectList()
	else

		self.phase = self.phase + ( c / 175000 )
		if self.phase >= 22 then
			self.phase = self.phase - _tointeger(self.phase / 22) * 22
		end

		self.image.image = images.idle[scale][self.idleDir][math.floor(self.phase)]
		self.image:setProp()

	end

end

return Player
local Object = require("World\\Object")
local Image = require("GUI\\Image")
local GUIConst = require("GUI\\Const")

local Stone = Object:extend()
local images = { }
local psizes = { }

local scale, scaleK, curScale = 0, 0, { }

function Stone._setScale(_scale, _scaleK)
	scale = _scale + 1
	scaleK = _scaleK
	curScale._1 = psizes._1[scale]
end

function Stone.init(image)

	local scale = require("scale")

	images._1 = { }	
	psizes._1 = { }

	for i = 1, scale.count do
		local s = scale.d[i]

		images._1[i] = C_Image_get("png_01\\Stone_03_grass.png_" .. i)
		psizes._1[i] = {
			x   = _tointeger(21  * s[1] / s[2]),
			y   = _tointeger(80  * s[1] / s[2]),
			w   = _tointeger(161 * s[1] / s[2]),
			h   = _tointeger(96  * s[1] / s[2]),
			w_2 = _tointeger(161 * s[1] / s[2] / 2),
			h_2 = _tointeger(96  * s[1] / s[2] / 2)
		}
		
	end

end

function Stone:initialize(x, y)
	Object.initialize(self, x, y)
	self.type = "stone"
	self.variant = 1
	self.stoneVariant = "_1"
end

function Stone:getViewCoords(rmap, p)

	local x = _tointeger( self.x / scaleK ) - _tointeger( rmap.x / scaleK ) - p.w_2
	local y = _tointeger( self.y / scaleK ) - _tointeger( rmap.y / scaleK ) - p.h_2

	return x, y
end

function Stone:addToRenderer(renderer, rmap)
	
	-- lprint("Stone:addToRenderer")
	local p = curScale[self.stoneVariant]
	local x, y = self:getViewCoords(rmap, p)

	if self.image == nil then
	
		self.image = Image:new(
			x,
			y,
			images[self.stoneVariant][scale],

			_tointeger(p.x), 
			_tointeger(p.y),

			_tointeger(p.w),
			_tointeger(p.h),

			true
		)

	else

		self.image.image = images[self.stoneVariant][scale]
		self.image.x  = x
		self.image.y  = y
		self.image.sx = p.x
		self.image.sy = p.y
		self.image.sw = p.w
		self.image.sh = p.h
		self.image:setProp()

	end

	self.rx = x + p.w_2
	self.ry = y + p.h_2

	-- self:afterChangeCoords()
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Ground)
	end)

	self.added = true

end

function Stone:setScale()

	local p = curScale[self.stoneVariant]

	if self.image ~= nil then

		self.image.image = images[self.stoneVariant][scale]
		self.image.sx = p.x
		self.image.sy = p.y
		self.image.sw = p.w
		self.image.sh = p.h
		self.image:setProp()
		
	end

	self.w_2 = p.w_2
	self.h_2 = p.w_2

end

function Stone:reposition(rmap)

	local p = curScale[self.stoneVariant]
	local x, y = self:getViewCoords(rmap, p)

	self.rx = x + p.w_2
	self.ry = y + p.h_2

	self.image.x = x
	self.image.y = y

	self.image:setProp()
end

function Stone:delFromRenderer(r)
	-- lprint("Stone:delFromRenderer")
	self.added = false
	self.rx = nil
	self.ry = nil
	self.image:delFromRenderer(r)
end

function Stone:_serialize(data)
	data.variant = self.variant
end

function Stone:update(data, rmap, renderer)
end


return Stone
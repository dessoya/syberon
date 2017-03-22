local Const = require("World\\Const")
local Object = require("World\\Object")
local Image = require("GUI\\Image")
local GUIConst = require("GUI\\Const")

local RespawnCrystal = Object:extend()
local images = { }
local psizes = { }

local scale, scaleK, curScale = 0, 0, { }

function RespawnCrystal._setScale(_scale, _scaleK)
	scale = _scale + 1
	scaleK = _scaleK
	curScale = psizes[scale]
end

function RespawnCrystal.init(image)

	local scale = require("scale")

	images = { }	
	psizes = { }

	for i = 1, scale.count do
		local s = scale.d[i]

		images[i] = C_Image_get("png_02\\Kristal_01.png_" .. i)
		psizes[i] = {
			x   = _tointeger(0  * s[1] / s[2]),
			y   = _tointeger(0  * s[1] / s[2]),
			w   = _tointeger(200* s[1] / s[2]),
			h   = _tointeger(200* s[1] / s[2]),
			w_2 = _tointeger(200* s[1] / s[2] / 2),
			h_2 = _tointeger(200* s[1] / s[2] / 2)
		}
		
	end

end

function RespawnCrystal:initialize(x, y)
	Object.initialize(self, x, y)
	self.a = Array:new()
	self.a:set(0, #OT_RESPAWN_CRYSTAL, self.id)
	self.type = #OT_RESPAWN_CRYSTAL
end

function RespawnCrystal:getViewCoords(rmap, p)

	local x = _tointeger( self.x / scaleK ) - _tointeger( rmap.x / scaleK ) - p.w_2
	local y = _tointeger( self.y / scaleK ) - _tointeger( rmap.y / scaleK ) - p.h_2

	return x, y
end

function RespawnCrystal:addToRenderer(renderer, rmap)
	
	-- lprint("Stone:addToRenderer")
	local p = curScale
	local x, y = self:getViewCoords(rmap, p)

	if self.image == nil then
	
		self.image = Image:new(
			x,
			y,
			images[scale],

			_tointeger(p.x), 
			_tointeger(p.y),

			_tointeger(p.w),
			_tointeger(p.h),

			true
		)

	else

		self.image.image = images[scale]
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

function RespawnCrystal:setScale()

	local p = curScale

	if self.image ~= nil then

		self.image.image = images[scale]
		self.image.sx = p.x
		self.image.sy = p.y
		self.image.sw = p.w
		self.image.sh = p.h
		self.image:setProp()
		
	end

	self.w_2 = p.w_2
	self.h_2 = p.w_2

end

function RespawnCrystal:reposition(rmap)

	local p = curScale
	local x, y = self:getViewCoords(rmap, p)

	self.rx = x + p.w_2
	self.ry = y + p.h_2

	self.image.x = x
	self.image.y = y

	self.image:setProp()
end

function RespawnCrystal:delFromRenderer(r)
	-- lprint("Stone:delFromRenderer")
	self.added = false
	self.rx = nil
	self.ry = nil
	self.image:delFromRenderer(r)
end

function RespawnCrystal:_serialize(data)
end

function RespawnCrystal:update(data, rmap, renderer)
end

return RespawnCrystal
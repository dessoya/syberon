local Object = require("World\\Object")
local Image = require("GUI\\Image")
local GUIConst = require("GUI\\Const")

local Stone = Object:extend()
local images = { }
local psizes = { }
local scale = require("scale")

function Stone.init(image)

	images._1 = { }	
	psizes._1 = { }

	C_Image_disableDDS(image)

	for i = 1, scale.count do
		local s = scale.d[i]

		images._1[i] = C_Image_scale(image, s[1], s[2])
		psizes._1[i] = {
			_tointeger(21  * s[1] / s[2]),
			_tointeger(80  * s[1] / s[2]),
			_tointeger(161 * s[1] / s[2]),
			_tointeger(96  * s[1] / s[2])
		}
		
	end

end

function Stone:initialize(x, y)

	Object.initialize(self, x, y)

	self.type = "stone"

	self.added = false

end

function Stone:addToRenderer(renderer, rmap)
	
	-- lprint("Stone:addToRenderer")

	local k = rmap:getScaleK()
	local s = rmap.scale + 1
	local p = psizes._1[s]

	local x = _tointeger( _tointeger( self.x / k ) - _tointeger( rmap.x / k ) ) - _tointeger( p[3] / 2 )
	local y = _tointeger( _tointeger( self.y / k ) - _tointeger( rmap.y / k ) ) - _tointeger( p[4] / 2 )

	if self.image == nil then
	
		self.image = Image:new(
			x,
			y,
			images._1[s],

			_tointeger(p[1]), 
			_tointeger(p[2]),

			_tointeger(p[3]),
			_tointeger(p[4]),

			true
		)

	else

		self.image.image = images._1[s]
		self.image.x  = x
		self.image.y  = y
		self.image.sx = _tointeger(p[1])
		self.image.sy = _tointeger(p[2])
		self.image.sw = _tointeger(p[3])
		self.image.sh = _tointeger(p[4])
		self.image:setProp()

	end

	-- self:afterChangeCoords()
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Ground)
	end)

	self.added = true

end

function Stone:setScale(scale, scaleK)
-- function Player:afterChangeScale(rmap)

	local s = scale + 1
	local p = psizes._1[s]

	if self.image ~= nil then

		self.image.image = images._1[s]
		self.image.sx = _tointeger(p[1])
		self.image.sy = _tointeger(p[2])
		self.image.sw = _tointeger(p[3])
		self.image.sh = _tointeger(p[4])
		self.image:setProp()
		
	end

	self.w_2 = _tointeger(p[3] / 2)
	self.h_2 = _tointeger(p[4] / 2)

end

function Stone:reposition(rmap, scale, scaleK)

	local s = scale + 1
	local p = psizes._1[s]

	self.image.x = _tointeger( ( _tointeger( self.x / scaleK ) - _tointeger( rmap.x / scaleK ) ) ) - _tointeger( p[3] / 2 )
	self.image.y = _tointeger( ( _tointeger( self.y / scaleK ) - _tointeger( rmap.y / scaleK ) ) ) - _tointeger( p[4] / 2 )

	self.image:setProp()
end

function Stone:delFromRenderer(r)
	-- lprint("Stone:delFromRenderer")
	self.added = false
	self.image:delFromRenderer(r)
end



return Stone
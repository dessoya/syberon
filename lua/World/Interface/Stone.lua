local IObject = require("World\\Interface\\Object")
local Stone = IObject:extend()

local scale = require("scale")
local sizes = { }
for i = 1, scale.count do
	local s = scale.d[i]
	sizes[i] = {
		w   = _tointeger(161 * s[1] / s[2]),
		h   = _tointeger(96  * s[1] / s[2]),
		w_2 = _tointeger(161 * s[1] / s[2] / 2),
		h_2 = _tointeger(96  * s[1] / s[2] / 2)
	}
end

function Stone:initialize(data)	
	IObject.initialize(self, data)
	self.variant = data.variant
end

function Stone:getSize(scale)
	local s = sizes[scale]
	return s.w_2, s.h_2
end


IObject.map.stone = Stone

return Stone
C_InstallModule("map")

local Object = require("Object")

local Map = Object:extend()

function Map:initialize()
	self._ptr = C_Map_New()
end

function Map:set(x, y, id)
	C_Map_set(self._ptr, x, y, id)
end

return Map
 
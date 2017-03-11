C_InstallModule("map")

local Object = require("Object")

local Map = Object:extend()

function Map:initialize()
	self.x = 0
	self.y = 0
	self._ptr = C_Map_New()
end

function Map:set(x, y, id)
	C_Map_set(self._ptr, x, y, id)
end

function Map:dump()
	C_Map_dump(self._ptr)
end

local dot7 = {
	{ 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0 }
}

local dot5 = {
	{ 0, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0 },
}

function Map:dot(x, y, s, id)

	local dot = nil
	if s == 5 then
		x = x - 3
		y = y - 3
		dot = dot5
	end

	if dot == nil then return end

	for i = 1, s do
		for j = 1, s do
			if dot[i][j] == 1 then
				self:set(x + i, y + j, id)
			end
		end
	end

end

return Map
 
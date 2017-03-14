C_InstallModule("random")

local Object = require("Object")
local Random = Object:extend()

function Random:initialize(seed)
	self.seed = seed
end

function Random:get(r)
	local s, inr = C_Random_get(self.seed, r)
	self.seed = s
	return inr
end

--[[
function Random:range(f, t)

	local r = t - f + 0.0
	local j = math.floor(r * (self:get() / (0x7fffffffffff + 1.0))) + t
	return j

end
]]

return Random 
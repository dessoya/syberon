local IObject = require("World\\Interface\\Object")
local Player = IObject:extend()


local scale = require("scale")
local sizes = { }
for i = 1, scale.count do
	local s = scale.d[i]
	sizes[i] = {
		w   = _tointeger(53 * s[1] / s[2]),
		h   = _tointeger(73 * s[1] / s[2]),
		w_2 = _tointeger(53 * s[1] / s[2] / 2),
		h_2 = _tointeger(73 * s[1] / s[2] / 2)
	}
end


function Player:initialize(data)	
	IObject.initialize(self, data)
	self.dir = data.dir
end

local dirs = {
	{ 7, 0, 1 },
	{ 6,-1, 2 },
	{ 5, 4, 3 }
}
function Player:makeDirFromKeys(key, keyCodes, keyStates)

	if key == keyCodes.up or key == keyCodes.down or key == keyCodes.left or key == keyCodes.right then
		local x = 2
		local y = 2
		if keyStates[keyCodes.up]		then y = y - 1 end
		if keyStates[keyCodes.down]		then y = y + 1 end
		if keyStates[keyCodes.left]		then x = x - 1 end
		if keyStates[keyCodes.right]	then x = x + 1 end
		local dir = dirs[y][x]
		if self.dir ~= dir then
			self.dir = dir
			self:updateObject({ dir = dir })
		end
	end

end

function Player:getSize(scale)
	local s = sizes[scale]
	return s.w_2, s.h_2
end

IObject.map.player = Player

return Player 
local IObject = require("World\\Interface\\Object")
local Player = IObject:extend()

function Player:initialize(data)	
	IObject:initialize(data)
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

IObject.map.player = Player

return Player 
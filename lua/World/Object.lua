local Object = require("Object")

local WorldObject = Object:extend()

function WorldObject:initialize(x, y)
	self.x = x
	self.y = y
end




return WorldObject
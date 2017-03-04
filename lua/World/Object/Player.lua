local Object = require("World\\Object")
local Image = require("GUI\\Image")
local GUIConst = require("GUI\\Const")

local Player = Object:extend()
local images = { }

function Player.init(idle, run)
	images.idle = idle
	images.run = run
end

function Player:initialize(x, y)
	Object.initialize(self, x, y)
	self.type = "player"
end

function Player:addToRenderer(renderer)
	self.image = Image:new(self.x, self.y, images.idle, 0, 0, 53, 73, true)
	renderer:modify(function()
		renderer:add(self.image, GUIConst.Layers.Objects)
	end)
end

return Player
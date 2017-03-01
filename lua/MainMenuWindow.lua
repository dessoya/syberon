local Object = require("Object")
local MainMenuWindow = Object:extend()
local GUI = require("GUI")

function MainMenuWindow:initialize(renderer, video_cb, play_cb, quit_cb)

	self.renderer = renderer

	self.window = GUI.Window:new(10, 10, 350, 600)	
	self.window:addChild(GUI.Button:new(10, 10, 330, 25, "Video", video_cb))
	self.window:addChild(GUI.Button:new(10, 45, 330, 25, "Play", play_cb))
	self.window:addChild(GUI.Button:new(10, 565, 330, 25, "Quit", quit_cb))

	self.renderer:add(self.window)
	

end

function MainMenuWindow:del()
	self.renderer:del(self.window)
	self.window = nil
end

return MainMenuWindow 
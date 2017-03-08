local Object = require("Object")
local InterfaceMenuWindow = Object:extend()
local GUI = require("GUI")
local Const = require("World\\Const")

function InterfaceMenuWindow:initialize(renderer, hwnd, optionFile, back_cb)

	self.renderer = renderer
	self.optionFile = optionFile

	self.window = GUI.Window:new(10, 10, 350, 600)	

	self.centerCamera = self.window:addChild(GUI.CheckBox:new(10, 10, "Center camera on player", function(state)

		local g = self.optionFile:getGroup("interface")
		g.centerCamera = state
		self.optionFile:setGroup("interface", g)
		self.optionFile:save()

		-- lprint("Const.CMD_UpdateOptions " .. Const.CMD_UpdateOptions)
		C_Thread_PostMessage(hwnd, Const.CMD_UpdateOptions, Const.Options.Interface, C_PackTable(g))

	end))

	local g = self.optionFile:getGroup("interface")
	self.centerCamera:setCheckState(g.centerCamera)

	self.window:addChild(GUI.Button:new(10, 565, 330, 25, "Back", back_cb))
	
	self.renderer:add(self.window)

end


function InterfaceMenuWindow:del()
	self.renderer:del(self.window)
end


return InterfaceMenuWindow
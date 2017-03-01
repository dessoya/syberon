local Object = require("Object")
local VideoMenuWindow = Object:extend()
local GUI = require("GUI")

function VideoMenuWindow:initialize(renderer, optionFile, back_cb)

	self.renderer = renderer
	self.optionFile = optionFile
	self.modeList = self.renderer:getModeList()

	self.window = GUI.Window:new(10, 10, 350, 600)	

	self.fullscreen = self.window:addChild(GUI.CheckBox:new(10, 10, "Full screen"))

	local onOption = function(name, state)
		self:activateOption(name)
	end

	self.options = {
		["4:3"] = self.window:addChild(GUI.RadioBox:new(10, 60, "4:3", onOption)),
		["16:9"] = self.window:addChild(GUI.RadioBox:new(120, 60, "16:9", onOption)),
		["16:10"] = self.window:addChild(GUI.RadioBox:new(220, 60, "16:10", onOption))
	}

	self.modes = self.window:addChild(GUI.ComboBox:new(10, 102, 330, self.renderer))

	local g = self.optionFile:getGroup("video")
	self:updateVideoOptions(g)

	self.window:addChild(GUI.Button:new(10, 530, 330, 25, "Apply", function()

		local i = self.modes.selectedItem
		if i == nil then return end

		local g = self.optionFile:getGroup("video")

		if g.fullscreen then

			if not self.fullscreen.CheckBox_state then
				self.renderer:disableFullScreen()

			elseif i.w ~= g.screenWidth or i.h ~= g.screenHeight then
				self.renderer:enableFullScreen(i.w, i.h)
			end
		else
			if self.fullscreen.CheckBox_state then
				self.renderer:enableFullScreen(i.w, i.h)
			end
		end

		g.fullscreen = self.fullscreen.CheckBox_state
		g.screenWidth = i.w
		g.screenHeight = i.h

		self.optionFile:save()

	end))

	self.window:addChild(GUI.Button:new(10, 565, 330, 25, "Back", back_cb))
	
	self.renderer:add(self.window)

end

function VideoMenuWindow:activateOption(name)
	
	for optionName, option in pairs(self.options) do
		if name ~= optionName then
			option:setCheckState(false)
		end 
	end

	self.modes:hideList()
	self.modes:setupList(self.modeList[name], 1)
end

function VideoMenuWindow:del()
	self.renderer:del(self.window)
	self.window = nil
	self.fullscreen = nil
	self.op1 = nil
	self.op2 = nil
	self.op3 = nil
	self.modes = nil
	self.modeList = nil	
end

function VideoMenuWindow:updateVideoOptions(g)

	if self.fullscreen == nil then return end

	self.fullscreen:setCheckState(g.fullscreen)

	for ratio, list in pairs(self.modeList) do
		for index, item in pairs(list) do

			if item.w == g.screenWidth and item.h == g.screenHeight then

				if self.options[ratio] ~= nil then
					local ctr = self.options[ratio]
					ctr:setCheckState(true)
					ctr.cb(ratio, true)
					self.modes:setupItem(index)
				end

				return

			end
		end
	end

end


return VideoMenuWindow
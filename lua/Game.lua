C_InstallModule("imagefont")

local Object = require("Object")
local Timer = require("Timer")
local Renderer = require("Renderer")
local GUI = require("GUI")
local WindowsConst = require("Windows\\Const")

local OptionFile = require("OptionFile")
local MessagePump = require("MessagePump")

local ImageLoader = require("ImageLoaderHost")
local World = require("World\\Host")
local Keys = require("Keys")

local MainMenuWindow = require("MainMenuWindow")
local VideoMenuWindow = require("VideoMenuWindow")

local Images = { }

local Game = Object:extend()

function Game:initialize()

	self.messagePump = MessagePump:new({		
		[WindowsConst.WM.R_RecreateSurface]		= "onRecreateSurface",
		[WindowsConst.WM.R_DisableFullscreen]	= "onDisableFullscreen",
		[WindowsConst.WM.R_EnableFullscreen]	= "onEnableFullscreen",
	})

end

function Game:onStart(hwnd)

	self.menuIsOpened = false
	self.gameInProgress = false
	Timer.init(hwnd, self.messagePump)

	self.hwnd = hwnd
	self.renderer = Renderer:new(hwnd, self.messagePump)

	self.messagePump:registerReciever(self)

	self.keys = Keys:new()
	self.keys:attachToPump(self.messagePump)
	self.keys:registerReciever(self)

	Timer:new(1300, function()
		lprint("timer")
	end)

	self.optionFile = OptionFile:new("game.cfg")

	local g = self.optionFile:getGroup("video")
	if g == nil then
		g = { }
	end

	if g.fullscreen == nil then
		g.fullscreen = false
	end

	if g.screenWidth == nil then
		g.screenWidth = self.renderer:getCurrentWidth()
		g.screenHeight = self.renderer:getCurrentHeight()
	end

	self.optionFile:setGroup("video", g)
	self.optionFile:save()

	if g.fullscreen then
		self.renderer:enableFullScreen(g.screenWidth, g.screenHeight)
	end


	Timer:new(300, function()
		self.fpsText.text = "FPS: " .. self.renderer:getFPS()
		self.fpsText:setProp()

		self.ftText.text = "Frame time: " .. C_Renderer_getFrameTime(self.renderer._ptr)
		self.ftText:setProp()

	end)


	self.renderer:modify(function()
		self.background = self.renderer:add(GUI.Background:new())
		self.fpsText = self.renderer:add(GUI.Text:new(10, 10, "FPS: ", GUI.Fonts.basic, 255, 255, 255))		
		self.ftText = self.renderer:add(GUI.Text:new(10, 30, "Frame time: ", GUI.Fonts.basic, 255, 255, 255))		
	end)

	
	self.imageLoader = ImageLoader:new(hwnd, self.messagePump, self.renderer, function(images)
		
		for name, image in pairs(images) do
			Images[name] = image
		end

		self.imageLoader = nil

		self:onLoadResource()

		self.renderer:modify(function()
			self:openMainMenu()
		end)

	end)

	self.world = World:new(self.messagePump, Images, hwnd, self.renderer._ptr)

end

function Game:keyPressed(key, alt)
end

function Game:keyUnPressed(key, alt)
	if key == Keys.Codes.Esc then

		if self.menuIsOpened == false and self.mainMenu == nil then

			self.renderer:modify(function()
				self:openMainMenu()
			end)

		elseif self.gameInProgress then

			if self.mainMenu ~= nil then

				self.renderer:modify(function()
					self.mainMenu:del()
					self.menuIsOpened = false
					self.mainMenu = nil
				end)

			elseif self.videoMenu ~= nil then

				self.renderer:modify(function()
					self.videoMenu:del()
					self.menuIsOpened = false
					self.videoMenu = nil
				end)

			end

		end


	end
end

function Game:onLoadResource()
	-- Images["controls.png"]
	GUI.CheckBox.init(Images["controls.png"])
	GUI.RadioBox.init(Images["controls.png"])

end

function Game:openMainMenu()

	self.menuIsOpened = true
	self.mainMenu = MainMenuWindow:new(

		self.renderer,

		-- video button

		function()
			self.renderer:modify(function()
				self.mainMenu:del()
				self.menuIsOpened = false
				self.mainMenu = nil
				self:openVideoMenu()
			end)
		end,

		-- play  button

		function()
			self.renderer:modify(function()
				self.mainMenu:del()
				self.menuIsOpened = false
				self.mainMenu = nil
				self.renderer:del(self.background)
				-- self.renderer:add(GUI.Image:new(210, 150, Images["ter1.png"], 0, 0, 150, 150, false))
			end)
			self.gameInProgress = true
			self.world:create()
		end,

		-- quit  button

		function()
			C_Exit()
		end
	)
	
end


function Game:openVideoMenu()

	self.menuIsOpened = true

	self.videoMenu = VideoMenuWindow:new(

		self.renderer,
		self.optionFile,

		function()
			self.renderer:modify(function()
				self.videoMenu:del()
				self.menuIsOpened = false
				self.videoMenu = nil
				self:openMainMenu()
			end)
		end

	)

end

function Game:onRecreateSurface()

	lprint("recreate surfaces")
	for key, val in pairs(Images) do
		C_Image_Restore(val)
	end

end

function Game:onDisableFullscreen()

	lprint("Game:onDisableFullscreen")

	local g = self.optionFile:getGroup("video")
	g.fullscreen = false
	self.optionFile:save()

	if self.videoMenu ~= nil then
		self.videoMenu:updateVideoOptions(g)
	end

end

function Game:onEnableFullscreen()

	lprint("Game:onEnableFullscreen")

	local g = self.optionFile:getGroup("video")
	g.fullscreen = true
	g.screenWidth = self.renderer:getCurrentWidth()
	g.screenHeight = self.renderer:getCurrentHeight()
	self.optionFile:save()

	if self.videoMenu ~= nil then
		self.videoMenu:updateVideoOptions(g)
	end

end

function Game:_onWindowMessage(message, lparam, lparam1, lparam2, wparam)
	return self.messagePump:onWindowMessage(message, lparam, lparam1, lparam2, wparam)
end

return Game

#include KeyCodes

C_InstallModule("ds_array")
C_InstallModule("imagefont")

local Object = require("Object")
local Timer = require("Timer")
local Renderer = require("Renderer")
local GUI = require("GUI")
local GUIConst = require("GUI\\Const")
local WindowsConst = require("Windows\\Const")

local OptionFile = require("OptionFile")
local MessagePump = require("MessagePump")

local ImageLoader = require("ImageLoader\\Host")
local World = require("World\\Host")

local Keys = require("Keys")

local MainMenuWindow = require("MainMenuWindow")
local VideoMenuWindow = require("VideoMenuWindow")
local InterfaceMenuWindow = require("InterfaceMenuWindow")

local Random = require("Random")


local Game = Object:extend()

function Array:info()
	lprint("Array:info " .. self:size())
end

function Game:initialize()

	self.messagePump = MessagePump:new({		
		[WindowsConst.WM.R_RecreateSurface]		= "onRecreateSurface",
		[WindowsConst.WM.R_DisableFullscreen]	= "onDisableFullscreen",
		[WindowsConst.WM.R_EnableFullscreen]	= "onEnableFullscreen",
	})

	local a = Array:new()
	a:info()

end


--[[
function Game:onStart(hwnd)
	self.hwnd = hwnd

	local Thread = require("Thread\\Host")
	local cqueue = C_CQueue_new()
	local t = Thread:new("Test", cqueue, "World")

	local a, b = C_CQueue_get(cqueue)
	lprint("data from child")
	lprint("" .. a)
	lprint("" .. b)
	t:send("message",234,345)
	C_Timer_Sleep(5000)
	t:send("quit")
	C_Timer_Sleep(5)

	C_Thread_PostMessage(self.hwnd, WindowsConst.WM.Destroy)
end
]]

function Game:onStart(hwnd)

	--[[
	local r = Random:new(0)
	local f = { }
	for i = 1, 10000 do
		local n = r:get(4)
		-- lprint(n)
		if f[n] == nil then f[n] = 0 end
		f[n] = f[n] + 1		
	end
	dump(f)
	]]

	self.cqueue = C_CQueue_new()

	self.menuIsOpened = false
	self.gameInProgress = false
	Timer.init(hwnd, self.messagePump)

	self.hwnd = hwnd
	self.renderer = Renderer:new(hwnd, self.messagePump)

	self.messagePump:registerReciever(self)

	self.keys = Keys:new()
	self.keys:attachToPump(self.messagePump)
	self.keys:registerReciever(self)

	self.optionFile = OptionFile:new("game.cfg")

	self:checkVideoOptions()
	self:checkKeysOptions()
	self:checkInterfaceOptions()

	local g = self.optionFile:getGroup("video")
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
		self.background = self.renderer:add(GUI.Background:new(), GUIConst.Layers.Map)
		self.fpsText = self.renderer:add(GUI.Text:new(10, 10, "FPS: ", GUI.Fonts.basic, 255, 255, 255))		
		self.ftText = self.renderer:add(GUI.Text:new(10, 30, "Frame time: ", GUI.Fonts.basic, 255, 255, 255))		
	end)


	self.imageLoader = ImageLoader:new(self.cqueue, self.messagePump, self.renderer, self.optionFile:getGroup("interface").useFileCache, function()
		
		self.imageLoader = nil

		self:onLoadResource()

		-- C_Thread_PostMessage(self.hwnd, WindowsConst.WM.Destroy)

		self.world = World:new(self.cqueue, self.messagePump, Images, hwnd, self.renderer._ptr, self.optionFile)

		self.renderer:modify(function()
			self:openMainMenu()
		end)

	end)
	

end


function Game:checkKeysOptions()
	local g = self.optionFile:getGroup("keys")
	if g == nil then
		g = { }
	end

	local default = {
		up 		= #Key_W,
		down 	= #Key_S,
		left 	= #Key_A,
		right 	= #Key_D,
		map 	= #Key_M
	}

	for name, code in pairs(default) do
		if g[name] == nil then
			g[name] = code
		end
	end

	self.optionFile:setGroup("keys", g)
	self.optionFile:save()
end

function Game:checkInterfaceOptions()
	local g = self.optionFile:getGroup("interface")
	if g == nil then
		g = { }
	end

	local default = {
		centerCamera = false,
		useFileCache = true
	}

	for name, code in pairs(default) do
		if g[name] == nil then
			g[name] = code
		end
	end

	self.optionFile:setGroup("interface", g)
	self.optionFile:save()
end

function Game:checkVideoOptions()

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

end

function Game:keyPressed(key, alt)
	if key == #Key_F4 and alt then
		C_Thread_PostMessage(self.hwnd, WindowsConst.WM.Destroy)
	end
end

function Game:keyUnPressed(key, alt)
	if key == #Key_Esc then

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

			elseif self.interfaceMenu ~= nil then

				self.renderer:modify(function()
					self.interfaceMenu:del()
					self.menuIsOpened = false
					self.interfaceMenu = nil
				end)

			end

		end

	end
end

function Game:onLoadResource()

	GUI.CheckBox.init(C_Image_get("controls.png"))
	GUI.RadioBox.init(C_Image_get("controls.png"))
	
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

		-- interface button

		function()
			self.renderer:modify(function()
				self.mainMenu:del()
				self.menuIsOpened = false
				self.mainMenu = nil
				self:openInterfaceMenu()
			end)
		end,

		-- play  button

		function()
			self.renderer:modify(function()
				self.mainMenu:del()
				self.menuIsOpened = false
				self.mainMenu = nil
				-- self.renderer:del(self.background)				
			end)
			self.gameInProgress = true
			self.world:create(self.background._ptr)
		end,

		-- quit  button

		function()
			C_Thread_PostMessage(self.hwnd, WindowsConst.WM.Destroy)
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

function Game:openInterfaceMenu()

	self.menuIsOpened = true

	self.interfaceMenu = InterfaceMenuWindow:new(

		self.renderer,
		self.hwnd,
		self.optionFile,

		function()
			self.renderer:modify(function()
				self.interfaceMenu:del()
				self.menuIsOpened = false
				self.interfaceMenu = nil
				self:openMainMenu()
			end)
		end

	)

end


function Game:onRecreateSurface()

	C_Image_RestoreAll()

end

function Game:onDisableFullscreen()

	-- lprint("Game:onDisableFullscreen")

	local g = self.optionFile:getGroup("video")
	g.fullscreen = false
	self.optionFile:save()

	if self.videoMenu ~= nil then
		self.videoMenu:updateVideoOptions(g)
	end

end

function Game:onEnableFullscreen()

	-- lprint("Game:onEnableFullscreen")

	local g = self.optionFile:getGroup("video")
	g.fullscreen = true
	g.screenWidth = self.renderer:getCurrentWidth()
	g.screenHeight = self.renderer:getCurrentHeight()
	self.optionFile:save()

	if self.videoMenu ~= nil then
		self.videoMenu:updateVideoOptions(g)
	end

end

function Game:messageLoop()
	while not C_CQueue_empty(self.cqueue) do
		local message, lparam, wparam = C_CQueue_get(self.cqueue)
		self.messagePump:onWindowMessage(message, lparam, 0, 0, wparam)
	end
end

function Game:_onWindowMessage(message, lparam, lparam1, lparam2, wparam, w1)
	return self.messagePump:onWindowMessage(message, lparam, lparam1, lparam2, wparam, w1)
end

function Game:onQuit()
	lprint("Game:onQuit")
end

return Game

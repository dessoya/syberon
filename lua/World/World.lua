local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local GUIConst = require("GUI\\Const")
local MessagePump = require("MessagePump")
local Renderer = require("Renderer")
local Map = require("GUI\\Map")

local ImageGetter = require("World\\ImageGetter")

local Player = require("World\\Object\\Player")

local Object = require("Object")

local World = Object:extend()

function World:initialize(hwnd, renderer_ptr)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_Create]			= "onCreate",
		[WConst.WM.Size]			= "onWindowSize"
	})
	self.pump:registerReciever(self)
	self.renderer = Renderer:new(hwnd, nil, renderer_ptr)

end

function World:start()

	self.imageGetter = ImageGetter:new(self.hwnd,
		{ "ter1.png", "player-basic-idle.png", "player-basic-run.png" }, self.pump, function(images)
		lprint("ImageGetter done")
		self.imageGetter = nil
		self.images = images
		Player.init(images["player-basic-idle.png"], images["player-basic-run.png"])
	end)

	self.work = true
	while self.work do

		self:beforePeekMessage()

		while C_Thread_PeekMessage() do			
			local message = C_Thread_GetMessageId()
			lprint("message " .. message .. " " .. C_Thread_GetLParam() .. " " .. C_Thread_GetWParam())
			self.pump:onWindowMessage(message, C_Thread_GetLParam(), 0, 0, C_Thread_GetWParam())
		end

		C_Timer_Sleep(35)
	end
end

function World:beforePeekMessage()
end

function World:onQuit()
	self.work = false
end

function World:onCreate()

	lprint("World:onCreate")
	self.map = Map:new(80, 80)
	local t = self.images["ter1.png"]
	self.map:setupCellImage(0, t, 214, 280)

	local vw = math.floor(self.renderer:getCurrentViewWidth() / 80) + 2
	local vh = math.floor(self.renderer:getCurrentViewHeight() / 80) + 2

	self.map:setupViewSize(vw, vh)
	-- lprint("map:setupViewSize " .. vw .. "x" .. vh)

	self.renderer:modify(function()
		lprint("add map")
		self.renderer:add(self.map, GUIConst.Layers.Map)
	end)

	self.objects = { }

	local player = Player:new(200, 200)
	-- self:centerView(player)
	self:add(player)

end

function World:add(object)
	table.insert(self.objects, object)
	if self:checkForViewPort(object) then
		object:addToRenderer(self.renderer)
		self:sendToInterface(object)
	end
end

function World:checkForViewPort(object)
	return true
end

function World:sendToInterface(object)

end

function World:onWindowSize(w, d1, d2, h)

	if self.map ~= nil then
		local vw = math.floor(w / 80) + 2
		local vh = math.floor(h / 80) + 2

		self.map:setupViewSize(vw, vh)
		-- lprint("map:setupViewSize " .. vw .. "x" .. vh)
	end
end

return World

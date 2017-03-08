local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local GUIConst = require("GUI\\Const")
local MessagePump = require("MessagePump")
local Renderer = require("Renderer")
local Map = require("Map")
local RMap = require("GUI\\Map")

local ImageGetter = require("World\\ImageGetter")

local Player = require("World\\Object\\Player")

local Object = require("Object")

local World = Object:extend()

function World:initialize(hwnd, renderer_ptr)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_Create]			= "onCreate",
		[WConst.WM.Size]			= "onWindowSize",
		[Const.CMD_ThreadId]		= "onThreadId",
		[Const.CMD_UpdateObject]	= "onUpdateObject",
		[Const.CMD_Timer]			= "onTimer",
		[Const.CMD_UpdateOptions]	= "onUpdateOptions"		
	})
	self.pump:registerReciever(self)
	self.renderer = Renderer:new(hwnd, nil, renderer_ptr)

end

function World:onThreadId(lparam)
	lprint("World:onThreadId")
	local data = C_UnpackTable(lparam)
	dump(data)
	self.interfaceThreadId = data.interface
end

function World:onUpdateOptions(options, l1, l2, data)
	lprint("World:onUpdateOptions " .. options)
	local g = C_UnpackTable(data)
	if options == Const.Options.Interface then
		self.interface = g
		if self.player ~= nil then
			self.player:onInterfaceUpdate(self.interface)
		end
	end
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
			-- lprint("message " .. message .. " " .. C_Thread_GetLParam() .. " " .. C_Thread_GetWParam())
			self.pump:onWindowMessage(message, C_Thread_GetLParam(), 0, 0, C_Thread_GetWParam())
		end

		self:onTimer()
		C_Timer_Sleep(5)
	end
end

function World:beforePeekMessage()
end

function World:onQuit()
	self.work = false
end

function World:onCreate()

	lprint("World:onCreate")
	self.map = Map:new()
	self.map:set(0, 0, 1)
	self.map:set(1, 1, 1)
	self.map:set(3, 1, 1)
	
	self.rmap = RMap:new(self.map, 80, 80)
	local t = self.images["ter1.png"]
	-- self.map:setupCellImage(0, t, 214, 280)
	self.rmap:setupCellImage(1, t, 134, 120)

	local vw = math.floor(self.renderer:getCurrentViewWidth() / 80) + 2
	local vh = math.floor(self.renderer:getCurrentViewHeight() / 80) + 2

	self.rmap:setupViewSize(vw, vh, self.renderer:getCurrentViewWidth(), self.renderer:getCurrentViewHeight())
	-- lprint("map:setupViewSize " .. vw .. "x" .. vh)

	self.renderer:modify(function()
		lprint("add map")
		self.renderer:add(self.rmap, GUIConst.Layers.Map)
	end)

	self.objects = { }

	self.player = Player:new(200, 200, self.rmap, self.interface)
	-- self:centerView(player)
	self:add(self.player)

end

function World:onTimer()
	-- treat object
	if self.objects == nil then return end
	
	for id, object in pairs(self.objects) do
		object:treat()
	end
end

function World:add(object)

	-- table.insert(self.objects, object)
	self.objects[object.id] = object

	if self:checkForViewPort(object) then
		object:addToRenderer(self.renderer)
		self:sendToInterface(object)
	end
end

function World:onUpdateObject(lparam)
	local data = C_UnpackTable(lparam)
	lprint("World:onUpdateObject")
	dump(data)
	local id = data.id

	if self.objects[id] == nil then
		lprint("Error: World:onUpdateObject self.objects[id] == nil")
		return
	end

	local object = self.objects[id]
	data.id = nil

	-- treat to this moment
	object:treat()
	object:update(data)

end

function World:checkForViewPort(object)
	return true
end

function World:sendToInterface(object)	
	C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_AddObject, C_PackTable(object:serialize()))
end

function World:onWindowSize(w, d1, d2, h)

	if self.rmap ~= nil then
		local vw = math.floor(w / 80) + 2
		local vh = math.floor(h / 80) + 2

		self.rmap:setupViewSize(vw, vh, w, h)
		if self.player ~= nil then
			self.player:afterChangeCoords()
		end
		-- lprint("map:setupViewSize " .. vw .. "x" .. vh)
	end
end

return World
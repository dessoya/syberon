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
local scale = require("scale")

function World:initialize(hwnd, renderer_ptr)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_Create]			= "onCreate",
		[WConst.WM.Size]			= "onWindowSize",
		[Const.CMD_ThreadId]		= "onThreadId",
		[Const.CMD_UpdateObject]	= "onUpdateObject",
		[Const.CMD_Timer]			= "onTimer",
		[Const.CMD_UpdateOptions]	= "onUpdateOptions",
		[WConst.WM.MouseWheel]		= "onMouseWheel"		
	})
	self.pump:registerReciever(self)
	self.renderer = Renderer:new(hwnd, nil, renderer_ptr)

end

function World:onMouseWheel(d)
	if d < 0 then
		-- forward
		if self.scale ~= scale.count - 1 then
			self.scale = self.scale + 1
			self.rmap:setScale(self.scale)
			self.player:afterChangeScale(self.rmap)
		end
	else
		if self.scale ~= 0 then
			self.scale = self.scale - 1
			self.rmap:setScale(self.scale)
			self.player:afterChangeScale(self.rmap)
		end

	end
end

function World:onThreadId(lparam)
	-- lprint("World:onThreadId")
	local data = C_UnpackTable(lparam)
	-- dump(data)
	self.interfaceThreadId = data.interface
end

function World:onUpdateOptions(options, l1, l2, data)
	-- lprint("World:onUpdateOptions " .. options)
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
		{	"png_01\\Grass.png", "png_01\\Ground.png",
			"player-basic-idle.png", "player-basic-run.png" }, self.pump, function(images)
		-- lprint("ImageGetter done")
		self.imageGetter = nil
		self.images = images
		Player.init(self.images["player-basic-idle.png"], self.images["player-basic-run.png"])
	end)

	self.work = true
	while self.work do

		self:beforePeekMessage()

		while C_Thread_PeekMessage() do			
			local message = C_Thread_GetMessageId()
			--[message] lprint("message " .. message .. " " .. C_Thread_GetLParam() .. " " .. C_Thread_GetWParam())
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

function World:onCreate(p)

	local data = C_UnpackTable(p)
	
	lprint("World:onCreate")

	self.map = Map:new()

	-- self.rmap = RMap:new(self.map, scale.count, 10)

	--[[
	local t = self.images["ter1.png"]
	-- self.map:setupCellImage(0, t, 214, 280)
	self.rmap:setupCellImage(1, t, 134, 520) -- gross
	self.rmap:setupCellImage(2, t, 605, 516) -- pustinya
	self.rmap:setupCellImage(3, t, 743, 418) -- water
	self.rmap:setupCellImage(4, t, 328, 478) -- gori
	]]

	local cells = {
		{ 1, "png_01\\Grass.png" },
		{ 2, "png_01\\Ground.png" },
	}

	-- lprint("RMap:new")
	self.rmap = RMap:new(self.map, scale.count, table.getn(cells) + 1)

	for img = 1, table.getn(cells) do

		-- lprint("img " .. img)
		local cell = cells[img]
		-- lprint(cell[2] .. " " .. cell[1])
		local t = self.images[cell[2]]
		C_Image_disableDDS(t)
		local id = cell[1]
		
		for i = 1, scale.count do
			
			local s = scale.d[i]
			-- lprint("make scale " .. s[1] .. " " .. s[2])			
			local sc = i - 1
			if s[1] == -1 then
				self.rmap:setScaleInfo(sc, 200, 200)
				self.rmap:setupCellImage(sc, id, t, 0, 0)			
			else
				self.rmap:setScaleInfo(sc, math.floor(200 * s[1] / s[2]), math.floor(200 * s[1] / s[2]))
				local ts = C_Image_scale(t, s[1], s[2])
				self.rmap:setupCellImage(sc, id, ts, 0, 0)
			end
			-- lprint("done")

		end
	end

	self.scale = 2
	self.rmap:setScale(self.scale)

	-- lprint(1)
	self.rmap:setupViewSize(self.renderer:getCurrentViewWidth(), self.renderer:getCurrentViewHeight())
	-- lprint("map:setupViewSize " .. vw .. "x" .. vh)

	-- lprint("renderer:modify")
	self.renderer:modify(function()
		-- lprint("add map")
		self.renderer:add(self.rmap, GUIConst.Layers.Map)
		-- lprint("map added")
	end)

	lprint("generating map")
	for y = -1000,1000 do
		for x = -1000,1000 do
			self.map:set(x, y, 2)
		end
	end
	
	-- self.map:dot(0, 0, 5, 1)
	self.map:dot(4, 4, 5, 1)
	
	self.rmap:updateCells()

	self.objects = { }
	-- self.rmap:setCoords()

	self.player = Player:new(1024 * 1, 1024 * 1, self.rmap, self.interface)
	-- self:centerView(player)
	self:add(self.player)
	lprint("done")

	C_Renderer_del(self.renderer._ptr, data.backgroundPtr, GUIConst.Layers.Map)
end

function World:onTimer()
	-- treat object
	if self.objects == nil then return end
	
	for id, object in pairs(self.objects) do
		object:treat(self.rmap)
	end
end

function World:add(object)

	-- table.insert(self.objects, object)
	self.objects[object.id] = object

	if self:checkForViewPort(object) then
		object:addToRenderer(self.renderer, self.rmap)
		self:sendToInterface(object)
	end
end

function World:onUpdateObject(lparam)
	local data = C_UnpackTable(lparam)
	--lprint("World:onUpdateObject")
	--dump(data)
	local id = data.id

	if self.objects[id] == nil then
		--lprint("Error: World:onUpdateObject self.objects[id] == nil")
		return
	end

	local object = self.objects[id]
	data.id = nil

	-- treat to this moment
	object:treat(self.rmap)
	object:update(data, self.rmap)

end

function World:checkForViewPort(object)
	return true
end

function World:sendToInterface(object)	
	C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_AddObject, C_PackTable(object:serialize()))
end

function World:onWindowSize(w, d1, d2, h)

	if self.rmap ~= nil then

		--lprint(2)
		self.rmap:setupViewSize(w, h)
		if self.player ~= nil then
			self.player:afterChangeCoords()
		end
		--[size] lprint("map:setupViewSize " .. vw .. "x" .. vh)
	end
end

return World
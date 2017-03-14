local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local GUIConst = require("GUI\\Const")
local MessagePump = require("MessagePump")
local Renderer = require("Renderer")
local Map = require("Map")
local RMap = require("GUI\\Map")

local ImageGetter = require("World\\ImageGetter")

local Player = require("World\\Object\\Player")
local Stone  = require("World\\Object\\Stone")

local Object = require("Object")

local World = Object:extend()
local scale = require("scale")
local Random = require("Random")

local ConstScaleObject = 6

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

function World:setScale(scale)

	self.renderer:lockObjectList()

	self.scale = scale
	self.rmap:setScale(self.scale)
	self.scaleK = self.rmap:getScaleK()
	
	self.player:setScale(scale, self.scaleK)

	if scale > ConstScaleObject then
		for id, object in pairs(self.visibleObject) do
			if object.type ~= "player" then
				if object.added then
					self.visibleObject[object.id] = nil
					object:delFromRenderer(self.renderer)
					-- self:sendToInterface(object)
				end
			end
		end
	else

		local rmap = self.rmap
		local scaleK = self.scaleK
		for id, object in pairs(self.visibleObject) do
			if object.type ~= "player" then
				if object.added then
					object:setScale(scale, scaleK)
					object:reposition(rmap, scale, scaleK)

					if not self:checkForViewPort(object) then
						self.visibleObject[object.id] = nil
						object:delFromRenderer(self.renderer)
						-- self:sendToInterface(object)
					end
				end
			end
		end


		-- check for new

		local y1 = rmap.y - 32768
		local y2 = rmap.y + (rmap.vh * scaleK) + 32768
		local x1 = rmap.x - 32768
		local x2 = rmap.x + (rmap.vw * scaleK) + 32768
		local map = self.map
		local vo = self.visibleObject
		local o = self.objects
		
		while y1 < y2 do

			local cx = x1
			while cx < x2 do
				local ids = map:getIDs(cx, y1)

				for ind, id in pairs(ids) do
					if vo[id] == nil then
						local object = o[id]
						if object.type ~= "player" then
							object:setScale(scale, scaleK)
							if self:checkForViewPort(object) then
								self.visibleObject[object.id] = object
								object:addToRenderer(self.renderer, self.rmap)
								self:sendToInterface(object)
							end
						end
					end
				end

				cx = cx + 32768
			end

			y1 = y1 + 32768
		end
	end

	self.renderer:unlockObjectList()
end

function World:onMouseWheel(d)
	if d < 0 then
		-- forward
		if self.scale ~= scale.count - 1 then
			self:setScale(self.scale + 1)
		end
	else
		if self.scale ~= 0 then
			self:setScale(self.scale - 1)
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
		{	"png_01\\Grass.png", "png_01\\Ground.png", "png_02\\Water_01.png", "png_02\\Mountain_01.png",
			"png_01\\Stone_03_grass.png",
			"player-basic-idle.png", "player-basic-run.png" }, self.pump, function(images)
		-- lprint("ImageGetter done")
		self.imageGetter = nil
		self.images = images
		Player.init(images["player-basic-idle.png"], images["player-basic-run.png"])
		Stone.init(images["png_01\\Stone_03_grass.png"])
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

	local cells = {
		{ 1, "png_01\\Grass.png" },
		{ 2, "png_01\\Ground.png" },
		{ 3, "png_02\\Water_01.png" },
		{ 4, "png_02\\Mountain_01.png" }
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
				self.rmap:setScaleInfo(sc, _tointeger(200 * s[1] / s[2]), _tointeger(200 * s[1] / s[2]))
				local ts = C_Image_scale(t, s[1], s[2])
				self.rmap:setupCellImage(sc, id, ts, 0, 0)
			end
			-- lprint("done")

		end
	end

	self.scale = 2
	self.rmap:setScale(self.scale)
	self.scaleK = self.rmap:getScaleK()

	self.rmap:setupViewSize(self.renderer:getCurrentViewWidth(), self.renderer:getCurrentViewHeight())

	self.renderer:modify(function()
		self.renderer:add(self.rmap, GUIConst.Layers.Map)
	end)

	lprint("generating map")

	local r = Random:new(1)

	local s, c = 15, 30
	local h = s * (c + 1) / 2
	for y = 1, c do		
		for x = 1, c do
			local xo = r:get(10) - 5
			local yo = r:get(10) - 5
			self.map:makeGrass(r, x * s - h + xo, y * s - h + yo, 1)
		end
	end

	local make = function(func, s, c, co, bx, by, id, step)

		local y1 = s / 2
		local yd = 1
		local h = (c / 2) + 1
		local hco = co / 2
		local hstep = step / 2
		for y = 1, c do

			local x1 = s / 2
			local xd = 1

			if y == h then
				yd = -1
				y1 = s / 2
			end

			for x = 1, c do

				if x == h then
					xd = -1
					x1 = s / 2
				end

				x1 = x1 + ( r:get(co) - hco )
				y1 = y1 + ( r:get(co) - hco )

				self.map[func](self.map, r, bx + (x1 * xd), by + (y1 * yd), id)
				x1 = x1 + s + r:get(step) - hstep
			end
			y1 = y1 + s + r:get(step) - hstep
		end

	end

	make("makeRiver", 180, 4, 88, 130, 0, 3, 56)
	make("makeMountain", 128, 6, 66, 50, 70, 4, 76)
	
	-- self.map:set(1, 1, 1)
	self.rmap:updateCells()

	self.objects = { }
	self.visibleObject = { }
	-- self.rmap:setCoords()


	self.player = Player:new(1024 * 1, 1024 * 1, self.rmap, self.interface,
		function(player)

			local rmap = self.rmap
			local scale = self.scale
			local scaleK = self.scaleK

			if scale <= ConstScaleObject then


			self.renderer:lockObjectList()

			for id, object in pairs(self.visibleObject) do
				if object.type ~= "player" then
					if object.added then
						object:reposition(rmap, scale, scaleK)
						if not self:checkForViewPort(object) then
							self.visibleObject[object.id] = nil
							object:delFromRenderer(self.renderer)
						end
					end
				end
			end

			-- check for new

			local y1 = rmap.y - 32768
			local y2 = rmap.y + (rmap.vh * scaleK) + 32768
			local x1 = rmap.x - 32768
			local x2 = rmap.x + (rmap.vw * scaleK) + 32768
			local map = self.map
			local vo = self.visibleObject
			local o = self.objects
			
			while y1 < y2 do
	
				local cx = x1
				while cx < x2 do
					local ids = map:getIDs(cx, y1)

					for ind, id in pairs(ids) do
						if vo[id] == nil then
							local object = o[id]
							if object.type ~= "player" then
								object:setScale(scale, scaleK)
								if self:checkForViewPort(object) then
									self.visibleObject[object.id] = object
									object:addToRenderer(self.renderer, self.rmap)
									self:sendToInterface(object)
								end
							end
						end
					end

					cx = cx + 32768
				end

				y1 = y1 + 32768
			end


			self.renderer:unlockObjectList()
			end

		end
	)
	-- self:centerView(player)
	self:add(self.player)


	-- make stones

	-- local s = Stone:new(1024 * 2, 1024 * 2)
	-- self:add(s)

	local s, c = 15, 30
	local h = s * (c + 1) / 2
	local map = self.map

	for y = -1 * h, h do		
		for x = -1 * h, h do
			local x1 = _tointeger(x)
			local y1 = _tointeger(y)
			local id = map:get(x1, y1)
			local add = false
			if id == 1 then -- grass
				if r:get(100) > 95 then
					add = true
				end
			elseif id == 2 then -- ground
				if r:get(100) > 90 then
					add = true
				end
			end
			if add then
				local s = Stone:new((1024 * x1) + r:get(1024), (1024 * y1) + r:get(1024))
				self:add(s)
			end
		end
	end

	lprint("done")

	self.renderer:modify(function()
		C_Renderer_del(self.renderer._ptr, data.backgroundPtr, GUIConst.Layers.Map)
	end)
end

function World:onTimer()
	-- treat object
	if self.objects == nil then return end
	
	for id, object in pairs(self.objects) do
		object:treat(self.rmap, self.renderer)
	end
end

function World:add(object)

	-- table.insert(self.objects, object)
	self.objects[object.id] = object

	local x, y, sz = self.map:addID(object.x, object.y, object.id)
	-- lprint(sz) 32768
	object.cx = x
	object.cy = y

	object:setScale(self.scale, self.scaleK)
	if self:checkForViewPort(object) then
		self.visibleObject[object.id] = object
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
	self.renderer:lockObjectList()
	object:treat(self.rmap, self.renderer)
	object:update(data, self.rmap, self.renderer)
	self.renderer:unlockObjectList()

end

function World:checkForViewPort(object)

	local rmap = self.rmap
	local k = rmap:getScaleK()

	-- lprint("" .. object.w_2 .. " " .. object.h_2)
	if object.x + object.w_2 * k < rmap.x or object.y + object.h_2 * k < rmap.y or
		rmap.vw * k < object.x - rmap.x - object.w_2 * k or
		rmap.vh * k < object.y - rmap.y - object.h_2 * k then

		return false
	end


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
			self.player.afterChangePosition_cb(self.player)
		end
		--[size] lprint("map:setupViewSize " .. vw .. "x" .. vh)
	end
end

return World

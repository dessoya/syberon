C_InstallModule("ds_array")

#include World\Const

local GUI = require("GUI")
local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local GUIConst = require("GUI\\Const")
local MessagePump = require("MessagePump")
local Renderer = require("Renderer")
local Map = require("Map")
local RMap = require("GUI\\Map")

local Player = require("World\\Object\\Player")
local Stone  = require("World\\Object\\Stone")
local RespawnCrystal = require("World\\Object\\RespawnCrystal")

local Object = require("Object")

local ThreadChild = require("Thread\\Child")
local World = ThreadChild:extend()
local scale = require("scale")
local Random = require("Random")
local ThreadPool = require("Thread\\Pool")

local ConstScaleObject = 4
local startScale = 0

function World:init(hwnd, renderer_ptr)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_Create]			= "onCreate",
		[WConst.WM.Size]			= "onWindowSize",
		[Const.CMD_Queue]			= "onQueue",
		[Const.CMD_UpdateObject]	= "onUpdateObject",
		[Const.CMD_Timer]			= "onTimer",
		[Const.CMD_UpdateOptions]	= "onUpdateOptions",
		[WConst.WM.MouseWheel]		= "onMouseWheel",

		[Const.CMD_MiniMap_Opened]	= "onMiniMapOpened",
		[Const.CMD_MiniMap_Closed]	= "onMiniMapClosed",

		[Const.MG_ForestDone]		= "onForestDone",
		[Const.MG_FigureDone]		= "onFigureDone",

		-- [Const.CMD_Data]			= "onData"

	})
	self.pump:registerReciever(self)
	self.renderer = Renderer:new(hwnd, nil, renderer_ptr)
	self.miniMap = false
end


--[[
function World:onData(a)
	lprint("World:onData")
	lprint(a:len())
	lprint(a:size())
	local t = { a:get(0, a:len()) }
	lprint("t:len " .. table.getn(t))
	dump(t)
	-- a:a("zxc")
	-- self:send(Const.CMD_Data, a)
end
]]

function World:onMiniMapOpened()
	self.miniMap = true
end

function World:onMiniMapClosed()
	self.miniMap = false
end

function World:start()

	C_Thread_SetName("World")

	Player.init()
	Stone.init()
	RespawnCrystal.init()

	self.work = true
	while self.work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
				-- lprint(messageId)

				--[message] lprint("message " .. message .. " " .. C_Thread_GetLParam() .. " " .. C_Thread_GetWParam())
				self.pump:onWindowMessage(messageId, a1, 0, 0, a2)
			end
		end

		self:onTimer()
		C_Timer_Sleep(3)
	end
end


function onPlayerChangePosition(player, world)

	C_CQueue_send(world.interfaceQueue, Const.CMD_PlayerCoords, player.x, player.y)

	world:updateInterfaceObjectPosition(player)
	world:updatePlayerVision()

	local rmap = world.rmap
	local scale = world.scale
	local scaleK = world.scaleK

	if scale <= ConstScaleObject then

		world.renderer:lockObjectList()

		for id, object in pairs(world.visibleObject) do
			if object.type ~= "player" then
				if object.added then
					object:reposition(rmap)
					if not world:checkForViewPort(object) then
						world.visibleObject[object.id] = nil
						object:delFromRenderer(world.renderer)
						world:deleteFromInterface(object)
					else
						world:updateInterfaceObjectPosition(object)
					end
				end
			end
		end

		-- check for new

		local y1 = rmap.y - 32768
		local y2 = rmap.y + (rmap.vh * scaleK) + 32768
		local x1 = rmap.x - 32768
		local x2 = rmap.x + (rmap.vw * scaleK) + 32768
		local map = world.map
		local vo = world.visibleObject
		local o = world.objects
		local renderer = world.renderer
		
		while y1 < y2 do

			local cx = x1
			while cx < x2 do
				local ids = map:getIDs(cx, y1)

				for ind, id in pairs(ids) do
					if vo[id] == nil then
						local object = o[id]
						if object.type ~= "player" then
							object:setScale()
							if world:checkForViewPort(object) then
								vo[object.id] = object
								object:addToRenderer(renderer, rmap)
								world:sendToInterface(object)
							end
						end
					end
				end

				cx = cx + 32768
			end

			y1 = y1 + 32768
		end


		world.renderer:unlockObjectList()
	end

end

function World:removeVisibleObjects()

	local vo = self.visibleObject

	for id, object in pairs(vo) do
		if object.type ~= "player" then
			if object.added then
				vo[object.id] = nil
				object:delFromRenderer(self.renderer)
				self:deleteFromInterface(object)
			end
		end
	end

end

function World:objectsSetScale()
	Stone._setScale(self.scale, self.scaleK)
	Player._setScale(self.scale, self.scaleK)
	RespawnCrystal._setScale(self.scale, self.scaleK)
end

function World:setScale(scale)

	C_CQueue_send(self.interfaceQueue, Const.CMD_SetScale, scale + 1)
	-- C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_SetScale, scale + 1)

	self.renderer:lockObjectList()

	self.scale = scale
	self.rmap:setScale(self.scale)
	self.scaleK = self.rmap:getScaleK()

	self:objectsSetScale()	
	self.player:setScale()

	self:updateInterfaceObjectPosition(self.player)

	if scale > ConstScaleObject then

		-- remove objects
		self:removeVisibleObjects()

	else

		-- check visible objects 

		local rmap = self.rmap
		local vo = self.visibleObject

		for id, object in pairs(vo) do
			if object.type ~= "player" then
				if object.added then
					object:setScale()
					object:reposition(rmap)

					if not self:checkForViewPort(object) then
						self.visibleObject[object.id] = nil
						object:delFromRenderer(self.renderer)
						self:deleteFromInterface(object)
					else
						self:updateInterfaceObjectPosition(object)
					end
				end
			end
		end


		-- check for new

		local y1 = rmap.y - 32768
		local y2 = rmap.y + (rmap.vh * self.scaleK) + 32768
		local x1 = rmap.x - 32768
		local x2 = rmap.x + (rmap.vw * self.scaleK) + 32768
		local map = self.map
		local o = self.objects
		local renderer = self.renderer
		
		while y1 < y2 do

			local cx = x1
			while cx < x2 do
				local ids = map:getIDs(cx, y1)

				for ind, id in pairs(ids) do
					if vo[id] == nil then
						local object = o[id]
						if object.type ~= "player" then
							object:setScale()
							if self:checkForViewPort(object) then
								vo[object.id] = object
								object:addToRenderer(renderer, rmap)
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

	if self.player == nil then return end
	if self.miniMap then return end

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

function World:onQueue(queueId, l1, l2, queue)
	lprint("World:onQueue")
	-- local data = C_UnpackTable(lparam)
	-- dump(data)
	self.interfaceQueue = queue
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


function World:onQuit()
	self.work = false
end

function World:makeRMap()

	local cells = {
		{ 1, "png_01\\Grass_02.png" },
		{ 2, "png_01\\Ground.png" },
		{ 3, "png_02\\Water_01.png" },
		{ 4, "png_02\\Mountain_01.png" }
	}

	self.rmap = RMap:new(self.playerMap, scale.count, table.getn(cells) + 1)

	for img = 1, table.getn(cells) do

		local cell = cells[img]
		local id = cell[1]
		
		for i = 1, scale.count do
			
			local s = scale.d[i]
			local sc = i - 1
			self.rmap:setScaleInfo(sc, _tointeger(200 * s[1] / s[2]), _tointeger(200 * s[1] / s[2]))
			local ts = C_Image_get(cell[2] .. "_" .. i)
			self.rmap:setupCellImage(sc, id, ts, 0, 0)

		end
	end
end

function World:generateMap(cnt_forest, cnt_river, cnt_mountain)

	self.cnt_river = cnt_river
	self.cnt_mountain = cnt_mountain
	self.cnt_forest = cnt_forest
	lprint("generating map")
	self.r = Random:new(1)
	local r = self.r

	self.mapPool = ThreadPool:new("World\\MapGenerator", self.childQueue, 8, self.map._ptr, self.rcList)

	self.progressBar:setCaption("Generating map - forest")

	self.workDone = 0
	self.workNeed = 0

	self.iter = 0
	self.iterM = cnt_forest + 1
	local s, c = 15, cnt_forest
	local h = s * (c + 1) / 2
	for y = 1, c do		
		for x = 1, c do
			local xo = r:get(10) - 5
			local yo = r:get(10) - 5

			self.workNeed = self.workNeed + 1

			-- self.map:makeGrass(r, x * s - h + xo, y * s - h + yo, 1)
			self.mapPool:send(Const.MG_Forest, r.seed, x * s - h + xo, y * s - h + yo, 1)			
		end
		-- self.progressBar:inc()
	end

end

function World:onForestDone()

	self.workDone = self.workDone + 1

	self.iter = self.iter + 1
	if self.iter == self.iterM then
		self.iter = 0
		self.progressBar:inc()
	end

	if self.workDone == self.workNeed then
		-- self:onMapGenerateDone()

		local rc = RespawnCrystal:new((1024 * 0), (1024 * -20))
		self:add(rc)
		rc = RespawnCrystal:new((1024 * 20), (1024 * 20))
		self:add(rc)
		 rc = RespawnCrystal:new((1024 * -20), (1024 * 20))
		self:add(rc)

		self.progressBar:setCaption("Generating map - river")
		self:makeFigure("makeRiver", 180, self.cnt_river, 88, 130, 0, 3, 56)
		-- self:makeFigure("makeRiver", 60, self.cnt_river, 33, 66, 0, 3, 56)

	end

end

function World:onFigureDone(name)

	self.workDone = self.workDone + 1

	self.iter = self.iter + 1
	if self.iter == self.iterM then
		self.iter = 0
		self.progressBar:inc()
	end

	if self.workDone == self.workNeed then
		if name == "makeRiver" then
			self.progressBar:setCaption("Generating map - mountain")
			self:makeFigure("makeMountain", 128, self.cnt_mountain, 66, 50, 70, 4, 76)
			-- self:makeFigure("makeMountain", 66, self.cnt_mountain, 27, 12, 70, 4, 76)
		elseif name == "makeMountain" then

			self.mapPool:sendAll(Const.CMD_Quit)


			local i, l = 0, self.rcList:len()
			while i < l do

				local rc = self.rcList:get(i)
				local rx, ry = rc:get(#ODP_X, 2) 
				self.map:makeCircle(rx / 1024, ry / 1024, 1, 1, 16, function(map, x, y)
					map:setIfEmptyAndId(x, y, 2, 3)
					map:setIfEmptyAndId(x, y, 2, 4)
				end)

				i = i + 1
			end

			self.progressBar:setCaption("Generating map - stones")

			local r = self.r
			local s, c = 15, self.cnt_forest
			local h = s * (c + 1) / 2
			local map = self.map
			local inc = 0

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

				inc = inc + 1
				if inc == c then
					self.progressBar:inc()
					inc = 0
				end
			end



			self:onMapGenerateDone()
		end
--[[
		self.progressBar:setCaption("Generating map - river")
		self:makeFigure("makeRiver", 180, self.cnt_river, 88, 130, 0, 3, 56)
]]
	end

end




function World:makeFigure(func, s, c, co, bx, by, id, step)

	self.workDone = 0
	self.workNeed = 0

	local r = self.r

	self.iter = 0
	self.iterM = c + 1

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

			-- self.map[func](self.map, r, bx + (x1 * xd), by + (y1 * yd), id)
			self.workNeed = self.workNeed + 1
			self.mapPool:send(Const.MG_Figure, func, r.seed, bx + (x1 * xd), by + (y1 * yd), id)

			x1 = x1 + s + r:get(step) - hstep

			-- self.progressBar:inc()
		end
		y1 = y1 + s + r:get(step) - hstep
	end

end

function World:onCreate(p)

	self.rcList = Array:new()

	local cnt_forest = 30
	local cnt_river = 4
	local cnt_mountain = 5

	local count = cnt_forest + (cnt_river + 1 ) * (cnt_river + 1) + (cnt_mountain + 1) * (cnt_mountain + 1) + cnt_forest

	self.renderer:modify(function()
		self.progressBar = self.renderer:add(GUI.ProgressBar:new("Generating map", count, true))
		self.progressBar:onWindowSize(self.renderer:getCurrentViewWidth(), self.renderer:getCurrentViewHeight())
	end)

	self.gameData = C_UnpackTable(p)
	
	lprint("World:onCreate")

	self.map = Map:new()
	self.playerMap = Map:new()

	self:makeRMap()

	self.scale = startScale
	self.rmap:setScale(self.scale)
	self.scaleK = self.rmap:getScaleK()

	C_CQueue_send(self.interfaceQueue, Const.CMD_SetScale, self.scale + 1)
	-- C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_SetScale, self.scale + 1)

	self:objectsSetScale()

	self.rmap:setupViewSize(self.renderer:getCurrentViewWidth(), self.renderer:getCurrentViewHeight())

	self.objects = { }
	self.visibleObject = { }

	self:generateMap(cnt_forest, cnt_river, cnt_mountain)
end

function World:updatePlayerVision()
	local x, y = _tointeger(self.player.x / 1024), _tointeger(self.player.y / 1024)
	if self.player.vx ~= x or self.player.vy ~= y then

		if self.player.vx ~= nil then

			self.map:makeCircle(self.player.vx, self.player.vy, 1, 1, 10, function(map, x, y)
				self.playerMap:setFlags(x, y, 1)
			end)

		end

		self.player.vx = x
		self.player.vy = y

		self.map:makeCircle(x, y, 1, 1, 10, function(map, x, y)
			self.playerMap:set(x, y, map:get(x, y))
		end)
		self.rmap:updateCells()
	end
end

function World:onMapGenerateDone()

	self.player = Player:new(1024 * 1, 1024 * 1, self.rmap, self.renderer, self.interface, onPlayerChangePosition, self)

	self:updatePlayerVision()	
	self:add(self.player)

	onPlayerChangePosition(self.player, self)

	self.renderer:modify(function()
		self.renderer:del(self.progressBar)			
		self.renderer:add(self.rmap, GUIConst.Layers.Map)
		C_Renderer_del(self.renderer._ptr, self.gameData.backgroundPtr, GUIConst.Layers.Map)

		C_CQueue_send(self.interfaceQueue, Const.CMD_Create, self.playerMap._ptr, self.rcList)
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

	if object.a then
		object.a:set(#ODP_X, object.x, object.y, object.cx, object.cy)		
	end

	
	if object.type == #OT_RESPAWN_CRYSTAL then
		self.rcList:push(object.a)

		local x, y = object.x / 1024, object.y / 1024
		self.map:makeCircle(x, y, 1, 1, 30, function(map, x, y)
			self.playerMap:set(x, y, map:get(x, y))
			self.playerMap:setFlags(x, y, 1)
		end)

		-- lprint("rcList len " .. self.rcList:len())
	end

	object:setScale(self.scale, self.scaleK)

	if self:checkForViewPort(object) then
		self.visibleObject[object.id] = object
		object:addToRenderer(self.renderer, self.rmap)
		self:sendToInterface(object)
	end
end

function World:onUpdateObject(lparam)
	local data = C_UnpackTable(lparam)
	-- lprint("World:onUpdateObject")
	-- dump(data)
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

	local x, y = object.x / 1024, object.y / 1024
	if object.x < 0 and object.x % 1024 ~= 0 then x = x - 1 end
	if object.y < 0 and object.y % 1024 ~= 0 then y = y - 1 end

	local id, flags = self.playerMap:get(x, y)
	if flags == 1 or id == 0 or id == 0xfff then
		return false
	end

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
	C_CQueue_send(self.interfaceQueue, Const.CMD_AddObject, C_PackTable(object:serialize()))
	-- C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_AddObject, C_PackTable(object:serialize()))
end

function World:deleteFromInterface(object)	
	C_CQueue_send(self.interfaceQueue, Const.CMD_DelObject, object.id)
	-- C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_DelObject, object.id)
end

function World:updateInterfaceObjectPosition(object)	

	if object.rx ~= nil then
		if object.rx ~= object.srx or object.ry ~= object.sry then
			object.srx = object.rx
			object.sry = object.ry
			C_CQueue_send(self.interfaceQueue, Const.CMD_UpdateObject, C_PackTable({ id = object.id, x = object.rx, y = object.ry }))
			-- C_Thread_PostMessage(self.interfaceThreadId, Const.CMD_UpdateObject, C_PackTable({ id = object.id, x = object.rx, y = object.ry }))
		end
	end
end


function World:onWindowSize(w, d1, d2, h)

	if self.rmap ~= nil then

		--lprint(2)
		self.rmap:setupViewSize(w, h)
		if self.player ~= nil then
			self.player:afterChangeCoords()
			self.player.afterChangePosition_cb(self.player, self)
		end
		--[size] lprint("map:setupViewSize " .. vw .. "x" .. vh)
	end
end

return World

C_InstallModule("timer")

local Const = require("World\\Const")
local WConst = require("Windows\\Const")
local GUIConst = require("GUI\\Const")

local MessagePump = require("MessagePump")
local Keys = require("Keys")

local Object = require("Object")
local IObject = require("World\\Interface\\Object")

local Player = require("World\\Interface\\Player")
local Stone = require("World\\Interface\\Stone")

local ThreadChild = require("Thread\\Child")
local Interface = ThreadChild:extend()

local Renderer = require("Renderer")
local MiniMap = require("GUI\\MiniMap")

local Map = require("Map")

function Interface:init(hwnd, renderer_ptr)

	IObject.hwnd = hwnd
	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		-- [WConst.WM.Size]			= "onWindowSize",

		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_AddObject]		= "onAddObject",
		[Const.CMD_DelObject]		= "onDelObject",
		[Const.CMD_UpdateObject]	= "onUpdateObject",
		[Const.CMD_SetScale]		= "onSetScale",
		[WConst.WM.MouseMove]		= "onMouseMove",
		[Const.CMD_RescaleDone]		= "onRescaleDone",

		[Const.CMD_Create]			= "onCreate",
		[Const.CMD_PlayerCoords]	= "onPlayerCoords",


		[Const.CMD_Keys]			= "onKeys",
		[Const.CMD_Queue]			= "onQueue",
		[Const.CMD_UpdateOptions]	= "onUpdateOptions"
	})
	self.pump:registerReciever(self)

	self.controlKeys = { }
	self.keys = Keys:new()
	self.keys:attachToPump(self.pump)
	self.keys:registerReciever(self)

	self.objects = { }
	self.isStart = false

	self.renderer = Renderer:new(hwnd, self.pump, renderer_ptr)	

end

function Interface:onCreate(playerMap_ptr, rcList)
	self.isStart = true
	self.playerMap = Map:new(playerMap_ptr)
	self.rcList = rcList
end

function Interface:onRescaleDone(scale, index)
	if self.miniMap ~= nil then
		self.miniMap:rescaleDone(scale, index)
	end
end

function Interface:onMouseMove(lparam, x, y)

	local ctx = { }
	for id, object in pairs(self.objects) do
		if object:pointOnObject(x, y) then
			ctx.hover = true
			if not object.hover then
				object.hover = true
				object:onHover()
			end
		else
			if object.hover then
				ctx.hoverLost = true
				object.hover = false
				object:onHoverLost()
			end
		end
	end

	if ctx.hover then self:changeCursor(WConst.Cursor.Hand) 
	elseif ctx.hoverLost then self:changeCursor(WConst.Cursor.Arrow) end

end

function Interface:onPlayerCoords(x, y)
	self.playerX = x
	self.playerY = y
end

function Interface:changeCursor(cursor)
	if self.cursor ~= cursor then
		self.cursor = cursor
		C_Thread_PostMessage(IObject.hwnd, Const.CMD_SetCursor, cursor)	
	end
end

function Interface:onUpdateOptions(options, data)
	--lprint("Interface:onUpdateOptions " .. options)
	local g = C_UnpackTable(data)
	if options == Const.Options.Interface then
		self.interface = g
	end
end

function Interface:onQueue(queueId, queue)
	--lprint("Interface:onThreadId")
	-- local data = C_UnpackTable(lparam)
	--dump(data)
	self.worldQueue = queue
	IObject.init(queue)

end


function Interface:onKeys(lparam)
	self.controlKeys = C_UnpackTable(lparam)
	--lprint("Interface:onKeys")
	--dump(self.controlKeys)
end

function Interface:keyPressed(key, l1, l2, alt)
	-- lprint("Interface:keyPressed " .. key .. " " .. bn[alt])

	if self.player ~= nil and self.miniMap == nil then
		self.player:makeDirFromKeys(key, self.controlKeys, self.keys.keyStates)
	end

	if self.isStart then
		if key == self.controlKeys.map then

			if self.miniMap == nil then
				-- open mini map

				C_CQueue_send(self.worldQueue, Const.CMD_MiniMap_Opened)

				self.miniMap = MiniMap:new(
					self.renderer,
					self.playerMap,
					self.playerX,
					self.playerY,
					self.childQueue,
					self.rcList					
				)

				self.renderer:modify(function()
					self.renderer:add(self.miniMap, GUIConst.Layers.Iterface)
				end)
			else

				self.renderer:modify(function()
					self.renderer:del(self.miniMap)
				end)
				C_CQueue_send(self.worldQueue, Const.CMD_MiniMap_Closed)

				self.miniMap = nil

			end
			
		end
	end

end

function Interface:keyUnPressed(key, l1, l2, alt)
	-- lprint("Interface:keyUnPressed " .. key .. " " .. bn[alt])

	if self.player ~= nil and self.miniMap == nil then
		self.player:makeDirFromKeys(key, self.controlKeys, self.keys.keyStates)
	end
end

function Interface:start()

	C_Thread_SetName("Interface")

	self.work = true
	while self.work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2, a3, a4 = self:get()
				-- lprint("message " .. messageId)

				--[message] lprint("message " .. message .. " " .. C_Thread_GetLParam() .. " " .. C_Thread_GetWParam())
				self.pump:onWindowMessage(messageId, a1, a2, a3, a4)
			end
		end

		C_Timer_Sleep(23)
	end

end

function Interface:onQuit()
	self.work = false
end

function Interface:onSetScale(scale)
	self.scale = scale
	IObject.setScale(scale)
end


function Interface:onUpdateObject(lparam)
	local data = C_UnpackTable(lparam)
	-- lprint("Interface:onUpdateObject")
	-- dump(data)
	local object = self.objects[data.id]
	if object ~= nil then
		data.id = nil
		for k,v in pairs(data) do
			object[k] = v
		end
	end
end

function Interface:onDelObject(id)
	-- lprint("del object " .. id)
	self.objects[id] = nil
end

function Interface:onAddObject(lparam)
	local data = C_UnpackTable(lparam)
	-- lprint("Interface:onAddObject")
	-- dump(data)
	local object = IObject.make(data)
	if object ~= nil then
		-- lprint("------")
		-- dump(object)
		if object.type == "player" then
			self.player = object
			--lprint("got player object")
		end
		self.objects[object.id] = object
	else
		-- eprint("error can't make IObject")
		lprint("add unknown object")
		dump(data)
	end

end

--[[
function Interface:onWindowSize(l,w,h)
	lprint("Interface:onWindowSize " .. w .. " " .. h)
end
]]

return Interface

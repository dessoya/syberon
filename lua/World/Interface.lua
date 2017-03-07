local Const = require("World\\Const")
local MessagePump = require("MessagePump")
local Keys = require("Keys")

local Object = require("Object")
local IObject = require("World\\Interface\\Object")

local Player = require("World\\Interface\\Player")

local Interface = Object:extend()

function Interface:initialize(hwnd)

	self.hwnd = hwnd
	self.pump = MessagePump:new({		
		[Const.CMD_Quit]			= "onQuit",
		[Const.CMD_AddObject]		= "onAddObject",
		[Const.CMD_Keys]			= "onKeys",
		[Const.CMD_ThreadId]		= "onThreadId"		
	})
	self.pump:registerReciever(self)

	self.controlKeys = { }
	self.keys = Keys:new()
	self.keys:attachToPump(self.pump)
	self.keys:registerReciever(self)

	self.objects = { }

end

function Interface:onThreadId(lparam)
	lprint("World:onThreadId")
	local data = C_UnpackTable(lparam)
	dump(data)
	self.worldThreadId = data.world
	IObject.init(self.worldThreadId)

end


function Interface:onKeys(lparam)
	self.controlKeys = C_UnpackTable(lparam)
	lprint("Interface:onKeys")
	dump(self.controlKeys)
end

function Interface:keyPressed(key, alt)
	-- lprint("Interface:keyPressed " .. key .. " " .. bn[alt])

	if self.player ~= nil then
		self.player:makeDirFromKeys(key, self.controlKeys, self.keys.keyStates)
	end

end

function Interface:keyUnPressed(key, alt)
	-- lprint("Interface:keyUnPressed " .. key .. " " .. bn[alt])

	if self.player ~= nil then
		self.player:makeDirFromKeys(key, self.controlKeys, self.keys.keyStates)
	end
end

function Interface:start()

	self.work = true
	while self.work do

		self:beforePeekMessage()

		while C_Thread_GetMessage() do			
			local message = C_Thread_GetMessageId()
			self.pump:onWindowMessage(C_Thread_GetMessageId(), C_Thread_GetLParam(), 0, 0, C_Thread_GetWParam())	
		end

		-- C_Timer_Sleep(35)
	end
end

function Interface:beforePeekMessage()
end

function Interface:onQuit()
	self.work = false
end

function Interface:onAddObject(lparam)
	local data = C_UnpackTable(lparam)
	dump(data)
	local object = IObject.make(data)
	if object ~= nil then
		if object.type == "player" then
			self.player = object
			lprint("got player object")
		end
		table.insert(self.objects, object)
	else
		lprint("error can't make IObject")
		dump(data)
	end

end

return Interface

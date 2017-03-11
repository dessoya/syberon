local Object = require("Object")
local WindowsConst = require("Windows\\Const")


local MessagePump = Object:extend()

function MessagePump:initialize(messageNames)
	if messageNames ~= nil then
		self.messageNames = messageNames
	else
		self.messageNames = { }
	end

	self.messageNames.onWindowMessage = "onWindowMessage"
	self.messageMap = { }
end

function MessagePump:addNames(messageNames)
	for message, messageName in pairs(messageNames) do
		self.messageNames[message] = messageName
	end
end

function MessagePump:onWindowMessage(message, lparam, lparam1, lparam2, wparam, w1)

	local flag = false
	-- lprint("message " .. message)

	if self.messageMap[message] ~= nil then

		local recievers = self.messageMap[message]
		local method = self.messageNames[message]
		-- lprint("method " .. method)
		for i, r in ipairs(recievers) do
			if r[method](r, lparam, lparam1, lparam2, wparam, w1) then
				flag = true
			end
		end

	end

	if self.messageMap.onWindowMessage ~= nil then

		local recievers = self.messageMap.onWindowMessage
		for i, r in ipairs(recievers) do
			-- lprint(".onWindowMessage")
			if r:onWindowMessage(message, lparam, lparam1, lparam2, wparam, w1) then
				flag = true
			end
		end

	end

	return flag
end

function MessagePump:registerReciever(reciever)
	for message, messageName in pairs(self.messageNames) do
		if reciever[messageName] ~= nil and type(reciever[messageName]) == "function" then
			self:registerMessage(message, reciever)
		end
	end
end

function MessagePump:registerMessage(message, reciever)
	if self.messageMap[message] == nil then self.messageMap[message] = { } end
	-- lprint("message " .. message)
	table.insert(self.messageMap[message], reciever)
end


return MessagePump 
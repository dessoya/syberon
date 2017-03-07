local Object = require("Object")
local WindowsConst = require("Windows\\Const")

local Keys = Object:extend()

function Keys:initialize()

	self.recievers = { }
	self.keyStates = { }
	for i = 1, 256 do
		self.keyStates[i] = false
	end

end

function Keys:registerReciever(r)
	table.insert(self.recievers, r)
end

function Keys:attachToPump(pump)

	pump:addNames({
		[WindowsConst.WM.SYSKeyDown]	= "onSysKeyDown",
		[WindowsConst.WM.SYSKeyUp]		= "onSysKeyUp",
		[WindowsConst.WM.KeyDown]		= "onKeyDown",
		[WindowsConst.WM.KeyUp]			= "onKeyUp",
		[WindowsConst.WM.Active]		= "onWindowActive"
	})

	pump:registerReciever(self)	
end

-- 18 - alt key

function Keys:onSysKeyDown(lparam, l1, l2, wparam)
	-- lprint("Keys:onSysKeyDown " .. lparam .. " " .. wparam)
	self:keyDown(wparam)
end

function Keys:onSysKeyUp(lparam, l1, l2, wparam)
	-- lprint("Keys:onSysKeyUp " .. lparam .. " " .. wparam)
	self:keyUp(wparam)
end

function Keys:onKeyDown(lparam, l1, l2, wparam)
	-- lprint("Keys:onKeyDown " .. lparam .. " " .. wparam)
	self:keyDown(wparam)
end

function Keys:onKeyUp(lparam, l1, l2, wparam)
	-- lprint("Keys:onKeyUp " .. lparam .. " " .. wparam)
	self:keyUp(wparam)
end

function Keys:onWindowActive()
	self.keyStates[18] = false
end

function Keys:keyDown(key)
	if not self.keyStates[key] then
		self.keyStates[key] = true
		self:keyPressed(key, self.keyStates[18])
	end
end

function Keys:keyUp(key)
	if self.keyStates[key] then
		self.keyStates[key] = false
		self:keyUnPressed(key, self.keyStates[18])
	end
end

function Keys:keyPressed(key, alt)
	-- lprint("Keys:keyPressed " .. key .. " " .. bn[alt])
	for index, r in pairs(self.recievers) do
		r:keyPressed(key, alt)
	end
end

function Keys:keyUnPressed(key, alt)
	-- lprint("Keys:keyUnPressed " .. key .. " " .. bn[alt])
	for index, r in pairs(self.recievers) do
		r:keyUnPressed(key, alt)
	end
end

Keys.Codes = {
	Alt = 18,
	Esc = 0x1b,
	F4 = 0x73,
	W = 0x57,
	S = 0x53,
	A = 0x41,
	D = 0x44
}

return Keys
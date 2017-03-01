
C_InstallModule("timer")

local Object = require("Object")
local WindowsConst = require("Windows\\Const")

local Timer = Object:extend()

local hwnd
local timerId = 1000
local timersMap = { }

function Timer:initialize(ms, cb)
	self.id = timerId
	timerId = timerId + 1
	self.cb = cb
	timersMap[self.id] = self
	C_Timer_Set(hwnd, self.id, ms)
end

function Timer:kill()
	C_Timer_Kill(hwnd, self.id)
	timersMap[self.id] = nil
end

return {

	init = function(_hwnd, pump)
		hwnd = _hwnd
		pump:addNames({ [WindowsConst.WM.Timer] = "onTimer" })
		pump:registerReciever({
			onTimer	= function(object, lparam, lparam1, lparam2, wparam)
				if timersMap[wparam] ~= nil then
					local timer = timersMap[wparam]
					local cb = timer.cb
					cb()
				end
			end
		})
	end,

	new = function(object, hwnd, ms, cb)
		return Timer:new(hwnd, ms, cb)
	end	
}
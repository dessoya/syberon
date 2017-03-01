C_InstallModule("music")

local Object = require("Object")
local Music = Object:extend()

function Music:initialize(filepath)
	self._ptr = C_Music_New(filepath)
end

function Music:play()
	C_Music_play(self._ptr)
end

return Music
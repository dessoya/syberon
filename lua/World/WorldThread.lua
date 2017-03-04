
local World = require("World\\World")

function thread(hwnd, renderer_ptr)

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_Thread_SetName("World")

	lprint("start")

	local world = World:new(hwnd, renderer_ptr)
	world:start()

	lprint("done")
end
 
 
 local Interface = require("World\\Interface")

function thread(hwnd)

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_Thread_SetName("Interface")

	lprint("start")

	local interface = Interface:new(hwnd)
	interface:start()

	lprint("done")
end
 
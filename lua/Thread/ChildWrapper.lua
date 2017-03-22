C_InstallModule("ds_array")
C_InstallModule("thread")

function thread(filename, ownerQueue, childQueue, ...)

	C_Thread_SetName(filename)

	local status, r = pcall(C_ExecuteFile, filename .. ".lua")
	if not status then
		eprint("syntax error in " .. filename .. ".lua")
		return
	end

	local o = r:new(0, ownerQueue, nil, childQueue, ...)
	-- lprint("start")
	o:start()
	-- lprint("done")

end
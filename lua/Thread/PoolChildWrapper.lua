C_InstallModule("ds_array")
C_InstallModule("thread")

function thread(filename, index, ownerQueue, poolQueue, childQueue, ...)

	C_Thread_SetName(filename .. ":" .. index)

	local status, r = pcall(C_ExecuteFile, filename .. ".lua")
	if not status then
		eprint("syntax error in " .. filename .. ".lua")
		return
	end

	local o = r:new(index, ownerQueue, poolQueue, childQueue, ...)

	-- lprint("start")
	local status, r = pcall(o.start, o)
	if not status then
		eprint("syntax error in " .. filename .. ".lua " .. r)
	end
	-- lprint("done")

end
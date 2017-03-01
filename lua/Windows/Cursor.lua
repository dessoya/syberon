C_InstallModule("cursor")

local Cursor = { }

Cursor.cache = { }

Cursor.get = function(id)
	if Cursor.cache[id] == nil then
		Cursor.cache[id] = C_Cursor_load(id)
	end
	return Cursor.cache[id]
end

return Cursor
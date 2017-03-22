C_InstallModule("ds_array")

#include World\Const

local Map = require("Map")
local Object = require("Object")
local Const = require("World\\Const")
local ThreadChild = require("Thread\\Child")
local Random = require("Random")

local MapGenerator = ThreadChild:extend()

function MapGenerator:start()

	self.map = Map:new(self.args[1])
	local rcList = self.args[2]

	C_Thread_SetName("MapGenerator:" .. self.index)

	C_InstallModule("timer")

	local r = Random:new(0)

	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
					
				if messageId == Const.CMD_Quit then
					-- lprint("mapgen time " .. count .. " " .. readTime)
					work = false
				end
			end
		end

		read = true
		while read do
			read = false
			if not self:poolEmpty() then
				read = true
				local messageId, a1, a2, a3, a4, a5 = self:poolGet()
			
				if messageId == Const.MG_Forest then

					local s, x, y, id = a1, a2, a3, a4					

					r.seed = s
					self.map:makeGrass(r, x, y, id)

					self:send(Const.MG_ForestDone)

				elseif messageId == Const.MG_Figure then

					local name, s, x, y, id = a1, a2, a3, a4, a5
					local px, py = x * 1024, y * 1024

					-- check for near rc
					local skip = false
					local i, l = 0, rcList:len()
					while i < l do

						local rc = rcList:get(i)
						local rx, ry = rc:get(#ODP_X, 2) 
						local dx, dy = rx - px, ry - py
						local d = _tointeger(math.sqrt(dx * dx + dy * dy))
						-- lprint("fx " .. px .. " fy " .. py .. " rx " .. rx .. " ry " .. ry .. " d " .. d)
						if d < 40000 then
							skip = true
							break
						end

						i = i + 1
					end

					if not skip then
						r.seed = s
						self.map[name](self.map, r, x, y, id)
					end

					self:send(Const.MG_FigureDone, name)

				end
			end
		end

		C_Thread_yield()

	end

end

return MapGenerator
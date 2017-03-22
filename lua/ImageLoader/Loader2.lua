
local Object = require("Object")
local Const = require("ImageLoader\\Const")
local scale = require("scale")

local ThreadChild = require("Thread\\Child")
local ThreadPool = require("Thread\\Pool")

local Loader = ThreadChild:extend()

function Loader:init(useFileCache)
	self.useFileCache = useFileCache
	self.tasks = 0
	self.load = { }
	self.afterWork = { }
end

function Loader:start()

	C_Thread_SetName("ImageLoader")

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_InstallModule("image")
	C_InstallModule("file")
	C_InstallModule("filecache")
	C_InstallModule("pack")

	-- start threads

	--[[
	if useFileCache and _pack_image then
		unpackThread = ThreadHost:new("ImageLoader\\Unpacker", self.childQueue)
	end
	]]

	--[[

	  Const.IC_Load, "controls.png"
	, Const.IC_Split, 53, 73, 22, 8, "player-basic-idle.png"
	, Const.IC_Scale, Const.Scale_xy, 22, 8, "player-basic-idle.png"
	, Const.IC_Split, 48, 71, 22, 8, "player-basic-run.png"
	, Const.IC_Scale, Const.Scale_xy, 22, 8, "player-basic-run.png"

	, Const.IC_Scale, Const.Scale_one, "png_01\\Grass_02.png"
	, Const.IC_Scale, Const.Scale_one, "png_01\\Ground.png"
	, Const.IC_Scale, Const.Scale_one, "png_02\\Water_01.png"
	, Const.IC_Scale, Const.Scale_one, "png_02\\Mountain_01.png"
	, Const.IC_Scale, Const.Scale_one, "png_01\\Stone_03_grass.png"

	]]

	self:addLoad("controls.png")
	self:addSplitScale("player-basic-idle.png", 53, 73, 22, 8)
	self:addSplitScale("player-basic-run.png", 48, 71, 22, 8)
	self:addScale("png_01\\Grass_02.png")
	self:addScale("png_01\\Ground.png")
	self:addScale("png_02\\Water_01.png")
	self:addScale("png_02\\Mountain_01.png")
	self:addScale("png_01\\Stone_03_grass.png")
	self:addScale("png_02\\Kristal_01.png")
	self:addOneScale("png_02\\Kristal_01.png", 1, 10)


	local work = true
	while work do

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				local messageId, a1, a2 = self:get()
			
				if messageId == Const.CMD_StartLoad then

					self._start = C_Timer_QueryPerformanceCounter()

					self:send(Const.CMD_ImageCount, self.tasks)
					self:startWork(self.load)

				elseif messageId == Const.CMD_Quit then

					local _end = C_Timer_QueryPerformanceCounter()
					local time = _end - self._start
					lprint("load time " .. time)

					work = false

					if self.readPool ~= nil then self.readPool:sendAll(Const.CMD_Quit) end
					if self.scalePool ~= nil then self.scalePool:sendAll(Const.CMD_Quit) end					

				elseif messageId == Const.CMD_ImageData then

					local name, image = a1, a2
					self:onImage(name, image)

				end

			end

		end
		
		C_Thread_yield()
	end

end

function Loader:addLoad(name)

	self.tasks = self.tasks + 1

	table.insert(self.load, { type = Const.W_Read, name = name})

end

function Loader:addSplitScale(name, iw, ih, w, h)

	self.tasks = self.tasks + 1 + ((scale.count + 1) * (w * h))
	table.insert(self.load, { type = Const.W_Read, name = name})

	local list = { }
	for x = 1, w do
		for y = 1, h do
			local dst = name .. "_" .. x .. "x" .. y
			table.insert(list, {
				type = Const.W_Split,
				src = name,
				dst = dst,
				w = iw,
				h = ih,
				x = x * iw - iw,
				y = y * ih - ih })

			local slist = { }
			for i = 1, scale.count do
				local s = scale.d[i]
				local m = s[1]
				local d = s[2]

				table.insert(slist, { type = Const.W_Scale, src = dst, dst = dst .. "_" .. i, m = m, d = d })
			end
			self.afterWork[dst] = slist

		end
	end

	self.afterWork[name] = list
end


function Loader:addOneScale(name, m, d)	
	self.tasks = self.tasks + 1
	table.insert(self.afterWork[name], { type = Const.W_Scale, src = name, dst = name .. "_" .. m .. "d" .. d, m = m, d = d })
end

function Loader:addScale(name)

	self.tasks = self.tasks + scale.count + 1
	table.insert(self.load, { type = Const.W_Read, name = name})

	local list = { }
	for i = 1, scale.count do
		local s = scale.d[i]
		local m = s[1]
		local d = s[2]

		table.insert(list, { type = Const.W_Scale, src = name, dst = name .. "_" .. i, m = m, d = d })
	end

	self.afterWork[name] = list
end

function Loader:onImage(name, image)

	-- lprint("Loader:onImage " .. name)

	C_Image_register(name, image)
	self:send(Const.CMD_Image)	

	-- check for sub task
	self:startWork(self.afterWork[name])

end


function Loader:sendToScaler(src, dst, m, d)

	if self.scalePool == nil then
		self.scalePool = ThreadPool:new("ImageLoader\\Scaler", self.childQueue, 8)
	end

	-- lprint("src " .. src .. ", dst " .. dst .. ", m " .. m .. ", d " .. d);
	self.scalePool:send(Const.CMD_ScaleImage, src, dst, m, d)	

end

function Loader:sendToSplitter(src, dst, w, h, x, y)

	if self.scalePool == nil then
		self.scalePool = ThreadPool:new("ImageLoader\\Scaler", self.childQueue, 8)
	end

	-- lprint("src " .. src .. ", dst " .. dst .. ", m " .. m .. ", d " .. d);
	self.scalePool:send(Const.CMD_SplitImage, src, dst, w, h, x, y)	

end


function Loader:sendToReader(name)

	if self.readPool == nil then
		self.readPool = ThreadPool:new("ImageLoader\\Reader", self.childQueue, 8)
	end

	self.readPool:send(Const.CMD_ReadImage, name)	

end

function Loader:startWork(list)

	if list == nil then return end

	local c = 0
	for i, work in pairs(list) do

		if work.type == Const.W_Read then

			self:sendToReader(work.name)

		elseif work.type == Const.W_Scale then

			-- lprint("----- work")
			-- dump(work)

			self:sendToScaler(work.src, work.dst, work.m, work.d)

		elseif work.type == Const.W_Split then

			self:sendToSplitter(work.src, work.dst, work.w, work.h, work.x, work.y)
		end

		c = c + 1
		if c > 10 then
			c = 0
			C_Thread_yield()
		end
	end

end

return Loader
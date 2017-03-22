
local Object = require("Object")
local Const = require("ImageLoaderConst")
local scale = require("scale")

local useFileCache = false

local _pack_image = false

local opt = ""

if _pack_image then
	opt = opt .. "b"
else
	opt = opt .. "p"
end

local OLoad = Object:extend()
function OLoad:initialize(name)
	self.name = name
end

function OLoad:process(cb)
	local image = C_Image_New("resource\\" .. self.name)
	cb(self.name, image)
end

local OSplitOne = Object:extend()
function OSplitOne:initialize(name, splitName, w, h, x, y)
	self.name = name
	self.splitName = splitName
	self.w = w
	self.h = h
	self.x = x
	self.y = y
end

local OSplitOneTime = 0

--[[

read from cache

non packed		128.727
				130.082

packed			276.399
				288.896

write to cache

non packed		5.313.948
				5.284.830

packed			16.530.438
				10.775.872

just scale 		334.908

]]

--[[

read from cache

non packed		1.529.656
				

packed			3.226.323
				

write to cache

non packed		45.213.492
				

packed			87.923.497
				

just scale 		5.107.172

]]


function OSplitOne:process(cb)

	local splitImage

	local _start = C_Timer_QueryPerformanceCounter()

	if useFileCache then

		local fwtime = C_File_getWTime("resource\\" .. self.name)
		local cacheFile = C_FileCache_getDir() .. "\\" .. self.splitName .. "_" .. fwtime .. "_" .. opt


		if C_FileCache_exists(cacheFile) then
			-- lprint("load from cache")
			splitImage = C_Image_NewFromCache(cacheFile, _pack_image)
		else
			-- lprint("make new image")
			local image = C_Image_get(self.name)
			splitImage = C_Image_scaleEx(image, 1, 1, self.x, self.y, self.w, self.h)
			C_Image_SaveToCache(splitImage, cacheFile, _pack_image)
		end

	else

		local image = C_Image_get(self.name)
		splitImage = C_Image_scaleEx(image, 1, 1, self.x, self.y, self.w, self.h)

	end

	local _end = C_Timer_QueryPerformanceCounter()
	OSplitOneTime = OSplitOneTime + (_end - _start)

	cb(self.splitName, splitImage)

end

local OScaleOne = Object:extend()
function OScaleOne:initialize(name, scaleName, m, d, oname)
	self.name = name
	self.oname = oname
	self.scaleName = scaleName
	self.m = m
	self.d = d
end

local OScaleOneTime = 0

function OScaleOne:process(cb)

	local scaleImage

	local _start = C_Timer_QueryPerformanceCounter()

	if useFileCache then

		local fwtime = C_File_getWTime("resource\\" .. self.oname)
		local cacheFile = C_FileCache_getDir() .. "\\" .. C_FileCache_prepareFilename(self.scaleName) .. "_" .. fwtime .. "_" .. opt


		if C_FileCache_exists(cacheFile) then
			-- lprint("load from cache")
			scaleImage = C_Image_NewFromCache(cacheFile, _pack_image)
		else
			-- lprint("make new image")
			local image = C_Image_get(self.name)
			scaleImage = C_Image_scale(image, self.m, self.d)
			C_Image_SaveToCache(scaleImage, cacheFile, _pack_image)
		end


	else

		local image = C_Image_get(self.name)
		scaleImage = C_Image_scale(image, self.m, self.d)

	end

	local _end = C_Timer_QueryPerformanceCounter()
	OScaleOneTime = OScaleOneTime + (_end - _start)

	cb(self.scaleName, scaleImage)

end


local OSplit = Object:extend()

function OSplit:initialize(o, name, w, h, iw, ih)
	table.insert(o, OLoad:new(name))
	for x = 1, iw do
		for y = 1, ih do
			table.insert(o, OSplitOne:new(name, name .. "_" .. x .. "x" .. y, w, h, x * w - w, y * h - h))
		end
	end
end

local OScale_xy = Object:extend()

function OScale_xy:initialize(o, name, w, h)
	for i = 1, scale.count do
		local s = scale.d[i]
		local m = s[1]
		local d = s[2]
		for x = 1, w do
			for y = 1, h do
				table.insert(o, OScaleOne:new(name .. "_" .. x .. "x" .. y, name .. "_" .. x .. "x" .. y .. "_" .. i, m, d, name ))
			end
		end
	end
end

local OScale_one = Object:extend()

function OScale_one:initialize(o, name)
	table.insert(o, OLoad:new(name))
	for i = 1, scale.count do
		local s = scale.d[i]
		local m = s[1]
		local d = s[2]
		table.insert(o, OScaleOne:new(name, name .. "_" .. i, m, d, name))
	end
end


local _Images = {

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
	
--	, "font_v1.png"
}

-- make micro command

function makeStream(t)
	local s = { empty = false, data = t, p = 1, l = table.getn(t) }

	s.get = function(stream)
		-- lprint("" .. stream.p .. " " .. stream.l)
		-- dump(stream.data)
		local d = stream.data[stream.p]
		stream.p = stream.p + 1
		if stream.p > stream.l then
			stream.empty = true
		end
		-- lprint("" .. d)
		return d
	end

	return s
end

function makeOperations(commands)
	local operations = { }

	local stream = makeStream(commands)
	while not stream.empty do

		local command = stream:get()

		if command == Const.IC_Load then

			local name = stream:get()

			table.insert(operations, OLoad:new(name))

		elseif command == Const.IC_Split then

			local w = stream:get()
			local h = stream:get()
			local iw = stream:get()
			local ih = stream:get()
			local name = stream:get()
			
			local o = OSplit:new(operations, name, w, h, iw, ih)

		elseif command == Const.IC_Scale then

			local scaleCommand = stream:get()
			if scaleCommand == Const.Scale_xy then
				local w = stream:get()
				local h = stream:get()
				local name = stream:get()
			
				local o = OScale_xy:new(operations, name, w, h)
			elseif scaleCommand == Const.Scale_one then
				local name = stream:get()
			
				local o = OScale_one:new(operations, name)

			end

		end

	end

	return operations
end


local Images = makeOperations(_Images)
lprint(table.getn(Images))


local TestImages = { }

if 0 == 1 then
	for i = 1, 200 do
		table.insert(TestImages, i)
	end
end

function thread(hwnd, _useFileCache)

	dump(_useFileCache)
	lprint("useFileCache " .. bn[_useFileCache])

	useFileCache = _useFileCache

	C_InstallModule("timer")
	C_InstallModule("thread")
	C_InstallModule("image")
	C_InstallModule("file")
	C_InstallModule("filecache")
	C_Thread_SetName("ImageLoader")

	lprint("start")

	local work = true
	while work do

		while C_Thread_PeekMessage() do			
			local message = C_Thread_GetMessageId()
			
			if message == Const.CMD_StartLoad then
				local imageCount = table.getn(Images) + table.getn(TestImages)
				C_Thread_PostMessage(hwnd, Const.CMD_ImageCount, imageCount)

				for k, o in pairs(Images) do
					--[[
					local image = C_Image_New("resource\\" .. v)
					C_Thread_PostMessage(hwnd, Const.CMD_Image, C_PackTable({ name = v, image = image }))		
					]]
					o:process(function(name, image)
						C_Image_register(name, image)
						-- 
						-- C_Thread_PostMessage(hwnd, Const.CMD_Image, C_PackTable({ name = name, image = image }))
						C_Thread_PostMessage(hwnd, Const.CMD_Image)
					end)
				end

				lprint("OSplitOneTime " .. OSplitOneTime)
				lprint("OScaleOneTime " .. OScaleOneTime)


				for k, v in pairs(TestImages) do

					C_Timer_Sleep(10)
					C_Thread_PostMessage(hwnd, Const.CMD_Image, C_PackTable({ name = v, image = v }))		
				end

			elseif message == Const.CMD_Quit then
				work = false
			end

		end

		C_Timer_Sleep(100)
	end

	lprint("done")
end

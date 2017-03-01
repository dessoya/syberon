local Object = require("Object")
local ILConst = require("ImageLoaderConst")
local Thread = require("Thread")
local GUI = require("GUI")

local ImageLoader = Object:extend()

function ImageLoader:initialize(hwnd, messagePump, renderer, cb)

	self.renderer = renderer
	self.cb = cb
	self.images = { }

	messagePump:addNames({
		[ILConst.CMD_ImageCount]		= "onImageCount",
		[ILConst.CMD_Image]				= "onImage"
	})

	messagePump:registerReciever(self)

	self.thread = Thread:new("ImageLoaderThread", hwnd)
	self.thread:send(ILConst.CMD_StartLoad)

end

function ImageLoader:onImageCount(lparam)

	self._images = lparam
	self._imagesLoaded = 0

	self.renderer:modify(function()
		self.progressBar = self.renderer:add(GUI.ProgressBar:new("Load images", lparam))
	end)

end

function ImageLoader:onImage(lparam)

	local data = C_UnpackTable(lparam)
	local imageName = data.name
	local image = data.image

	self.images[imageName] = image

	self.progressBar:inc()
	self._imagesLoaded = self._imagesLoaded + 1
	if self._imagesLoaded == self._images then

		self._imagesLoaded = nil
		self._images = nil

		self.thread:send(ILConst.CMD_Quit)

		self.renderer:modify(function()
			self.renderer:del(self.progressBar)			
		end)

		self.cb(self.images)

	end

end

return ImageLoader
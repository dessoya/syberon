local Object = require("Object")
local ILConst = require("ImageLoader\\Const")
local ThreadHost = require("Thread\\Host")
local GUI = require("GUI")

local ImageLoader = Object:extend()

function ImageLoader:initialize(ownerQueue, messagePump, renderer, useFileCache, cb)

	self.renderer = renderer
	self.cb = cb
	self.images = { }

	messagePump:addNames({
		[ILConst.CMD_ImageCount]		= "onImageCount",
		[ILConst.CMD_Image]				= "onImage"
	})

	messagePump:registerReciever(self)
	self.thread = ThreadHost:new("ImageLoader\\Loader2", ownerQueue, useFileCache)
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

	self.progressBar:inc()
	self._imagesLoaded = self._imagesLoaded + 1
	if self._imagesLoaded == self._images then

		self._imagesLoaded = nil
		self._images = nil

		self.thread:send(ILConst.CMD_Quit)

		self.renderer:modify(function()
			self.renderer:del(self.progressBar)			
		end)

		self.cb()

	end

end

return ImageLoader
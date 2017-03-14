
local GUIObject = require("GUI\\Object")
local Map = GUIObject:extend()

function Map:initialize(map, scales, cells)

	--[[
	self.cw = cw
	self.ch = ch
	]]

	self.x = -1
	self.y = -1
	self.nx = 0
	self.ny = 0
	self.vw = 0
	self.vh = 0
	self._ptr = C_GUI_Map_New(map._ptr, scales, cells)
	self.id = self:getID()
end

function Map:setScaleInfo(s, cw, ch)
	C_GUI_Map_setScaleInfo(self._ptr, s, cw, ch)
end

function Map:setScale(s)
	self.scale = s
	C_GUI_Map_setScale(self._ptr, s)
end

function Map:setupCellImage(s, id, image, x, y)
	C_GUI_Map_setupCellImage(self._ptr, s, id, image, x, y)
end

function Map:setupViewSize(wp, hp)
	self.vw = math.floor(wp)
	self.vh = math.floor(hp)
	C_GUI_Map_setupViewSize(self._ptr, self.vw, self.vh)
end

function Map:getScaleK()
	return C_GUI_Map_getScaleK(self._ptr)
end

function Map:updateCells()
	C_GUI_Map_updateCells(self._ptr)
end

function Map:setCoords()

	self.nx = math.floor(self.nx)
	self.ny = math.floor(self.ny)

	if self.x ~= self.nx or self.y ~= self.ny then
		self.x = self.nx
		self.y = self.ny
		-- lprint("Map:setCoords " .. math.floor(self.nx) .. " " .. math.floor(self.ny))
		C_GUI_Map_setCoords(self._ptr, self.nx, self.ny)
	end
end



return Map
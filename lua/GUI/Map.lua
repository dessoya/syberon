
local GUIObject = require("GUI\\Object")
local Map = GUIObject:extend()

function Map:initialize(map, cw, ch)
	self.cw = cw
	self.ch = ch
	self.x = 0
	self.y = 0
	self.nx = 0
	self.ny = 0
	self.vw = 0
	self.vh = 0
	self._ptr = C_GUI_Map_New(map._ptr, cw, ch)
	self.id = self:getID()
end

function Map:setupCellImage(id, image, x, y)
	C_GUI_Map_setupCellImage(self._ptr, id, image, x, y)
end

function Map:setupViewSize(w, h, wp, hp)
	self.vw = wp
	self.vh = hp
	C_GUI_Map_setupViewSize(self._ptr, w, h)
end

function Map:setCoords()
	if self.x ~= self.nx or self.y ~= self.ny then
		self.x = self.nx
		self.y = self.ny
		-- lprint("Map:setCoords " .. math.floor(self.nx) .. " " .. math.floor(self.ny))
		C_GUI_Map_setCoords(self._ptr, math.floor(self.nx), math.floor(self.ny))
	end
end



return Map
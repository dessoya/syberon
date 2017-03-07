
local GUIObject = require("GUI\\Object")
local Map = GUIObject:extend()

function Map:initialize(map, cw, ch)
	self.cw = cw
	self.ch = ch
	self._ptr = C_GUI_Map_New(map._ptr, cw, ch)
	self.id = self:getID()
end

function Map:setupCellImage(id, image, x, y)
	C_GUI_Map_setupCellImage(self._ptr, id, image, x, y)
end

function Map:setupViewSize(w, h)
	C_GUI_Map_setupViewSize(self._ptr, w, h)
end



return Map
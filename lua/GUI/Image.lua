C_InstallModule("image")

local GUIObject = require("GUI\\Object")
local Image = GUIObject:extend()

function Image:initialize(x,y,image,sx,sy,sw,sh,useAlpha)
	self.image = image
	self.x = x
	self.y = y
	self.sx = sx
	self.sy = sy
	self.sw = sw
	self.sh = sh
	self.useAlpha = useAlpha
	self._ptr = C_GUI_Image_New(x, y, image, sx, sy, sw, sh, useAlpha)
	self.id = self:getID()
end

function Image:setProp()
	C_GUI_Image_setProp(self._ptr, self.x, self.y, self.image, self.sx, self.sy, self.sw, self.sh, self.useAlpha)
end


return Image

local Const = { }

Const.WM = {
	R_RecreateSurface = 0x420,
	R_DisableFullscreen = 0x421,
	R_EnableFullscreen = 0x422,	

	Timer = 0x0113,
	Size = 5,
	MouseMove = 0x200,
	LButtonDown = 0x0201,
	LButtonUp = 0x0202,
	Active = 0x0006,
	RButtonDown = 0x0204
}

Const.Cursor = {
	Hand = 32649,
	Arrow = 32512
}

Const.Mouse = {
	LeftButton = 1
}


return Const 
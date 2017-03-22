
local Const = {

	CMD_StartLoad = 0x420,
	CMD_ImageCount = 0x421,
	CMD_Image = 0x422,
	CMD_Quit = 0x423,

	CMD_Unpack = 0x424,
	CMD_ThreadId = 0x425,
	CMD_ImageData = 0x426,
	CMD_ReadImage = 0x427,
	CMD_ScaleImage = 0x428,
	CMD_SplitImage = 0x429,

	W_Read = 1,
	W_Scale = 2,

	-- simple load image	
	IC_Load = 10,

	-- split, w, h
	IC_Split = 11,

	IC_Scale = 13,
	Scale_xy = 10,
	Scale_one = 11,

	IO_Load	= 10
}

return Const 
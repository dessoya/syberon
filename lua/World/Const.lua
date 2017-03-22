
local Const = {

	CMD_Quit		= 0x430,
	CMD_Create		= 0x431,
	CMD_Queue		= 0x435,
	CMD_AddObject	= 0x436,
	CMD_Timer		= 0x437,
	CMD_Keys		= 0x438,
	CMD_UpdateObject= 0x439,
	CMD_UpdateOptions= 0x43a,
	CMD_DelObject	= 0x43b,
	CMD_SetScale	= 0x43c,
	CMD_SetCursor	= 0x43d,
	MG_ForestDone	= 0x43e,
	MG_FigureDone	= 0x43f,

	CMD_Data		= 0x440,
	CMD_PlayerCoords= 0x441,
	CMD_MiniMap_Opened = 0x442,
	CMD_MiniMap_Closed = 0x443,
	CMD_Rescale		= 0x444,
	CMD_RescaleDone	= 0x445,

	MG_Forest		= 1,
	MG_Figure		= 2,

	Q_World			= 1,
	Q_Interface		= 2
}

Const.Options = {
	Interface = 1
}

-- object types

#const OT_PLAYER			1
#const OT_STONE				2
#const OT_RESPAWN_CRYSTAL	3

-- object data position

#const ODP_TYPE				0
#const ODP_ID				1
#const ODP_X				2
#const ODP_Y				3
#const ODP_CELL_X			4
#const ODP_CELL_Y			5

return Const 
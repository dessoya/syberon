#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Map.h"
#include "..\Pack.h"
#include "..\Utils.h"

static int luaC_Unpack(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto data = (Data *)ud->data;

	long l = 0;
	l = *((DWORD *)data->_data);

	// lprint("dest data size " + inttostr(l));

	char *d = new char[l];

	Unpacker u(d, l, data->_data + 4, data->_sz - 4);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Data;
	ud->data = new Data(d, l);


	return 1;
}



void lm_Pack_install(lua_State* _l) {

	lua_register(_l, "C_Unpack", luaC_Unpack);
}

LuaModule *lm_Pack = new LuaModule("pack", lm_Pack_install);
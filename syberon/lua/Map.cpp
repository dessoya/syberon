#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Map.h"

static int luaC_Map_New(lua_State *L) {

	auto m = new Map();
	m->addBlock(0, 0);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Map;
	ud->data = m;

	return 1;
}

static int luaC_Map_set(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto id = lua_tointeger(L, 4);

	if (m->getCell(x, y) == ABSENT_CELL) {
		m->addBlock(x, y);
	}

	m->setCell(x, y, id);

	return 0;
}


void lm_Map_install(lua_State* _l) {

	lua_register(_l, "C_Map_New", luaC_Map_New);
	lua_register(_l, "C_Map_set", luaC_Map_set);
	
}

LuaModule *lm_Map = new LuaModule("map", lm_Map_install);
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

	/*
	if (m->getCell(x, y) == ABSENT_CELL) {
		m->addBlock(x, y);
	}

	m->setCell(x, y, id);
	*/
	m->addSetCell(x, y, id);

	return 0;
}

static int luaC_Map_get(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);

	lua_pushinteger(L, m->getCell(x, y));

	return 1;
}


static int luaC_Map_setIfEmpty(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto id = lua_tointeger(L, 4);

	m->addSetIfEmpty(x, y, id);

	return 0;
}

static int luaC_Map_setIfEmptyAndId(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto id = lua_tointeger(L, 4);
	auto id2 = lua_tointeger(L, 5);

	m->addSetIfEmptyAndId(x, y, id, id2);

	return 0;
}



static int luaC_Map_dump(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;

	m->dump();

	return 0;
}


// real coord

#define _1 ((unsigned long long)1)

static int luaC_Map_addID(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto id = lua_tointeger(L, 4);

	auto b = m->addID(x, y, id);

	lua_pushinteger(L, ((long long)b->_x - MAP_MID) * 1024);
	lua_pushinteger(L, ((long long)b->_y - MAP_MID) * 1024);
	lua_pushinteger(L, _1 << (B_BITS + 10));


	return 3;
}

static int luaC_Map_delID(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto id = lua_tointeger(L, 4);

	m->delID(x, y, id);

	return 0;
}


static int luaC_Map_getIDs(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto m = (WorldMap *)ud->data;
	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);

	lua_newtable(L);

	auto b = m->getLastBlock(x, y);

	if (b && b->_count > 0) {
		luaL_setn(L, -1, b->_count);

		int pos = 1;
		lint *p = b->_ids;
		for (int i = 0; i < b->_count; i++, p++, pos++) {
			lua_pushinteger(L, *p);
			lua_rawseti(L, -2, pos);
		}
	}

	return 1;
}
	



void lm_Map_install(lua_State* _l) {

	lua_register(_l, "C_Map_New", luaC_Map_New);
	lua_register(_l, "C_Map_set", luaC_Map_set);
	lua_register(_l, "C_Map_get", luaC_Map_get);
	
	lua_register(_l, "C_Map_dump", luaC_Map_dump);
	lua_register(_l, "C_Map_setIfEmpty", luaC_Map_setIfEmpty);
	lua_register(_l, "C_Map_setIfEmptyAndId", luaC_Map_setIfEmptyAndId);
	
	
	lua_register(_l, "C_Map_addID", luaC_Map_addID);
	lua_register(_l, "C_Map_delID", luaC_Map_delID);
	lua_register(_l, "C_Map_getIDs", luaC_Map_getIDs);
}

LuaModule *lm_Map = new LuaModule("map", lm_Map_install);
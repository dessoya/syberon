#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

static int luaC_Cursor_load(lua_State *L) {

	auto cursor = LoadCursor(NULL, MAKEINTRESOURCE(lua_tointeger(L, 1)));

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Cursor;
	ud->data = cursor;

	return 1;
}

static int luaC_Cursor_set(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	SetCursor((HCURSOR)ud->data);
	return 0;
}



void lm_Cursor_install(lua_State* _l) {

	lua_register(_l, "C_Cursor_set", luaC_Cursor_set);
	lua_register(_l, "C_Cursor_load", luaC_Cursor_load);

}

LuaModule *lm_Cursor = new LuaModule("cursor", lm_Cursor_install);

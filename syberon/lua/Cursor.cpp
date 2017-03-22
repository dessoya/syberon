#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

HINSTANCE _hinst = NULL;
LPCTSTR r_arrow = NULL, r_hand;
HCURSOR c_arrow, c_hand;

static int luaC_Cursor_load(lua_State *L) {

	// lprint("");
	auto id = lua_tointeger(L, 1);
	LPCTSTR r = NULL;
	HCURSOR c = NULL;

	if (id == 32512) {
		// r = r_arrow;
		c = c_arrow;
	}
	else if (id == 32649) {
		// r = r_hand;
		c = c_hand;
	}
	else {
		r = MAKEINTRESOURCE(id);
		c = LoadCursor(_hinst, r);
	}
	/*
	auto cursor = LoadCursor(_hinst, r);
	if (cursor == NULL) {
		lprint("error");
	}
	*/

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Cursor;
	ud->data = c;

	return 1;
}

static int luaC_Cursor_set(lua_State *L) {

	// lprint("");
	auto ud = (UserData *)lua_touserdata(L, 1);
	SetCursor((HCURSOR)ud->data);
	return 0;
}



void lm_Cursor_install(lua_State* _l) {


	if (r_arrow == NULL) {
		r_arrow = IDC_ARROW;
		r_hand = IDC_HAND;
		c_arrow = LoadCursor(_hinst, r_arrow);
		c_hand = LoadCursor(_hinst, r_hand);
	}
	/*
	if (_hinst == NULL) {
		_hinst = GetModuleHandle(NULL);
	}
	*/

	lua_register(_l, "C_Cursor_set", luaC_Cursor_set);
	lua_register(_l, "C_Cursor_load", luaC_Cursor_load);

}

LuaModule *lm_Cursor = new LuaModule("cursor", lm_Cursor_install);

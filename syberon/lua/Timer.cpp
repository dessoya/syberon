#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

static int luaC_Timer_Set(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	HWND hwnd = (HWND)ud->data;
	auto id = lua_tointeger(L, 2);
	auto ms = lua_tointeger(L, 3);

	if (SetTimer(hwnd, id, ms, NULL) == 0) {
		lprint("error with SetTimer");
	}
	return 0;
}

static int luaC_Timer_Kill(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	HWND hwnd = (HWND)ud->data;
	auto id = lua_tointeger(L, 2);

	if (KillTimer(hwnd, id) == 0) {
		lprint("error with KillTimer");
	}
	return 0;
}


static int luaC_Timer_Sleep(lua_State *L) {

	Sleep(lua_tointeger(L, 1));
	return 0;
}

static int luaC_Timer_QueryPerformanceFrequency(lua_State *L) {

	LARGE_INTEGER l;
	QueryPerformanceFrequency(&l);
	lua_pushinteger(L, l.QuadPart);

	return 1;
}

static int luaC_Timer_QueryPerformanceCounter(lua_State *L) {

	LARGE_INTEGER l;
	QueryPerformanceCounter(&l);
	lua_pushinteger(L, l.QuadPart);

	return 1;
}


void lm_Timer_install(lua_State* _l) {

	lua_register(_l, "C_Timer_Set", luaC_Timer_Set);
	lua_register(_l, "C_Timer_Kill", luaC_Timer_Kill);
	
	lua_register(_l, "C_Timer_Sleep", luaC_Timer_Sleep);

	lua_register(_l, "C_Timer_QueryPerformanceFrequency", luaC_Timer_QueryPerformanceFrequency);
	lua_register(_l, "C_Timer_QueryPerformanceCounter", luaC_Timer_QueryPerformanceCounter);
}

LuaModule *lm_Timer = new LuaModule("timer", lm_Timer_install);

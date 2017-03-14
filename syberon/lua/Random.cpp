#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"


static int luaC_Random_get(lua_State *L) {

	auto s = lua_tointeger(L, 1);
	auto r = lua_tointeger(L, 2);

	auto p = s;

	s = (3037 * s + 1877) / 7 * 3559 + 3571;
	// s ^= p >> 17;
	// s ^= p >> 23;
	// s ^= p >> 48;

	s ^= p >> 32;

	if (s < 0) s *= -1;

	lua_pushinteger(L, s);
	lua_pushinteger(L, (s >> 25) % r);

	return 2;
}


void lm_Random_install(lua_State* _l) {

	lua_register(_l, "C_Random_get", luaC_Random_get);

}

LuaModule *lm_Random = new LuaModule("random", lm_Random_install);

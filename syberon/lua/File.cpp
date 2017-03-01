#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

static int luaC_File_Read(lua_State *L) {

	auto filename = lua_tostring(L, 1);

	FILE *f;
	fopen_s(&f, filename, "rb");
	if (f) {

		fseek(f, 0, SEEK_END);
		int sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *b = new char[sz + 1];
		fread(b, sz, 1, f);

		fclose(f);
		b[sz] = 0;
		lua_pushstring(L, b);
		delete b;
	}
	else {
		lua_pushstring(L, "");
	}

	return 1;
}

static int luaC_File_Write(lua_State *L) {

	auto filename = lua_tostring(L, 1);
	auto text = lua_tostring(L, 2);

	FILE *f;
	fopen_s(&f, filename, "wb");

	fwrite(text, strlen(text), 1, f);

	fclose(f);

	return 0;
}


void lm_File_install(lua_State* _l) {

	lua_register(_l, "C_File_Read", luaC_File_Read);
	lua_register(_l, "C_File_Write", luaC_File_Write);	

}

LuaModule *lm_File = new LuaModule("file", lm_File_install);

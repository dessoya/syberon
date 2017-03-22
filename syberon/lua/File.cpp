#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Files.h"

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

#include <map>
#include <string>

typedef std::map<std::string, std::string> WTimeMap;
WTimeMap wtimeMap;

std::string _getWTime(std::string &name) {

	// lprint(name);

	std::map<std::string, std::string>::iterator it = wtimeMap.find(name);

	if (it != wtimeMap.end()) {
		// lprint("from cache " + it->second);
		return it->second;
	}

	OFSTRUCT os;
	auto f = OpenFile(name.c_str(), &os, OF_READ);
	if (f == HFILE_ERROR) {
		eprint("can't open file '" + name + "'");
		return "";
	}
	FILETIME wt;

	auto s = GetFileTime((HANDLE)f, NULL, NULL, &wt);

	if (CloseHandle((HANDLE)f) == 0) {
		lprint("error CloseHandle");
	}

	if (s) {

		// lprint("GetFileTime ok");

		DWORD a = wt.dwHighDateTime ^ wt.dwLowDateTime;
		char buf[16];
		sprintf(buf, "%08X", (DWORD)a);
		// lprint(buf);

		std::string w(buf);

		wtimeMap.insert(std::pair<std::string, std::string>(name, w));
		return w;
	}
	else {
		eprint("can't GetFileTime '" + name + "'");
	}


	return "";
}


static int luaC_File_getWTime(lua_State *L) {

	auto filename = lua_tostring(L, 1);

	if (Files_exists(filename)) {
		File *ff = Files_getInfo(filename);

		lua_pushstring(L, _getWTime(ff->_filename).c_str());
		return 1;

	}

	lua_pushstring(L, _getWTime(std::string(filename)).c_str());
	return 1;
}

static int luaC_File_getData(lua_State *L) {

	auto filename = lua_tostring(L, 1);

	FILE *f = fopen(filename, "rb");

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *b = new char[sz];
	fread(b, sz, 1, f);

	fclose(f);

	UserData *ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Data;
	ud->data = new Data(b, sz);

	return 1;
}




void lm_File_install(lua_State* _l) {

	lua_register(_l, "C_File_Read", luaC_File_Read);
	lua_register(_l, "C_File_Write", luaC_File_Write);	
	lua_register(_l, "C_File_getWTime", luaC_File_getWTime);

	lua_register(_l, "C_File_getData", luaC_File_getData);
	

}

LuaModule *lm_File = new LuaModule("file", lm_File_install);

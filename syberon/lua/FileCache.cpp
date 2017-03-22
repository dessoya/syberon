#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "shlobj.h"

char *fileCacheDir = NULL;

static int luaC_FileCache_getDir(lua_State *L) {
	lua_pushstring(L, fileCacheDir);
	return 1;
}

static int luaC_FileCache_exists(lua_State *L) {
	auto w = makewstr(lua_tostring(L, 1));
	lua_pushboolean(L, PathFileExists(w));
	delete w;
	return 1;
}

static int luaC_FileCache_prepareFilename(lua_State *L) {
	auto f = lua_tostring(L, 1);
	auto l = strlen(f);
	char *s = new char[l + 1];
	char *p = s;

	while (*f) {
		auto c = *f;
		if (c == '\\') {
			c = '_';
		}
		*p = c;
		p++;
		f++;
	}
	*p = 0;
	lua_pushstring(L, s);
	delete s;

	return 1;
}


	
void lm_FileCache_install(lua_State* _l) {

	if (fileCacheDir == NULL) {

		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath))) {

			PathAppend(szPath, L"\\arklabGaming");
			if (!PathFileExists(szPath)) CreateDirectory(szPath, NULL);

			PathAppend(szPath, L"\\krystalion");
			if (!PathFileExists(szPath)) CreateDirectory(szPath, NULL);

			PathAppend(szPath, L"\\cache");
			if (!PathFileExists(szPath)) CreateDirectory(szPath, NULL);

			fileCacheDir = tchar2char(szPath);
		}


	}

	lua_register(_l, "C_FileCache_exists", luaC_FileCache_exists);
	lua_register(_l, "C_FileCache_getDir", luaC_FileCache_getDir);
	lua_register(_l, "C_FileCache_prepareFilename", luaC_FileCache_prepareFilename);
	
}

LuaModule *lm_FileCache = new LuaModule("filecache", lm_FileCache_install);

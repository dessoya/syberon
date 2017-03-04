#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Files.h"


static int luaC_Font_New(lua_State *L) {

	auto s = lua_gettop(L);

	wchar_t *wname;
	int sz;

	wchar_t *wfilepath = NULL;

	if (s == 3) {
		const char *filepath = lua_tostring(L, 1);

		const char *name = lua_tostring(L, 2);
		bool needDelete = false;

		if (Files_exists(filepath)) {
			Files_saveFile(filepath, "font.ttf");
			filepath = "font.ttf";
			needDelete = true;
		}

		wname = makewstr(name);
		sz = lua_tointeger(L, 3);

		wfilepath = makewstr(filepath);
		int nResults = AddFontResourceEx(
			wfilepath,
			FR_PRIVATE,    	// font characteristics
			NULL);

		if (needDelete) {
			// lprint(std::string("delete file font.ttf ") + inttostr(remove("font.ttf")));
		}
		else {
			delete wfilepath;
			wfilepath = NULL;
		}

	}
	else {
		const char *name = lua_tostring(L, 1);
		wname = makewstr(name);

		sz = lua_tointeger(L, 2);
	}
	LOGFONT MyLogFont;

	ZeroMemory(&MyLogFont, sizeof(MyLogFont));
	wcscpy_s(MyLogFont.lfFaceName, wname);
	delete wname;

	MyLogFont.lfQuality = ANTIALIASED_QUALITY;
	MyLogFont.lfHeight = sz;

	// MyLogFont.lfWeight = FW_NORMAL;
	MyLogFont.lfWeight = FW_BOLD;

	MyLogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

	HFONT font = CreateFontIndirect(&MyLogFont);

	if (wfilepath) {

		// lprint(std::string("RemoveFontResource ") + inttostr(RemoveFontResourceEx(wfilepath, FR_PRIVATE, NULL)));
		
		delete wfilepath;
		lprint(std::string("delete file font.ttf ") + inttostr(remove("font.ttf")));
	}

	UserData *ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Font;
	ud->data = font;

	return 1;
}



void lm_Font_install(lua_State* _l) {

	lua_register(_l, "C_Font_New", luaC_Font_New);

}

LuaModule *lm_Font = new LuaModule("font", lm_Font_install);

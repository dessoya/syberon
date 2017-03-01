#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Image.h"
#include "..\ImageFont.h"

static int luaC_ImageFont_New(lua_State *L) {

	auto imagefont = new ImageFont();

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_ImageFont;
	ud->data = imagefont;

	return 1;
}

static int luaC_ImageFont_addSymbols(lua_State *L) {
	
	auto ud = (UserData *)lua_touserdata(L, 1);
	auto imagefont = (ImageFont *)ud->data;

	ud = (UserData *)lua_touserdata(L, 2);
	auto image = (Image *)ud->data;

	auto symbols = lua_tostring(L, 3);

	imagefont->addSymbols(image, symbols);

	return 0;
}



void lm_ImageFont_install(lua_State* _l) {

	lua_register(_l, "C_ImageFont_New", luaC_ImageFont_New);
	lua_register(_l, "C_ImageFont_addSymbols", luaC_ImageFont_addSymbols);

}

LuaModule *lm_ImageFont = new LuaModule("imagefont", lm_ImageFont_install);

#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Image.h"

static int luaC_Image_New(lua_State *L) {

	auto filepath = lua_tostring(L, 1);
	auto image = new Image(filepath);
	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = image;

	return 1;
}

static int luaC_Image_Restore(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	if (ud) {
		auto image = (Image *)ud->data;
		image->restore();
	}
	else {
		lprint("error luaC_Image_Restore");
	}

	return 0;
}



void lm_Image_install(lua_State* _l) {

	lua_register(_l, "C_Image_New", luaC_Image_New);
	lua_register(_l, "C_Image_Restore", luaC_Image_Restore);
}

LuaModule *lm_Image = new LuaModule("image", lm_Image_install);

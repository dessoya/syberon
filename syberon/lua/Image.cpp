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

static int luaC_Image_disableDDS(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	if (ud) {
		auto image = (Image *)ud->data;
		image->disableDDS();
	}
	else {
		lprint("error luaC_Image_disableDDS");
	}

	return 0;
}

static int luaC_Image_scale(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	auto m = lua_tointeger(L, 2);
	auto d = lua_tointeger(L, 3);

	auto scaleImage = new Image(image, m, d);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = scaleImage;

	return 1;
}

static int luaC_Image_scaleEx(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	auto m = lua_tointeger(L, 2);
	auto d = lua_tointeger(L, 3);

	auto sx = lua_tointeger(L, 4);
	auto sy = lua_tointeger(L, 5);
	auto sw = lua_tointeger(L, 6);
	auto sh = lua_tointeger(L, 7);

	auto scaleImage = new Image(image, m, d, sx, sy, sw, sh);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = scaleImage;

	return 1;
}

void lm_Image_install(lua_State* _l) {

	lua_register(_l, "C_Image_New", luaC_Image_New);
	lua_register(_l, "C_Image_Restore", luaC_Image_Restore);
	lua_register(_l, "C_Image_disableDDS", luaC_Image_disableDDS);

	lua_register(_l, "C_Image_scale", luaC_Image_scale);	
	lua_register(_l, "C_Image_scaleEx", luaC_Image_scaleEx);
	
}

LuaModule *lm_Image = new LuaModule("image", lm_Image_install);

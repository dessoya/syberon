#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Image.h"

#include <map>
#include <string>

static int luaC_Image_NewFromCache(lua_State *L) {

	auto filepath = lua_tostring(L, 1);
	auto pack = lua_toboolean(L, 2);

	auto image = new Image(LoadFromCache, filepath, pack);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = image;

	return 1;
}

static int luaC_Image_NewFromData(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto data = (Data *)ud->data;

	auto image = new Image(LoadFromData, data);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = image;

	return 1;
}

static int luaC_Image_NewEmpty(lua_State *L) {

	auto image = new Image(NewEmpty, lua_tointeger(L, 1), lua_tointeger(L, 2));

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_Image;
	ud->data = image;

	return 1;
}



static int luaC_Image_SaveToCache(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;
	
	auto filepath = lua_tostring(L, 2);
	auto pack = lua_toboolean(L, 3);

	image->saveToCache(filepath, pack);

	return 0;
}

static int luaC_Image_Delete(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	delete image;

	return 0;
}


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


static int lua_color(lua_State *L) {
	auto r = lua_tointeger(L, 1);
	auto g = lua_tointeger(L, 2);
	auto b = lua_tointeger(L, 3);
	lua_pushinteger(L, (255 << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
	return 1;
}

static int luaC_Image_setPixel(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	image->setPixel(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));

	return 0;
}

static int luaC_Image_updateDDS(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	image->setupBitmap();

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

static int luaC_Image_drawImage(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto image = (Image *)ud->data;

	ud = (UserData *)lua_touserdata(L, 2);
	auto image2 = (Image *)ud->data;

	auto x = lua_tointeger(L, 3);
	auto y = lua_tointeger(L, 4);

	image->draw(image2, x, y);

	return 0;
}


typedef std::map<std::string, Image *> ImageMap;
ImageMap imageMap;
boost::mutex _imageMap;

static int luaC_Image_get(lua_State *L) {
	boost::unique_lock<boost::mutex> scoped_lock(_imageMap);

	auto name = lua_tostring(L, 1);
	// lprint(name);

	std::map<std::string, Image *>::iterator it = imageMap.find(std::string(name));

	if (it != imageMap.end()) {

		// lprint("found");
		Image *image = it->second;
		auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

		ud->type = UDT_Image;
		ud->data = image;
		return 1;

	}

	lprint(name + " not found");
	lua_pushnil(L);
	return 1;
}

static int luaC_Image_register(lua_State *L) {
	boost::unique_lock<boost::mutex> scoped_lock(_imageMap);

	auto name = lua_tostring(L, 1);

	// lprint(name);

	auto ud = (UserData *)lua_touserdata(L, 2);
	auto image = (Image *)ud->data;

	imageMap.insert(std::pair<std::string, Image *>(std::string(name), image));

	return 0;
}

static int luaC_Image_RestoreAll(lua_State *L) {
	boost::unique_lock<boost::mutex> scoped_lock(_imageMap);

	std::map<std::string, Image *>::iterator it = imageMap.begin(), e = imageMap.end();
	while (it != e) {
		it->second->restore();
		it++;
	}
	
	return 0;
}


void lm_Image_install(lua_State* _l) {

	lua_register(_l, "C_Image_get", luaC_Image_get);
	lua_register(_l, "C_Image_register", luaC_Image_register);
	lua_register(_l, "C_Image_RestoreAll", luaC_Image_RestoreAll);
	

	lua_register(_l, "C_Image_New", luaC_Image_New);
	lua_register(_l, "C_Image_Delete", luaC_Image_Delete);
	
	lua_register(_l, "C_Image_NewEmpty", luaC_Image_NewEmpty);
	lua_register(_l, "C_Image_Restore", luaC_Image_Restore);
	lua_register(_l, "C_Image_disableDDS", luaC_Image_disableDDS);

	lua_register(_l, "_color", lua_color);
	
	lua_register(_l, "C_Image_setPixel", luaC_Image_setPixel);
	lua_register(_l, "C_Image_drawImage", luaC_Image_drawImage);
	
	lua_register(_l, "C_Image_updateDDS", luaC_Image_updateDDS);


	lua_register(_l, "C_Image_scale", luaC_Image_scale);	
	lua_register(_l, "C_Image_scaleEx", luaC_Image_scaleEx);
	
	lua_register(_l, "C_Image_NewFromCache", luaC_Image_NewFromCache);
	lua_register(_l, "C_Image_NewFromData", luaC_Image_NewFromData);
	
	lua_register(_l, "C_Image_SaveToCache", luaC_Image_SaveToCache);
}

LuaModule *lm_Image = new LuaModule("image", lm_Image_install);

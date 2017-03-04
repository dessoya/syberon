#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Renderer.h"
#include <list>

typedef struct {
	int w, h;
} ModeInfo;

HRESULT WINAPI EnumModes_(_In_ LPDDSURFACEDESC2  d, _In_ LPVOID lpContext) {

	if (d->ddpfPixelFormat.dwRGBBitCount != 32) {
		return DDENUMRET_OK;
	}

	if (d->dwFlags & DDSD_HEIGHT && d->dwFlags & DDSD_WIDTH && d->dwFlags & DDSD_PITCH && d->dwFlags & DDSD_PIXELFORMAT && d->dwFlags & DDSD_REFRESHRATE) {

	}
	else {
		return DDENUMRET_OK;
	}

	auto list = (std::list<ModeInfo> *)lpContext;
	ModeInfo i = { d->dwWidth , d->dwHeight };
	for (auto it = list->begin(); it != list->end(); it++) {
		ModeInfo m = *it;
		if (m.w == i.w && m.h == i.h) {
			return DDENUMRET_OK;
		}
	}
	list->push_back(i);
	// lprint(std::string("EnumModes_ ") + inttostr(d->dwWidth) + " " + inttostr(d->dwHeight) + " " + inttostr(d->ddpfPixelFormat.dwRGBBitCount) + " " + inttostr(d->dwRefreshRate));

	return DDENUMRET_OK;
}

bool sort_modes(const ModeInfo &b, const ModeInfo &a) {
	if (a.w != b.w) {
		return a.w < b.w;
	}
	return a.h < b.h;
}

static int luaC_Renderer_getModeList(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	std::list<ModeInfo> list;
	renderer->_dd->EnumDisplayModes(DDEDM_REFRESHRATES, NULL, &list, EnumModes_);
	list.sort(sort_modes);
	lua_newtable(L);
	int n = 1;
	for (auto it = list.begin(); it != list.end(); it++) {
		ModeInfo i = *it;
		// lprint(std::string("Mode: ") + inttostr(i.w) + "x" + inttostr(i.h));
		lua_pushinteger(L, i.w);
		lua_rawseti(L, -2, n++);
		lua_pushinteger(L, i.h);
		lua_rawseti(L, -2, n++);
	}

	return 1;
}



static int luaC_Renderer_New(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);

	HWND hwnd = (HWND)ud->data;
	auto renderer = new Renderer(hwnd, false, 0, 0);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Renderer;
	ud->data = renderer;

	return 1;
}

static int luaC_Renderer_onWindowMessage(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	auto message = lua_tointeger(L, 2);
	auto lparam = lua_tointeger(L, 3);
	auto wparam = lua_tointeger(L, 4);

	bool b = renderer->onWindowMessage(NULL, message, wparam, lparam);
	lua_pushboolean(L, b);

	return 1;
}

static int luaC_Renderer_getFPS(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto *renderer = (Renderer *)ud->data;

	lua_pushinteger(L, renderer->getFPS());

	return 1;
}

static int luaC_Renderer_setFPS(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto *renderer = (Renderer *)ud->data;

	renderer->setFPS(lua_tointeger(L, 2));

	return 0;
}


static int luaC_Renderer_getFrameTime(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto *renderer = (Renderer *)ud->data;

	lua_pushinteger(L, renderer->_frameTime);

	return 1;
}

static int luaC_Renderer_add(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	ud = (UserData *)lua_touserdata(L, 2);
	auto object = (RObject *)ud->data;

	auto layer = lua_tointeger(L, 3);

	renderer->add(object, layer);

	return 0;
}

static int luaC_Renderer_del(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	ud = (UserData *)lua_touserdata(L, 2);
	auto object = (RObject *)ud->data;

	auto layer = lua_tointeger(L, 3);

	renderer->del(object, layer);

	return 0;
}

static int luaC_Renderer_lockObjectList(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	renderer->lockObjectList();

	return 0;
}

static int luaC_Renderer_unlockObjectList(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	renderer->unlockObjectList();

	return 0;
}

static int luaC_Renderer_enableFullScreen(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	renderer->enableFullScreen(lua_tointeger(L, 2), lua_tointeger(L, 3));

	return 0;
}

static int luaC_Renderer_disableFullScreen(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto renderer = (Renderer *)ud->data;

	renderer->disableFullScreen();

	return 0;
}

static int luaC_Renderer_getCurrentWidth(lua_State *L) {
	auto ud = (UserData *)lua_touserdata(L, 1);
	auto r = (Renderer *)ud->data;

	lua_pushinteger(L, r->_monitorModeInfo.dwWidth);
	return 1;
}

static int luaC_Renderer_getCurrentHeight(lua_State *L) {
	auto ud = (UserData *)lua_touserdata(L, 1);
	auto r = (Renderer *)ud->data;

	lua_pushinteger(L, r->_monitorModeInfo.dwHeight);

	return 1;
}

static int luaC_Renderer_getCurrentViewWidth(lua_State *L) {
	auto ud = (UserData *)lua_touserdata(L, 1);
	auto r = (Renderer *)ud->data;

	lua_pushinteger(L, r->_width);
	return 1;
}

static int luaC_Renderer_getCurrentViewHeight(lua_State *L) {
	auto ud = (UserData *)lua_touserdata(L, 1);
	auto r = (Renderer *)ud->data;

	lua_pushinteger(L, r->_height);

	return 1;
}


static int luaC_GUI_Object_getID(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto o = (RObject *)ud->data;

	lua_pushinteger(L, o->_id);

	return 1;
}

static int luaC_GUI_Rect_New(lua_State *L) {

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);
	auto w = lua_tointeger(L, 3);
	auto h = lua_tointeger(L, 4);
	auto r = lua_tointeger(L, 5);
	auto g = lua_tointeger(L, 6);
	auto b = lua_tointeger(L, 7);

	auto rect = new RRect(x,y,w,h,r,g,b);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RRect;
	ud->data = rect;

	return 1;
}

static int luaC_GUI_Rect_setProp(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto rect = (RRect*)ud->data;

	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto w = lua_tointeger(L, 4);
	auto h = lua_tointeger(L, 5);
	auto r = lua_tointeger(L, 6);
	auto g = lua_tointeger(L, 7);
	auto b = lua_tointeger(L, 8);

	rect->setProp(x, y, w, h, r, g, b);

	return 0;
}

static int luaC_GUI_Box_New(lua_State *L) {

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);
	auto w = lua_tointeger(L, 3);
	auto h = lua_tointeger(L, 4);
	auto r = lua_tointeger(L, 5);
	auto g = lua_tointeger(L, 6);
	auto b = lua_tointeger(L, 7);

	auto box = new RBox(x, y, w, h, r, g, b);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RBox;
	ud->data = box;

	return 1;
}

static int luaC_GUI_Box_setProp(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto box = (RBox *)ud->data;

	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto w = lua_tointeger(L, 4);
	auto h = lua_tointeger(L, 5);
	auto r = lua_tointeger(L, 6);
	auto g = lua_tointeger(L, 7);
	auto b = lua_tointeger(L, 8);

	box->setProp(x, y, w, h, r, g, b);

	return 0;
}

static int luaC_GUI_Text_New(lua_State *L) {

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);
	auto text = lua_tostring(L, 3);
	auto ud = (UserData *)lua_touserdata(L, 4);
	auto font = (HFONT)ud->data;
	auto r = lua_tointeger(L, 5);
	auto g = lua_tointeger(L, 6);
	auto b = lua_tointeger(L, 7);

	auto t = makewstr(text);
	auto rtext = new RText(x, y, std::wstring(t), font, r, g, b);
	delete t;

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RText;
	ud->data = rtext;

	return 1;
}

static int luaC_GUI_Text_setProp(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto rtext = (RText *)ud->data;

	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto text = lua_tostring(L, 4);
	ud = (UserData *)lua_touserdata(L, 5);
	auto font = (HFONT)ud->data;
	auto r = lua_tointeger(L, 6);
	auto g = lua_tointeger(L, 7);
	auto b = lua_tointeger(L, 8);

	auto t = makewstr(text);
	rtext->setProp(x, y, std::wstring(t), font, r, g, b);
	delete t;

	return 0;
}

static int luaC_GUI_ImageText_New(lua_State *L) {

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);
	auto text = lua_tostring(L, 3);
	auto ud = (UserData *)lua_touserdata(L, 4);
	auto font = (ImageFont *)ud->data;

	auto t = makewstr(text);
	auto rtext = new RIText(x, y, std::wstring(t), font);
	delete t;

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RIText;
	ud->data = rtext;

	return 1;
}

static int luaC_GUI_ImageText_setProp(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto rtext = (RIText *)ud->data;

	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto text = lua_tostring(L, 4);
	ud = (UserData *)lua_touserdata(L, 5);
	auto font = (ImageFont *)ud->data;
	

	auto t = makewstr(text);
	rtext->setProp(x, y, std::wstring(t), font);
	delete t;

	return 0;
}

static int luaC_GUI_Image_New(lua_State *L) {

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);

	auto ud = (UserData *)lua_touserdata(L, 3);
	auto image = (Image *)ud->data;

	auto sx = lua_tointeger(L, 4);
	auto sy = lua_tointeger(L, 5);
	auto sw = lua_tointeger(L, 6);
	auto sh = lua_tointeger(L, 7);
	auto useAlpha = lua_toboolean(L, 8);

	auto i = new RImage(x, y, image, sx, sy, sw, sh, useAlpha);

	lprint("luaC_GUI_Image_New " + image->_filename + " " + inttostr(x) + " " + inttostr(y) + " " + inttostr(sx) + " " + inttostr(sy) + " " + inttostr(sw) + " " + inttostr(sh));

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RImage;
	ud->data = i;

	return 1;
}

static int luaC_GUI_Image_setProp(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto i = (RImage *)ud->data;

	auto x = lua_tointeger(L, 2);
	auto y = lua_tointeger(L, 3);
	auto sx = lua_tointeger(L, 4);
	auto sy = lua_tointeger(L, 5);
	auto sw = lua_tointeger(L, 6);
	auto sh = lua_tointeger(L, 7);
	auto useAlpha = lua_toboolean(L, 8);

	i->setProp(x, y, sx, sy, sw, sh, useAlpha);

	return 0;
}

static int luaC_GUI_Map_New(lua_State *L) {

	auto cw = lua_tointeger(L, 1);
	auto ch = lua_tointeger(L, 2);
	auto useAlpha = lua_toboolean(L, 8);

	auto m = new RMap(cw, ch);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_RMap;
	ud->data = m;

	return 1;
}

static int luaC_GUI_Map_setupCellImage(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto m = (RMap *)ud->data;

	auto id = lua_tointeger(L, 2);

	ud = (UserData *)lua_touserdata(L, 3);
	auto image = (Image *)ud->data;

	auto x = lua_tointeger(L, 4);
	auto y = lua_tointeger(L, 5);

	m->setupCellImage(id, image, x, y);

	return 0;
}

static int luaC_GUI_Map_setupViewSize(lua_State *L) {

	UserData *ud = (UserData *)lua_touserdata(L, 1);
	auto m = (RMap *)ud->data;

	auto w = lua_tointeger(L, 2);
	auto h = lua_tointeger(L, 3);

	m->setupViewSize(w, h);

	return 0;
}


void lm_Renderer_install(lua_State* _l) {

	lua_register(_l, "C_Renderer_New", luaC_Renderer_New);
	lua_register(_l, "C_Renderer_onWindowMessage", luaC_Renderer_onWindowMessage);
	lua_register(_l, "C_Renderer_setFPS", luaC_Renderer_setFPS);
	lua_register(_l, "C_Renderer_getFPS", luaC_Renderer_getFPS);
	lua_register(_l, "C_Renderer_getFrameTime", luaC_Renderer_getFrameTime);
	lua_register(_l, "C_Renderer_add", luaC_Renderer_add);
	lua_register(_l, "C_Renderer_del", luaC_Renderer_del);
	lua_register(_l, "C_Renderer_lockObjectList", luaC_Renderer_lockObjectList);
	lua_register(_l, "C_Renderer_unlockObjectList", luaC_Renderer_unlockObjectList);
	lua_register(_l, "C_Renderer_enableFullScreen", luaC_Renderer_enableFullScreen);
	lua_register(_l, "C_Renderer_disableFullScreen", luaC_Renderer_disableFullScreen);

	lua_register(_l, "C_Renderer_getModeList", luaC_Renderer_getModeList);
	lua_register(_l, "C_Renderer_getCurrentWidth", luaC_Renderer_getCurrentWidth);
	lua_register(_l, "C_Renderer_getCurrentHeight", luaC_Renderer_getCurrentHeight);
	
	lua_register(_l, "C_Renderer_getCurrentViewWidth", luaC_Renderer_getCurrentViewWidth);
	lua_register(_l, "C_Renderer_getCurrentViewHeight", luaC_Renderer_getCurrentViewHeight);

		
	lua_register(_l, "C_GUI_Object_getID", luaC_GUI_Object_getID);

	lua_register(_l, "C_GUI_Rect_New", luaC_GUI_Rect_New);
	lua_register(_l, "C_GUI_Rect_setProp", luaC_GUI_Rect_setProp);

	lua_register(_l, "C_GUI_Box_New", luaC_GUI_Box_New);
	lua_register(_l, "C_GUI_Box_setProp", luaC_GUI_Box_setProp);

	lua_register(_l, "C_GUI_Text_New", luaC_GUI_Text_New);
	lua_register(_l, "C_GUI_Text_setProp", luaC_GUI_Text_setProp);

	lua_register(_l, "C_GUI_ImageText_New", luaC_GUI_ImageText_New);
	lua_register(_l, "C_GUI_ImageText_setProp", luaC_GUI_ImageText_setProp);

	lua_register(_l, "C_GUI_Image_New", luaC_GUI_Image_New);
	lua_register(_l, "C_GUI_Image_setProp", luaC_GUI_Image_setProp);


	lua_register(_l, "C_GUI_Map_New", luaC_GUI_Map_New);
	lua_register(_l, "C_GUI_Map_setupCellImage", luaC_GUI_Map_setupCellImage);
	lua_register(_l, "C_GUI_Map_setupViewSize", luaC_GUI_Map_setupViewSize);
	
}

LuaModule *lm_Renderer = new LuaModule("renderer", lm_Renderer_install);

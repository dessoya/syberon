#pragma once

#include <string>
#include <windows.h>
#include "Utils.h"

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}

enum {
	UDT_Void = 0,
	UDT_Int,
	UDT_String,
	UDT_TableList,
	UDT_Font,
	UDT_ImageFont,
	UDT_Music,
	UDT_Image,
	UDT_Cursor,
	UDT_Renderer,
	UDT_RRect,
	UDT_RBox,
	UDT_RText,
	UDT_RIText,
	UDT_RImage
};

typedef struct {

	int type;
	void *data;

} UserData;

class DataItem {
public:
	UserData ud;
	DataItem *_next;
	DataItem(void *v) : _next(NULL) { ud.data = v; ud.type = UDT_Void; }
	DataItem(_int i) : _next(NULL) { ud.data = (void *)i; ud.type = UDT_Int; }
	DataItem(const char *s) : _next(NULL) { ud.data = (void *)new std::string(s); ud.type = UDT_String; }
	DataItem(UserData *ud1) : _next(NULL) { ud.data = ud1->data; ud.type = ud1->type; }
	~DataItem() {
		switch (ud.type) {
		case UDT_String:
			delete (std::string *)ud.data;
			break;
		}
	}
	inline DataItem *next() {
		return _next;
	}
	inline void push(lua_State *L) {
		switch (ud.type) {
		case UDT_Int:
			lua_pushinteger(L, (_int)ud.data);
			break;
		case UDT_String:
			lua_pushstring(L, ((std::string *)ud.data)->c_str());
			break;
		case UDT_Void:
		case UDT_Image:
			auto ud1 = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud1->data = ud.data;
			ud1->type = ud.type;
			break;
		}
	}
	inline std::string *get() {
		if (ud.type == UDT_String) {
			return (std::string *)ud.data;
		}
		return NULL;
	}
};

class DataList {
public:
	DataItem *first, *last;
	int count;
	DataList() : first(NULL), last(NULL), count(0) {}
	~DataList() {
		auto item = first;
		while (item) {
			auto t = item;
			item = item->next();
			delete t;
		}
	}

	inline DataList *add(HWND hwnd) { add((void *)hwnd); return this; }
	inline DataList *add(void *v) { add(new DataItem(v)); return this; }
	inline DataList *add(_int i) { add(new DataItem(i)); return this; }
	inline DataList *add(const char *s) { add(new DataItem(s)); return this; }
	inline DataList *add(UserData *ud) { add(new DataItem(ud)); return this; }
	inline DataList *add(DataItem *da) {
		count++;
		if (first) {
			last->_next = da;
		}
		else {
			first = da;
		}
		last = da;
		return this;
	}
	inline DataList *add(lua_State *L, int index) {

		if (lua_isnumber(L, index)) {
			add((_int)lua_tointeger(L, index));
		}
		else if (lua_isstring(L, index)) {
			add(lua_tostring(L, index));
		}
		else if (lua_isuserdata(L, index)) {			
			add((UserData *)lua_touserdata(L, index));
		}
		return this;
	}
	inline void push(lua_State *L) {
		auto item = first;
		while (item) {
			item->push(L);
			item = item->next();
		}
	}
};

#define DL (new DataList())

typedef void(*InstallModule)(lua_State *L);

class LuaModule {
public:
	std::string name;
	InstallModule installModule;
	LuaModule(std::string name_, InstallModule installModule_);
};

class LuaValue {

public:
	bool b;
	const char *s;

	LuaValue() : b(false), s(NULL) {}
	int toint() { return 0; }
	bool tobool() { return b; }
	const char *tostring() { return s; }
};

class LuaScript {

private:
	lua_State* _l;
	std::string _filepath;
	LuaValue *_returnValue;

public:
	LuaScript();
	virtual ~LuaScript();

	static void addModule(std::string name, InstallModule installModule);

	int executeFile(std::string string);
	int executeObjectMethod(std::string object, std::string method, DataList *dl = NULL);
	int executeFunction(std::string func, DataList *dl = NULL);
	void makeReturnValue(int r);

	std::string getError();

	LuaValue *getReturnValue();
};

class LuaStackGuard {

public:
	LuaStackGuard(lua_State* L) : luaState_(L) {
		top_ = lua_gettop(L);
	}

	~LuaStackGuard() {
		lua_settop(luaState_, top_);
	}

private:
	lua_State* luaState_;
	int top_;
};

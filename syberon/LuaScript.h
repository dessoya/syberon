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

	// scalar types

	UDT_Nil = 0,
	UDT_Void,	
	UDT_Int,
	UDT_Double,
	UDT_Bool,
	UDT_String,

	// complex data type

	UDT_DataStruct,
	UDT_TableList,
	UDT_Data,
	UDT_CQueue,

	// resource types 

	UDT_Font,
	UDT_ImageFont,
	UDT_Music,
	UDT_Image,
	UDT_Cursor,

	// modules

	UDT_Map,
	UDT_MapPointer,
	UDT_Renderer,

	// renderer objects 

	UDT_RRect,
	UDT_RBox,
	UDT_RText,
	UDT_RIText,
	UDT_RImage,
	UDT_RMap
};

class Data {
public:
	char *_data;
	int _sz;

	Data(char *data, int sz) { _data = data; _sz = sz; }
};

typedef struct {

	int type;
	void *data;

} UserData;

class LuaArgumants {
	unsigned char *_data;
public:
	LuaArgumants(lua_State *L, int skip = 0);
	inline int count() {
		return *_data;
	}
	int push(lua_State *L);
	inline ~LuaArgumants() {
		delete _data;
	}
};


class DataItem {
public:
	UserData ud;
	DataItem *_next;
	DataItem(void *v) : _next(NULL) { ud.data = v; ud.type = UDT_Void; }
	DataItem(_int i) : _next(NULL) { ud.data = (void *)i; ud.type = UDT_Int; }
	DataItem(BOOL b, int d) : _next(NULL) { ud.data = (void *)b; ud.type = UDT_Bool; }
	DataItem(const char *s) : _next(NULL) { auto l = strlen(s); char *a = new char[l + 1]; memcpy(a, s, l + 1); ud.data = (void *)a; ud.type = UDT_String; }
	DataItem(UserData *ud1) : _next(NULL) { ud.data = ud1->data; ud.type = ud1->type; }
	~DataItem() {
		switch (ud.type) {
		case UDT_String:
			/*
			if (ud.data) {
				delete (std::string *)ud.data;
				ud.data = NULL;
			}
			*/
			delete (char *)ud.data;
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
		case UDT_Bool:
			lua_pushboolean(L, (BOOL)ud.data);
			break;
		case UDT_String:
			lua_pushstring(L, (char *)ud.data);
			break;
		case UDT_Void:
		case UDT_Data:
		case UDT_Image:
		case UDT_Renderer:
		case UDT_RRect:
			auto ud1 = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud1->data = ud.data;
			ud1->type = ud.type;
			break;
		}
	}
	/*
	inline std::string *get() {
		if (ud.type == UDT_String) {
			return (std::string *)ud.data;
		}
		return NULL;
	}
	*/
	char *get() {
		if (ud.type == UDT_String) {
			return (char *)ud.data;
		}
		return NULL;
	}
};

class DataList {
public:
	DataItem *first, *last;
	int count;
	// int deleted;
	DataList() : 
		// deleted(0), 
		first(NULL), last(NULL), count(0) {}
	~DataList() {
		// deleted ++;
		// if (first) {
			// first = NULL;
			auto item = first;
			while (item) {
				auto t = item;
				item = item->next();
				delete t;
			 }
		// }
	}

	inline DataList *add(HWND hwnd) { add((void *)hwnd); return this; }
	inline DataList *add(void *v) { add(new DataItem(v)); return this; }
	inline DataList *add(_int i) { add(new DataItem(i)); return this; }
	inline DataList *add(const char *s) { add(new DataItem(s)); return this; }
	inline DataList *addBool(BOOL b) { add(new DataItem(b, 1)); return this; }
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
		else if (lua_isboolean(L, index)) {
			addBool((BOOL)lua_toboolean(L, index));
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

	int _pp_pos;
	int _pp[16];

	LuaScript();
	virtual ~LuaScript();

	static void addModule(std::string name, InstallModule installModule);

	int executeFile(std::string string);
	int executeObjectMethod(std::string object, std::string method, DataList *dl = NULL);
	int executeFunction(std::string func, DataList *dl = NULL);
	int executeFunction(std::string func, LuaArgumants *a);
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

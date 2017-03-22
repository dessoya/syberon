#pragma once

#include "LuaScript.h"

// http://lua-users.org/wiki/SimplerCppBinding

typedef int(*mfp)(lua_State *L);
typedef struct { const char *name; mfp mfunc; } LuaClassRegType;

class LuaDataStruct {

	typedef struct { int type; LuaDataStruct *pT; } userdataType;

public:
	static long long idIterator;

	int _share;

	const char *_class;
	int _id;	

	void registerInNewState(lua_State *L) {
		luaL_getmetatable(L, _class);  // lookup metatable in Lua registry
		lua_setmetatable(L, -2);
	}
	
	LuaDataStruct() : _share(0)
		, _id(LuaDataStruct::idIterator++) 
	{ }
	virtual ~LuaDataStruct() { }

	static LuaDataStruct *getDataStruct(lua_State *L, int narg) {
		userdataType *ud = (userdataType *)lua_touserdata(L, narg);
		return ud->pT;
	}
};

template <typename T> class LuaClass {

	typedef struct { int type; T *pT; } userdataType;

public:

	// typedef int (T::*mfp)(lua_State *L);
	
	static void Register(lua_State *L) {

		lua_newtable(L);
		int methods = lua_gettop(L);

		luaL_newmetatable(L, T::className);
		int metatable = lua_gettop(L);

		// store method table in globals so that
		// scripts can add functions written in Lua.

		lua_pushstring(L, T::className);
		lua_pushvalue(L, methods);
		lua_settable(L, LUA_GLOBALSINDEX);

		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, gc_T);
		lua_settable(L, metatable);


		for (LuaClassRegType *l = T::metamethods; l->name; l++) {
			lua_pushstring(L, l->name);
			lua_pushcfunction(L, l->mfunc);
			lua_settable(L, metatable);
		}

		lua_pushliteral(L, "new");
		lua_pushcfunction(L, new_T);
		lua_settable(L, methods);       // add new_T to method table

										// fill method table with methods from class T
		for (LuaClassRegType *l = T::methods; l->name; l++) {
			lua_pushstring(L, l->name);
			lua_pushcfunction(L, l->mfunc);
			lua_settable(L, methods);
		}


		lua_pop(L, 2);  // drop metatable and method table
	}

private:
	LuaClass();

	/*
	// member function dispatcher
	static int thunk(lua_State *L) {
		// stack has userdata, followed by method args
		T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
		lua_remove(L, 1);  // remove self so member function args start at index 1
						   // get member function from upvalue

		RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
		return (obj->*(l->mfunc))(L);  // call member function
	}
	*/

	// create a new T object and
	// push onto the Lua stack a userdata containing a pointer to T object
	static int new_T(lua_State *L) {
		lua_remove(L, 1);   // use classname:new(), instead of classname.new()
		T *obj = new T(L);  // call constructor for T objects
		obj->_class = T::className;
		userdataType *ud = static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
		ud->pT = obj;  // store pointer to object in userdata
		ud->type = UDT_DataStruct;
		luaL_getmetatable(L, T::className);  // lookup metatable in Lua registry
		lua_setmetatable(L, -2);
		return 1;  // userdata containing pointer to T object
	}

	// garbage collection metamethod
	static int gc_T(lua_State *L) {
		userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
		T *obj = ud->pT;
		// lprint("id " + inttostr(obj->_id) + " " + inttostr(obj->_share));
		if (obj->_share == 0) {
			delete obj;  // call destructor for T objects
		}
		else {
			obj->_share--;
		}
		return 0;
	}

};

#define LuaClass(c) class c : public LuaDataStruct { public: static const char className[]; static LuaClassRegType methods[], metamethods[]; 

#define LuaMethod(c, a) static int _W##a(lua_State *L) { UserData *ud = (UserData *)lua_touserdata(L, 1); c *p = (c *)ud->data; lua_remove(L, 1); return p->a(L); } inline int a(lua_State *L)
#define LuaMethods(c) const char c::className[] = #c; LuaClassRegType c::methods[]
#define LuaMethodDesc(c, name) {#name, c::_W##name}

#define LuaMetaMethods(c) LuaClassRegType c::metamethods[]

// #define LuaClass_staff static const char className[]; static LuaClassRegType methods[];

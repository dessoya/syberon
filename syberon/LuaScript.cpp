
#include "Utils.h"
#include "LuaScript.h"
#include "Logger.h"
#include "Files.h"

#include "lua.hpp"

std::string lastError;

int errorHandler(lua_State* L) {

	const char* err = lua_tostring(L, 1);

	lastError = std::string("Error: ") + err;

	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	if (lua_pcall(L, 0, 1, 0)) {
		const char* err = lua_tostring(L, -1);

		// lprint(std::string("Error in debug.traceback() call: ") + err + "\n");
	}
	else {
		const char* stackTrace = lua_tostring(L, -1);
		lastError += std::string("\n") + stackTrace;
	}

	return 1;
}

static int luaC_lprint(lua_State *L) {
	const char *arg = lua_tostring(L, -1);
	lprint(arg);
	return 0;
}

static int luaC_executeFile(lua_State *L) {

	std::string filename = std::string("lua\\") + lua_tostring(L, -1);
	lprint("luaC_executeFile " + filename);

	char *b; int sz;

	
	File *f = Files_get(&filename.c_str()[4]);
	if (f) {
		lprint(std::string("getting file ") + filename + " from data.pack");
		b = f->_data;
		sz = f->_len;
	}
	else {
		FILE *f;
		fopen_s(&f, filename.c_str(), "rb");

		fseek(f, 0, SEEK_END);
		sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		b = new char[sz];
		fread(b, sz, 1, f);

		fclose(f);
	}

	lastError = "";
	lua_pushcfunction(L, errorHandler);

	if (luaL_loadbuffer(L, b, sz, filename.c_str())) {
		const char *err = lua_tostring(L, -1);
		lprint(std::string("Syntax error in luaC_loadFile: ") + err);
		return luaL_error(L, "%s", err);
	}

	if (lua_pcall(L, 0, LUA_MULTRET, -2)) {
		const char *err = lua_tostring(L, -1);
		lprint(std::string("Exec error in luaC_loadFile: ") + err + lastError);
		// return lua_error(L);
		return 1;
	}

	return 1;
}

LuaModule::LuaModule(std::string name_, InstallModule installModule_) : name(name_), installModule(installModule_) {
	LuaScript::addModule(name, installModule);
}

struct _moduleInfo;
typedef struct _moduleInfo {
	struct _moduleInfo *next;
	std::string name;
	InstallModule installModule;
} ModuleInfo;

ModuleInfo *modules, *lastModule = NULL;

void LuaScript::addModule(std::string name, InstallModule installModule) {
	if (modules) {
		lastModule->next = new ModuleInfo;
		lastModule = lastModule->next;
	}
	else {
		modules = lastModule = new ModuleInfo;
	}
	lastModule->next = NULL;
	lastModule->name = name;
	lastModule->installModule = installModule;
}

static int luaC_InstallModule(lua_State *L) {

	std::string moduleName(lua_tostring(L, 1));
	// lprint("luaC_InstallModule name " + moduleName);
	auto m = modules;
	while (m) {
		// lprint("luaC_InstallModule check " + m->name);
		if (m->name == moduleName) {
			break;
		}
		m = m->next;
	}
	if (m) {
		m->installModule(L);
	}
	else {
		lprint("error luaC_InstallModule: can't find module " + moduleName);
	}
	return 0;
}

static int luaC_PackTable(lua_State *L) {

	DataList *dl = DL;
	lua_pushnil(L);
	while (lua_next(L, 1) != 0) {

		dl->add(L, -2);
		dl->add(L, -1);

		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	UserData *ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_TableList;
	ud->data = dl;
	return 1;
}

static int luaC_UnpackTable(lua_State *L) {

	auto i = lua_tointeger(L, 1);
	auto list = (DataList *)i;

	lua_newtable(L);

	auto item = list->first;

	while (item) {
		std::string *k = item->get();

		item = item->next();
		item->push(L);

		item = item->next();

		lua_setfield(L, -2, k->c_str());
	}

	delete list;

	return 1;
}

static int luaC_Exit(lua_State *L) {

	exit(0);

	return 0;
}

void luaL_requiref(lua_State *L, const char *name, lua_CFunction func, int i) {
	lua_pushcfunction(L, func);
	lua_pushstring(L, name);
	lua_call(L, 1, 0);
}

LuaScript::LuaScript() : _returnValue(NULL) {

	_l = luaL_newstate();
	if (luaJIT_setmode(_l, 0, LUAJIT_MODE_ENGINE) == 0) {
		lprint("luaJIT_setmode error");
	}

	luaL_requiref(_l, "_G", luaopen_base, 1);
	luaL_requiref(_l, LUA_TABLIBNAME, luaopen_table, 1);

	luaL_requiref(_l, LUA_MATHLIBNAME, luaopen_math, 1);
	luaL_requiref(_l, LUA_STRLIBNAME, luaopen_string, 1);
	
	luaL_requiref(_l, LUA_DBLIBNAME, luaopen_debug, 1);
	lua_register(_l, "lprint", luaC_lprint);

	lua_register(_l, "C_ExecuteFile", luaC_executeFile);
	lua_register(_l, "C_InstallModule", luaC_InstallModule);

	lua_register(_l, "C_PackTable", luaC_PackTable);
	lua_register(_l, "C_UnpackTable", luaC_UnpackTable);

	lua_register(_l, "C_Exit", luaC_Exit);

	if (this->executeFile("core.lua")) {
		lprint(this->getError());
	}
}

LuaScript::~LuaScript() {
	if (_returnValue) {
		delete _returnValue;
	}
	lua_close(_l);
}

int LuaScript::executeFile(std::string filepath) {

	LuaStackGuard StackGuard(_l);

	lua_pushstring(_l, filepath.c_str());
	luaC_executeFile(_l);

	return 0;
}

void LuaScript::makeReturnValue(int r) {

	if (_returnValue) {
		delete _returnValue;
		_returnValue = NULL;
	}

	if (r == 0) {
		_returnValue = new LuaValue();
		switch (lua_type(_l, -1)) {
		case LUA_TBOOLEAN:
			_returnValue->b = lua_toboolean(_l, -1);
			break;
		case LUA_TSTRING:
			_returnValue->s = lua_tostring(_l, -1);
			break;
		}
	}
}


int LuaScript::executeFunction(std::string func, DataList *dl) {

	LuaStackGuard stackGuard(_l);

	lastError = "";
	lua_pushcfunction(_l, errorHandler);

	lua_getglobal(_l, func.c_str());
	if (lua_isfunction(_l, -1) == 0) {
		lastError = std::string("Error: [") + _filepath + "] can't find function " + func;
		return 1;
	}

	int params = 0, errorId = -2;
	if (dl) {
		params += dl->count;
		errorId += -1 * dl->count;

		dl->push(_l);
		delete dl;
	}

	int r = lua_pcall(_l, params, 1, errorId);
	makeReturnValue(r);

	return r;
}


int LuaScript::executeObjectMethod(std::string object, std::string method, DataList *dl) {

	LuaStackGuard stackGuard(_l);

	lastError = "";
	lua_pushcfunction(_l, errorHandler);

	lua_getglobal(_l, object.c_str());
	if (lua_istable(_l, -1) == 0) {
		lastError = std::string("Error: [") + _filepath + "] can't find object " + object;
		return 1;
	}

	lua_getfield(_l, -1, method.c_str());
	if (lua_isfunction(_l, -1) == 0) {
		lastError = std::string("Error: [") + _filepath + "] can't execute " + object + ":" + method;
		return 1;
	}

	lua_remove(_l, -2);

	lua_getglobal(_l, object.c_str());
	if (lua_istable(_l, -1) == 0) {
		lastError = std::string("Error: [") + _filepath + "] can't find object " + object;
		return 1;
	}

	int params = 1, errorId = -3;
	if (dl) {
		params += dl->count;
		errorId += -1 * dl->count;

		dl->push(_l);
		delete dl;
	}

	int r = lua_pcall(_l, params, 1, errorId);
	makeReturnValue(r);

	return r;
}

LuaValue *LuaScript::getReturnValue() {
	return _returnValue;
}


std::string LuaScript::getError() {
	return lastError;
}

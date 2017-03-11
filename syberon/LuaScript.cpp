#include "Utils.h"
#include "LuaScript.h"
#include "Logger.h"
#include "Files.h"

#include "lua.hpp"

#include "logConfig.h"

#ifdef _LOG_LUA_SCRIPT
#define lprint_LUA_SCRIPT(text) lprint(text)
#else
#define lprint_LUA_SCRIPT(text)
#endif

#ifdef _LOG_LUA_SCRIPT_EF
#define lprint_LS_EXECUTE_FILE(text) lprint(text)
#else
#define lprint_LS_EXECUTE_FILE(text)
#endif

void _lua_error(lua_State* L, char *buff) {

	std::string s(buff);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	if (lua_pcall(L, 0, 1, 0)) {
		const char* err = lua_tostring(L, -1);
		// lprint(std::string("Error in debug.traceback() call: ") + err + "\n");
	}
	else {
		const char* stackTrace = lua_tostring(L, -1);
		s += std::string("\n") + stackTrace;
	}

	lua_pushstring(L, s.c_str());
	lua_error(L);
}

int errorHandler(lua_State* L) {

	const char* err = lua_tostring(L, 1);

	lua_getglobal(L, "__error_string");
	std::string *lastError = (std::string *)lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (err != NULL) {
		(*lastError) = std::string("Error: ") + err;
	}
	else {
		(*lastError) = "";
	}

	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	if (lua_pcall(L, 0, 1, 0)) {
		const char* err = lua_tostring(L, -1);
		// lprint(std::string("Error in debug.traceback() call: ") + err + "\n");
	}
	else {
		const char* stackTrace = lua_tostring(L, -1);
		(*lastError) += std::string("\n") + stackTrace;
	}

	// lprint(lastError);

	return 1;
}

int _lua_print(lua_State *L, int log_file) {
	const char *arg = lua_tostring(L, -1);
	std::string p;

	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	if (lua_pcall(L, 0, 1, 0)) {
	}
	else {
		const char* stackTrace = lua_tostring(L, -1);
		lua_pop(L, 1);
		std::string s(stackTrace);
		/*
		[2017-03-08 18:04:22] [World             ] stack traceback:
		[2017-03-08 18:04:22] [World             ] 	[C]: in function 'lprint'
		[2017-03-08 18:04:22] [World             ] 	[string "lua\World\World.lua"]:56: in function 'cb'
		[2017-03-08 18:04:22] [World             ] 	[string "lua\World\ImageGetter.lua"]:27: in function 'makeImageRequest'
		[2017-03-08 18:04:22] [World             ] 	[string "lua\World\ImageGetter.lua"]:41: in function <[string "lua\World\ImageGetter.lua"]:37>
		[2017-03-08 18:04:22] [World             ] 	[string "lua\MessagePump.lua"]:35: in function 'onWindowMessage'
		[2017-03-08 18:04:22] [World             ] 	[string "lua\World\World.lua"]:70: in function 'start'
		[2017-03-08 18:04:22] [World             ] 	[string "lua\World\WorldThread.lua"]:14: in function <[string "lua\World\WorldThread.lua"]:5>
		*/
		auto f = s.find("[string \"lua\\", 0);
		if (f > 0) {
			auto f2 = s.find("\"]", f);
			p = s.substr(f + 13, f2 - f - 17);

			auto f3 = s.find(":", f2 + 3);
			std::string p2 = s.substr(f2 + 3, f3 - f2 - 3);

			char buf[1024];
			sprintf(buf, "[%-24s%4s] ", p.c_str(), p2.c_str());
			p = buf;
		}
	}

	switch (log_file) {
	case 0:
		_lprint(p + arg);
		break;
	case 1:
		_lprint(p + arg);
		_eprint(p + arg);
		break;
	}
	return 0;
}

static int luaC_eprint(lua_State *L) {

	// lua_check_args(L, );

	return _lua_print(L, 1);
}

static int luaC_lprint(lua_State *L) {
	return _lua_print(L, 0);
}

static int luaC_executeFile(lua_State *L) {

	std::string filename = std::string("lua\\") + lua_tostring(L, -1);
	char *b; int sz;	
	File *f = Files_get(&filename.c_str()[4]);
	char *saveptr = NULL;
	if (f) {
		lprint_LS_EXECUTE_FILE(filename + " from data.pack");
		b = f->_data;
		saveptr = b;
		sz = f->_len;

		char *tb = new char[sz + 1];
		memcpy(tb, b, sz);
		tb[sz] = 0;
		b = tb;
	}
	else {
		lprint_LS_EXECUTE_FILE(filename);

		FILE *f;
		fopen_s(&f, filename.c_str(), "rb");
		if (f == NULL) {
			char buff[1024];
			sprintf(buff, "luaC_executeFile: Can't open file '%s'", &filename.c_str()[4]);
			_lua_error(L, buff);
			return 0;
		}

		fseek(f, 0, SEEK_END);
		sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		b = new char[sz + 1];
		fread(b, sz, 1, f);
		b[sz] = 0;

		fclose(f);
	}

	lua_getglobal(L, "__error_string");
	std::string *lastError = (std::string *)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getglobal(L, "__CClass");
	LuaScript *s = (LuaScript *)lua_touserdata(L, -1);
	lua_pop(L, 1);

	for (int i = 0; i < s->_pp_pos; i++) {

		LuaStackGuard stackGuard(L);

		int ref = s->_pp[i];

		(*lastError) = "";
		lua_pushcfunction(L, errorHandler);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// lprint_LS_EXECUTE_FILE("text len " + inttostr(sz));
		lua_pushstring(L, &filename.c_str()[4]);
		lua_pushstring(L, b);

		lprint_LS_EXECUTE_FILE("before preprocessor");
		if (lua_pcall(L, 2, LUA_MULTRET, -4)) {
			eprint(*lastError);
		}
		else {
			lprint_LS_EXECUTE_FILE("after preprocessor");
			char *b2 = (char *)lua_tostring(L, -1);
			if(b != saveptr) delete b;
			sz = strlen(b2);
			b = new char[sz + 1];
			memcpy(b, b2, sz + 1);
			// lprint_LS_EXECUTE_FILE("text len " + inttostr(sz));
			// lprint(b);
		}

	}


	(*lastError) = "";
	lua_pushcfunction(L, errorHandler);

	if (luaL_loadbuffer(L, b, sz, filename.c_str())) {
		const char *err = lua_tostring(L, -1);
		lprint(std::string("Syntax error: ") + err);
		return luaL_error(L, "%s", err);
	}

	if (lua_pcall(L, 0, LUA_MULTRET, -2)) {
		const char *err = lua_tostring(L, -1);
		lprint(std::string("Executon error: ") + err + (*lastError));		
		return 1;
	}

	if (b != saveptr) delete b;

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
		eprint("Error can't find module " + moduleName);
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


static int luaC_InstallPreProcessor(lua_State *L) {

	LuaStackGuard stackGuard(L);

	lua_insert(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_getglobal(L, "__CClass");
	LuaScript *s = (LuaScript *)lua_touserdata(L, -1);
	lua_pop(L, 1);

	s->_pp[s->_pp_pos] = ref;
	s->_pp_pos ++;


	return 0;
}

void luaL_requiref(lua_State *L, const char *name, lua_CFunction func, int i) {
	lua_pushcfunction(L, func);
	lua_pushstring(L, name);
	lua_call(L, 1, 0);
}

LuaScript::LuaScript() : _returnValue(NULL) {

	_pp_pos = 0;

	_l = luaL_newstate();
	if (luaJIT_setmode(_l, 0, LUAJIT_MODE_ENGINE) == 0) {
		lprint("luaJIT_setmode error");
	}

	std::string *_error = new std::string;
	lua_pushlightuserdata(_l, _error);
	lua_setglobal(_l , "__error_string");

	lua_pushlightuserdata(_l, this);
	lua_setglobal(_l, "__CClass");

	luaL_requiref(_l, "_G", luaopen_base, 1);
	luaL_requiref(_l, LUA_TABLIBNAME, luaopen_table, 1);

	luaL_requiref(_l, LUA_MATHLIBNAME, luaopen_math, 1);
	luaL_requiref(_l, LUA_STRLIBNAME, luaopen_string, 1);
	
	luaL_requiref(_l, LUA_DBLIBNAME, luaopen_debug, 1);
	lua_register(_l, "lprint", luaC_lprint);
	lua_register(_l, "eprint", luaC_eprint);

	lua_register(_l, "C_ExecuteFile", luaC_executeFile);
	lua_register(_l, "C_InstallModule", luaC_InstallModule);

	lua_register(_l, "C_PackTable", luaC_PackTable);
	lua_register(_l, "C_UnpackTable", luaC_UnpackTable);

	lua_register(_l, "C_Exit", luaC_Exit);

	lua_register(_l, "C_InstallPreProcessor", luaC_InstallPreProcessor);
	

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

	lua_getglobal(_l , "__error_string");
	std::string *lastError = (std::string *)lua_touserdata(_l, -1);
	lua_pop(_l, 1);

	(*lastError) = "";
	lua_pushcfunction(_l, errorHandler);

	lua_getglobal(_l, func.c_str());
	if (lua_isfunction(_l, -1) == 0) {
		(*lastError) = std::string("Error: [") + _filepath + "] can't find function " + func;
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

	lua_getglobal(_l, "__error_string");
	std::string *lastError = (std::string *)lua_touserdata(_l, -1);
	lua_pop(_l, 1);

	(*lastError) = "";
	lua_pushcfunction(_l, errorHandler);

	lua_getglobal(_l, object.c_str());
	if (lua_istable(_l, -1) == 0) {
		(*lastError) = std::string("Error: [") + _filepath + "] can't find object " + object;
		return 1;
	}

	lua_getfield(_l, -1, method.c_str());
	if (lua_isfunction(_l, -1) == 0) {
		(*lastError) = std::string("Error: [") + _filepath + "] can't execute " + object + ":" + method;
		return 1;
	}

	lua_remove(_l, -2);

	lua_getglobal(_l, object.c_str());
	if (lua_istable(_l, -1) == 0) {
		(*lastError) = std::string("Error: [") + _filepath + "] can't find object " + object;
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
	lua_getglobal(_l, "__error_string");
	std::string *lastError = (std::string *)lua_touserdata(_l, -1);
	lua_pop(_l, 1);

	return *lastError;
}

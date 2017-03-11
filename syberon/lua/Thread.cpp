#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"
#include "..\Utils.h"

#include "..\logConfig.h"
#ifdef _LOG_LUA_THREAD
#define lprint_THREAD(text) lprint(text)
#else
#define lprint_THREAD(text)
#endif


typedef struct {

	std::string *filepath;
	DataList *list;

} LuaThreadParams;

void MiniDumpFunction(unsigned int nExceptionCode, EXCEPTION_POINTERS *pException);

DWORD WINAPI LuaThreadFunction(LPVOID lpParam) {

	std::string filepath;
	_set_se_translator(MiniDumpFunction);
	try  // this try block allows the SE translator to work
	{
		auto p = (LuaThreadParams *)lpParam;
		filepath = *p->filepath + ".lua";
		Logger::setThreadName(filepath.c_str());

		lprint_THREAD(filepath);
		LuaScript *L = new LuaScript();

		if (L->executeFile(filepath)) {
			eprint("Error in file " + filepath)
			eprint(L->getError());
			return 1;
		}

		if (L->executeFunction("thread", p->list)) {
			eprint("Error in function 'thread' in file " + filepath);
			eprint(L->getError());
			return 1;
		}

		delete L;
		delete p->filepath;
		delete p;

		lprint_THREAD("finish " + filepath);
		return 0;
	}
	catch (...)
	{
		eprint("catch ... in " + filepath);
		return -1;
	}
}

static int luaC_Thread_New(lua_State *L) {

	auto c = lua_gettop(L);
		
	auto filepath = lua_tostring(L, 1);

	auto p = new LuaThreadParams;
	p->list = DL;
	p->filepath = new std::string(filepath);

	// read params
	int pos = 2;
	while (c > 1) {
		p->list->add(L, pos);
		pos++;
		c--;
	}

	HANDLE ht = CreateThread(NULL, 0, LuaThreadFunction, p, 0, NULL);
	auto threadId = GetThreadId(ht);
	lua_pushinteger(L, threadId);

	lprint_THREAD("threadId " + inttostr(threadId));

	return 1;
}


static int luaC_Thread_PostMessage(lua_State *L) {

	auto messageId = lua_tointeger(L, 2);

	auto wparam = lua_tointeger(L, 4);
	if (lua_isuserdata(L, 4)) {
		auto ud = (UserData *)lua_touserdata(L, 4);
		wparam = (_int)ud->data;
	}

	auto lparam = lua_tointeger(L, 3);
	if (lua_isuserdata(L, 3)) {
		auto ud = (UserData *)lua_touserdata(L, 3);
		lparam = (_int)ud->data;
	}

	if (lua_isnumber(L, 1)) {
		auto threadId = lua_tointeger(L, 1);
		BOOL r = PostThreadMessage((DWORD)threadId, messageId, wparam, lparam);
		if (r == 0) {
			auto e = GetLastError();
			// ERROR_INVALID_THREAD_ID 1444
			// ERROR_NOT_ENOUGH_QUOTA 1816
			// eprint("Error thread " + inttostr(e));
			lua_pushboolean(L, TRUE);
		}
		else {
			lua_pushboolean(L, FALSE);
		}
	}
	else if (lua_isuserdata(L, 1)) {
		auto *ud = (UserData *)lua_touserdata(L, 1);
		BOOL r = PostMessage((HWND)ud->data, messageId, wparam, lparam);
		if (r == 0) {
			auto e = GetLastError();
			// eprint("Error hwnd " + inttostr(e));
			lua_pushboolean(L, TRUE);
		}
		else {
			lua_pushboolean(L, FALSE);
		}
	}

	return 1;
}

_int _messageId, _lparam, _wparam;

static int luaC_Thread_PeekMessage(lua_State *L) {

	MSG msg = { 0 };
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

		_messageId = msg.message;
		_lparam = msg.lParam;
		_wparam = msg.wParam;

		lua_pushboolean(L, TRUE);
	}
	else {
		lua_pushboolean(L, FALSE);
	}

	return 1;
}

static int luaC_Thread_GetMessage(lua_State *L) {

	MSG msg = { 0 };
	if (GetMessage(&msg, NULL, 0, 0)) {

		_messageId = msg.message;
		_lparam = msg.lParam;
		_wparam = msg.wParam;

		lua_pushboolean(L, TRUE);
	}
	else {
		lua_pushboolean(L, FALSE);
	}

	return 1;
}

static int luaC_Thread_GetMessageId(lua_State *L) {
	lua_pushinteger(L, _messageId);
	return 1;
}
static int luaC_Thread_GetLParam(lua_State *L) {
	lua_pushinteger(L, _lparam);
	return 1;
}
static int luaC_Thread_GetWParam(lua_State *L) {
	lua_pushinteger(L, _wparam);
	return 1;
}
static int luaC_Thread_SetName(lua_State *L) {
	Logger::setThreadName(lua_tostring(L, 1));
	return 0;
}

void lm_Thread_install(lua_State* _l) {

	lua_register(_l, "C_Thread_New", luaC_Thread_New);

	lua_register(_l, "C_Thread_PostMessage", luaC_Thread_PostMessage);	

	lua_register(_l, "C_Thread_PeekMessage", luaC_Thread_PeekMessage);
	lua_register(_l, "C_Thread_GetMessage", luaC_Thread_GetMessage);

	lua_register(_l, "C_Thread_GetMessageId", luaC_Thread_GetMessageId);
	lua_register(_l, "C_Thread_GetLParam", luaC_Thread_GetLParam);
	lua_register(_l, "C_Thread_GetWParam", luaC_Thread_GetWParam);

	lua_register(_l, "C_Thread_SetName", luaC_Thread_SetName);		
}

LuaModule *lm_Thread = new LuaModule("thread", lm_Thread_install);



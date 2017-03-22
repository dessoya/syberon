#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class CQueue {
private:
	std::queue<LuaArgumants *> the_queue;
	mutable boost::mutex the_mutex;
	boost::condition_variable the_condition_variable;
	volatile bool _empty;
public:

	CQueue::CQueue() : _empty(true) { }

	/*
	void push(Data const& data)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		the_queue.push(data);
		lock.unlock();
		the_condition_variable.notify_one();
	}

	bool empty() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	bool try_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		if (the_queue.empty())
		{
			return false;
		}

		popped_value = the_queue.front();
		the_queue.pop();
		return true;
	}

	void wait_and_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		while (the_queue.empty())
		{
			the_condition_variable.wait(lock);
		}

		popped_value = the_queue.front();
		the_queue.pop();
	}
	*/

	void send(LuaArgumants *a) {
		boost::mutex::scoped_lock lock(the_mutex);
		the_queue.push(a);
		_empty = false;
		// lock.unlock();
		// the_condition_variable.notify_one();
	}

	LuaArgumants *get() {

		boost::mutex::scoped_lock *lock = NULL;
		LuaArgumants *a = NULL;

		while (true) {
			boost::thread::yield();

			if (!_empty) {
				lock = new boost::mutex::scoped_lock(the_mutex);
				if (!_empty) {

					a = the_queue.front();
					the_queue.pop();
					_empty = the_queue.empty();
					delete lock;
					lock = NULL;
					break;
				}
				delete lock;
				lock = NULL;

			}

			Sleep(1);
		}

		if (lock) {
			delete lock;
		}

		return a;
	}

	bool empty() {
		// boost::mutex::scoped_lock lock(the_mutex);
		return _empty;
	}
};

static int luaC_CQueue_new(lua_State *L) {

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));

	ud->type = UDT_CQueue;
	ud->data = new CQueue;

	return 1;
}

static int luaC_CQueue_send(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto queue = (CQueue *)ud->data;

	LuaArgumants *a = new LuaArgumants(L, 1);
	queue->send(a);

	return 0;
}

static int luaC_CQueue_get(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto queue = (CQueue *)ud->data;

	auto a = queue->get();
	int c = a->push(L);
	delete a;
	return c;
}

static int luaC_CQueue_empty(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto queue = (CQueue *)ud->data;

	auto e = queue->empty();
	lua_pushboolean(L, e);
	return 1;
}


void lm_CQueue_install(lua_State* _l) {

	lua_register(_l, "C_CQueue_new", luaC_CQueue_new);
	lua_register(_l, "C_CQueue_send", luaC_CQueue_send);
	lua_register(_l, "C_CQueue_get", luaC_CQueue_get);
	lua_register(_l, "C_CQueue_empty", luaC_CQueue_empty);

}

LuaModule *lm_CQueue = new LuaModule("cqueue", lm_CQueue_install);

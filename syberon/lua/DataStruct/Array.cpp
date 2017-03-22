#include "windows.h"
#include "..\..\Logger.h"
#include "..\..\LuaClass.h"

class ArrayItem {
public:
	unsigned long long type : 8, len : 24;
	union {
		void *data;
		double d;
		long long i;
		const char *str;
	} d1;
	/*
	union {
		size_t len;
	} d2;
	*/
};

char typebuf[1024];
const char *_Array_type_to_str(unsigned char t) {
	switch (t) {
	case UDT_Nil: return "nil";
	case UDT_Int: return "integer";
	case UDT_Double: return "double";
	case UDT_String: return "string";
	case UDT_DataStruct: {
		
		return "datastruct";
	}
	}
	return "unknown";
}

const char *_ArrayItem_to_str(ArrayItem *d) {
	switch (d->type) {
	case UDT_Nil: return "";
	case UDT_Int: {
		sprintf(typebuf, "%d", d->d1.i);
		return typebuf;
	}
	case UDT_Double: {
		sprintf(typebuf, "%f", d->d1.d);
		return typebuf;
	}
	case UDT_String: {
		memcpy(typebuf, d->d1.str, d->len);
		typebuf[d->len] = 0;
		return typebuf;
	}
	case UDT_DataStruct: {
		return "";
	}
	}
	return "unknown";
}


LuaClass(Array)

	int _size, _count;
	ArrayItem *_data;


	Array(lua_State *L) : LuaDataStruct() {
		_count = 0;
		if (lua_gettop(L)) {
			_size = lua_tointeger(L, 1);
		}
		else {
			_size = 8;
		}
		if (_size < 8) _size = 8;

		_data = new ArrayItem[_size];
	}

	~Array() {
		lprint("id " + inttostr(_id));
		LuaDataStruct *ds;
		auto p = _data;
		for (int i = 0; i < _count; i++, p++) {
			switch (p->type) {
			case UDT_String:
				delete p->d1.str;
				break;
			case UDT_DataStruct:
				ds = (LuaDataStruct *)p->d1.data;
				lprint("remove ds " + inttostr(ds->_id) + " " + inttostr(ds->_share));
				if (ds->_share == 0) {
					delete ds;
				}
				ds->_share--;
				break;
			}
		}
		delete _data;
	}

	LuaMethod(Array, len) {
		lua_pushinteger(L, _count);
		return 1;
	}

	LuaMethod(Array, size) {
		lua_pushinteger(L, _size);
		return 1;
	}

	LuaMethod(Array, get) {
		auto t = lua_gettop(L);
		int c, p;
		if (t == 0) {
			p = 0;
			c = _count;
		}
		else {
			if (t > 1) {
				c = lua_tointeger(L, 2);
			}
			else {
				c = 1;
			}
			p = lua_tointeger(L, 1);
		}

		UserData *ud;
		LuaDataStruct *ds;

		// todo: check for bound
		auto d = &_data[p];
		for (int i = p, l = p + c; i < l; i++, d++) {
			switch (d->type) {
			case UDT_Nil:
				lua_pushnil(L);
				break;
			case UDT_Int:
				lua_pushinteger(L, d->d1.i);
				break;
			case UDT_Double:
				lua_pushnumber(L, d->d1.d);
				break;
			case UDT_String:
				lua_pushlstring(L, d->d1.str, d->len);
				break;
			case UDT_DataStruct:
				ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
				ud->type = UDT_DataStruct;
				ud->data = d->d1.data;
				ds = (LuaDataStruct *)ud->data;
				ds->_share++;
				ds->registerInNewState(L);

			}
		}

		return c;
	}

	LuaMethod(Array, push) {
		return _helper_set(L, 1, 1, _count);
	}

	LuaMethod(Array, set) {
		auto t = lua_gettop(L);
		auto c = t - 1;
		auto p = lua_tointeger(L, 1);

		return _helper_set(L, 2, c, p);
	}

	int _helper_set(lua_State *L, int index, int count, int setPosition) {

		/*
		auto t = lua_gettop(L);
		auto c = t - 1;
		auto p = lua_tointeger(L, 1);
		*/

		auto newSize = setPosition + count;
		if (newSize > _size) {
			// realoc
			newSize = (newSize / 8 + (newSize % 8 ? 1 : 0)) * 8;
			ArrayItem *i = new ArrayItem[newSize];
			if (_count) {
				memcpy(i, _data, sizeof(ArrayItem) * _count);
			}
			delete _data;
			_data = i;
			_size = newSize;
		}

		if (_count < setPosition) {
			auto d = &_data[_count];
			for (int i = _count; i < setPosition; i++, d++) {
				d->type = UDT_Nil;
			}
		}

		auto l = setPosition + count;
		if (l > _count) {
			_count = l;
		}

		// let's load
		size_t sz;
		const char *s;
		UserData *ud;
		long long ni;
		double nd;

		auto d = &_data[setPosition];
		for (int i = index, ie = index + count; i < ie; i++, d++) {
			auto tp = lua_type(L, i);
			switch (tp) {
				// LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
			case LUA_TNUMBER:
				ni = lua_tointeger(L, i);
				nd = lua_tonumber(L, i);
				if( ni == nd ) {
					d->type = UDT_Int;
					d->d1.i = ni;
				}
				else {
					d->type = UDT_Double;
					d->d1.d = nd;
				}
				break;
			case LUA_TSTRING:
				d->type = UDT_String;
				s = lua_tolstring(L, i, &sz);
				d->d1.str = new char[sz];
				memcpy((void *)d->d1.str, s, sz);
				d->len = sz;
				break;
			case LUA_TUSERDATA:
				ud = (UserData *)lua_touserdata(L, i);
				d->type = ud->type;
				d->d1.data = ud->data;
				if (d->type == UDT_DataStruct) {
					((LuaDataStruct *)ud->data)->_share++;
				}
				break;
			}
		}

		return 0;
	}

	LuaMethod(Array, dump) {
		lprint("dump Array id " + inttostr(_id));
		lprint("size " + inttostr(_size) + ", count " + inttostr(_count));
		auto d = _data;
		for (int i = 0; i < _count; i++, d++) {
			lprint("[" + inttostr(i) + "] type " + _Array_type_to_str(d->type) + ":" + _ArrayItem_to_str(d));
		}
		return 0;
	}

	/*
	LuaMethod(Array, a) {
		lprint(lua_tostring(L, 1));

		return 0;
	}

	LuaMethod(Array, __tostring) {
		lprint("");
		lua_pushstring(L, "Array");
		return 1;
	}
	*/
};

LuaMethods(Array) = {
	LuaMethodDesc(Array, set),
	LuaMethodDesc(Array, push),
	LuaMethodDesc(Array, get),
	LuaMethodDesc(Array, len),
	LuaMethodDesc(Array, size),
	LuaMethodDesc(Array, dump),
	{ 0,0 }
};

LuaMetaMethods(Array) = {
	// LuaMethodDesc(Array, __tostring),
	{ 0,0 }
};


void lm_DS_Array_install(lua_State* _l) {

	// lprint(inttostr(sizeof(ArrayItem)));
	LuaClass<Array>::Register(_l);
}

LuaModule *lm_DS_Array = new LuaModule("ds_array", lm_DS_Array_install);

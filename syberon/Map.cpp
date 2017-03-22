#include "Map.h"

#include "string.h"
#include "Logger.h"
#include "Utils.h"

#include "logConfig.h"
#ifdef _LOG_MAP
#define lprint_MAP(text) lprint(text)
#else
#define lprint_MAP(text)
#endif

#define _1 ((unsigned long long)1)
#define LB_SIZE (_1 << B_BITS)

LastBlock::LastBlock(lint x, lint y) : _x(x), _y(y) {

	_ids = NULL;
	_count = 0;
	_size = 0;

	_block = new CellID[1 << (B_BITS + B_BITS)];
	memset(_block, 0, (1 << (B_BITS + B_BITS)) * sizeof(CellID));
}

#define IDS_START_COUNT 8

void LastBlock::addID(lint id) {

	if (_ids == NULL) {
		_ids = new lint[IDS_START_COUNT];
		_size = IDS_START_COUNT;
	}

	if (_count == _size) {
		// make x2
		_size <<= 1;
		lint *ids = new lint[_size];
		memcpy(ids, _ids, sizeof(lint) * _count);
		delete _ids;
		_ids = ids;
	}

	_ids[_count] = id;
	_count++;
}

void LastBlock::delID(lint id) {

	lint *p = _ids;
	for (int i = 0; i < _count; i++, p++) {
		if (*p == id) {
			int last = _count - 1;
			if (_count > 1 && i != last) {
				*p = _ids[last];
			}
			_count--;
			return;
		}
	}

}

/*
int intpow(int x, int y) {
	int z = 1;
	int base = x;
	for (;;) {
		if (y & 1) {
			z *= base;
		}
		y >>= 1;
		if (y == 0) {
			break;
		}
		base *= base;
	}
	return z;
}
*/

Block::Block(lint level, lint x, lint y) : _level(level), _x(x), _y(y) {

	_m = level * B_BITS;

	if (_level == 1) {
		_lblock = new PLastBlock[1 << (B_BITS + B_BITS)];
		memset(_lblock, 0, (1 << (B_BITS + B_BITS)) * sizeof(PLastBlock));
	}
	else {
		_block = new PBlock[1 << (B_BITS + B_BITS)];
		memset(_block, 0, (1 << (B_BITS + B_BITS)) * sizeof(PBlock));
	}
}

Block *Block::getBlock(lint x, lint y) {

	/*
	{
		long long x1 = _x - MAP_MID;
		long long y1 = _y - MAP_MID;
		if (_x < MAP_MID) {
			x1 = (long long)(MAP_MID - _x) * -1;
		}
		if (_y < MAP_MID) {
			y1 = (long long)(MAP_MID - _y) * -1;
		}

		auto d = _1 << (_m + B_BITS);

		long long x2 = x1 + d;
		long long y2 = y1 + d;


		lprint_MAP(inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(x1) + "x" + inttostr(y1) + " " + inttostr(x2) + "x" + inttostr(y2));
	}
	*/

	lint x1 = (lint(x - _x)) >> (lint)_m;
	lint y1 = (lint(y - _y)) >> (lint)_m;
/*
	lint x1 = (lint(x - _x));
	x1 /= (_1 << (lint)_m);
	lint y1 = (lint(y - _y));
	y1 /= (_1 << (lint)_m);
*/
	// lprint_MAP(inttostr(_1 << _m) + " " + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(x1) + "x" + inttostr(y1));
	return _block[x1 + (y1 << B_BITS)];
}

Block *Block::addBlock(lint x, lint y) {

	auto x1 = (x - _x) >> _m;
	auto y1 = (y - _y) >> _m;

	lprint_MAP(inttostr(_1 << _m) + " " + inttostr(x1) + "x" + inttostr(y1));

	if (_level == 1) {
		return (Block *)(_lblock[x1 + (y1 << B_BITS)] = new LastBlock((x1 << _m) + _x, (y1 << _m) + _y));
	}
	return _block[x1 + (y1 << B_BITS)] = new Block(_level - 1, (x1 << _m) + _x, (y1 << _m) + _y);
}

void Block::dump(std::string *l) {
	if (l == NULL) {
		l = new std::string;
	}

	long long x1 = _x - MAP_MID;
	long long y1 = _y - MAP_MID;
	if (_x < MAP_MID) {
		x1 = (long long)(MAP_MID - _x) * -1;
	}
	if (_y < MAP_MID) {
		y1 = (long long)(MAP_MID - _y) * -1;
	}

	lprint_MAP((*l) + "level " + inttostr(_level) + " m " + inttostr(_m) + " x " + inttostr(x1) + " y " + inttostr(y1) + " size " + inttostr(x1 + (_1 << (_m + B_BITS))) + "x" + inttostr(y1 + (_1 << (_m + B_BITS))));

	std::string *ln = new std::string(*l);
	(*ln) += "  ";

	if (_level == 1) {
		lprint_MAP((*l) + "ids");
		int d = _1 << B_BITS;
		for (int y = 0; y < d; y++) {
			for (int x = 0; x < d; x++) {
				auto b = _lblock[x + (y << B_BITS)];
				if (b && b->_count) {
					lprint_MAP((*l) + "cell x " + inttostr(x) + " y " + inttostr(y));
					for (int i = 0; i < b->_count; i++) {
						lprint_MAP((*ln) + inttostr(b->_ids[i]));
					}
				}
			}
		}
		return;
	}
	
	int d = _1 << B_BITS;
	for (int y = 0; y < d; y++) {
		for (int x = 0; x < d; x++) {
			auto b = _block[x + (y << B_BITS)];
			if (b) {
				lprint_MAP((*l) + "cell x " + inttostr(x) + " y " + inttostr(y));
				b->dump(ln);
			}
		}
	}
}


// cells of basic map
// LB_SIZE * B_SIZE
// middle point = LB_SIZE / 2

Map::Map() {
	_level = 1;

	// auto d = (1 << (B_BITS - 1 + B_BITS));

	_root = new Block(_level, MAP_MID, MAP_MID);
	_m = B_BITS + B_BITS;
	
}

void Map::dump() {
	lprint_MAP("\n-------------------\n");
	lprint_MAP("B_BITS " + inttostr(B_BITS) + " size " + inttostr(_1 << B_BITS));
	_root->dump();
	lprint_MAP("\n-------------------\n");
}

void Map::trace_coords(lint x, lint y) {

	x += MAP_MID;
	y += MAP_MID;
	// level
	
	auto level = _level;
	auto b = _root;
	auto m = _m;

	auto d = b->_x + (_1 << m);

	if (b->_x > x || b->_y > y || d <= x || d <= y) {
		lprint_MAP("out of map " + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr(d));
		return;
	}

	lprint_MAP("map bound " + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr(d));

	while (level > 0) {


		auto bx = (x - b->_x) >> _m;
		auto by = (y - b->_y) >> _m;

		lprint_MAP("level " + inttostr(level) + " bx " + inttostr(bx - MAP_MID) + " by " + inttostr(by - MAP_MID));

		level--;
	}

}

void Map::setCell(long long __x, long long __y, CellID id) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	auto level = _level;
	auto b = _root;
	auto m = _m;

	auto dx = b->_x + (_1 << m);
	auto dy = b->_y + (_1 << m);

	if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
		// lprint_MAP(std::string("out of map ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID));
		return;
	}

	while (level > 0 && b) {

		// lprint(std::string("level ") + inttostr(level) + " bx " + inttostr(bx) + " by " + inttostr(by));
		b = b->getBlock(x, y);

		level--;
	}

	if (b == NULL) {
		return;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;

	/*
	long long x1 = x - l->_x;
	long long y1 = y - l->_y;

	if (x1 < MAP_MID) {
		x1 = (long long)(MAP_MID - x1) * -1;
	}
	else {
		x1 = x1 - MAP_MID;
	}

	if (y1 < MAP_MID) {
		y1 = (long long)(MAP_MID - y1) * -1;
	}
	else {
		y1 = y1 - MAP_MID;
	}


	lprint_MAP(std::string("last block ") + inttostr(l->_x - MAP_MID) + "x" + inttostr(l->_y - MAP_MID) + " " + inttostr(bx) + "x" + inttostr(by));
	*/

	l->_block[bx + (by << B_BITS)] = id;
}


CellID Map::getCell(long long __x, long long __y) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	auto level = _level;
	auto b = _root;
	auto m = _m;

	auto dx = b->_x + (_1 << m);
	auto dy = b->_y + (_1 << m);

	if (b->_x > x || b->_y > y || dx <= x || dy <= y) {

		/*
		long long x1 = x - l->_x;
		long long y1 = y - l->_y;

		if (x1 < MAP_MID) {
			x1 = (long long)(MAP_MID - x1) * -1;
		}
		else {
			x1 = x1 - MAP_MID;
		}

		if (y1 < MAP_MID) {
			y1 = (long long)(MAP_MID - y1) * -1;
		}
		else {
			y1 = y1 - MAP_MID;
		}
		*/

		// lprint(std::string("out of map ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr((void *)d));
		return ABSENT_CELL;
	}

	while (level > 0 && b) {

		// lprint(std::string("level ") + inttostr(level) + " bx " + inttostr(bx) + " by " + inttostr(by));
		b = b->getBlock(x, y);

		level--;
	}

	if (b == NULL) {
		return ABSENT_CELL;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;

	/*

	long long x1 = x - l->_x;
	long long y1 = y - l->_y;

	if (x1 < MAP_MID) {
		x1 = (long long)(MAP_MID - x1) * -1;
	}
	else {
		x1 = x1 - MAP_MID;
	}

	if (y1 < MAP_MID) {
		y1 = (long long)(MAP_MID - y1) * -1;
	}
	else {
		y1 = y1 - MAP_MID;
	}


	lprint(std::string("last block ") + inttostr(l->_x - MAP_MID) + "x" + inttostr(l->_y - MAP_MID) + " " + inttostr(bx) + "x" + inttostr(by));
	*/

	return l->_block[bx + (by << B_BITS)];
}


void Map::addSetFlags(long long __x, long long __y, CellID flags) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	lint level = _level;
	auto b = _root;
	lint m = _m;


	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
			// lprint_MAP("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;


	auto pos = bx + (by << B_BITS);
	auto id = l->_block[pos];
	l->_block[pos] = (id & 0xfff) | (flags << 12);

}


void Map::addSetCell(long long __x, long long __y, CellID id) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	lint level = _level;
	auto b = _root;
	lint m = _m;


	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		/*
		long long x1 = b->_x - MAP_MID;
		long long y1 = b->_y - MAP_MID;
		if (b->_x < MAP_MID) {
		x1 = (long long)(MAP_MID - b->_x) * -1;
		}
		if (b->_y < MAP_MID) {
		y1 = (long long)(MAP_MID - b->_y) * -1;
		}

		long long x2 = dx - MAP_MID;
		if (dx < MAP_MID) {
		(long long)(MAP_MID - dx) * -1;
		}
		long long y2 = dy - MAP_MID;
		if (dy < MAP_MID) {
		(long long)(MAP_MID - dy) * -1;
		}
		lprint_MAP("check bound level " + inttostr(level) + " " + inttostr(x1) + "x" + inttostr(y1) + " " + inttostr(x2) + "x" + inttostr(y2));
		*/

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
			// lprint_MAP("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		/*
		auto bx = (x - b->_x) >> m;
		auto by = (y - b->_y) >> m;

		lprint_MAP(std::string("level ") + inttostr(level) + " bx " + inttostr(bx) + " by " + inttostr(by));
		*/
		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;

	/*
	long long x1 = x - l->_x;
	long long y1 = y - l->_y;

	if (x1 < MAP_MID) {
	x1 = (long long)(MAP_MID - x1) * -1;
	}
	else {
	x1 = x1 - MAP_MID;
	}

	if (y1 < MAP_MID) {
	y1 = (long long)(MAP_MID - y1) * -1;
	}
	else {
	y1 = y1 - MAP_MID;
	}


	lprint_MAP(std::string("last block ") + inttostr(l->_x - MAP_MID) + "x" + inttostr(l->_y - MAP_MID) + " " + inttostr(bx) + "x" + inttostr(by));
	*/

	l->_block[bx + (by << B_BITS)] = id;


}

void Map::addSetIfEmptyAndId(long long __x, long long __y, CellID id, CellID id2) {

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	lint level = _level;
	auto b = _root;
	lint m = _m;


	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
			// lprint_MAP("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;

	int p = bx + (by << B_BITS);

	if (l->_block[p] == 0 || l->_block[p] == id2)
		l->_block[p] = id;


}


void Map::addSetIfEmpty(long long __x, long long __y, CellID id) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	lint level = _level;
	auto b = _root;
	lint m = _m;


	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		/*
		long long x1 = b->_x - MAP_MID;
		long long y1 = b->_y - MAP_MID;
		if (b->_x < MAP_MID) {
		x1 = (long long)(MAP_MID - b->_x) * -1;
		}
		if (b->_y < MAP_MID) {
		y1 = (long long)(MAP_MID - b->_y) * -1;
		}

		long long x2 = dx - MAP_MID;
		if (dx < MAP_MID) {
		(long long)(MAP_MID - dx) * -1;
		}
		long long y2 = dy - MAP_MID;
		if (dy < MAP_MID) {
		(long long)(MAP_MID - dy) * -1;
		}
		lprint_MAP("check bound level " + inttostr(level) + " " + inttostr(x1) + "x" + inttostr(y1) + " " + inttostr(x2) + "x" + inttostr(y2));
		*/

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
			// lprint_MAP("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		/*
		auto bx = (x - b->_x) >> m;
		auto by = (y - b->_y) >> m;

		lprint_MAP(std::string("level ") + inttostr(level) + " bx " + inttostr(bx) + " by " + inttostr(by));
		*/
		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

	LastBlock *l = (LastBlock *)b;

	auto bx = x - l->_x;
	auto by = y - l->_y;

	/*
	long long x1 = x - l->_x;
	long long y1 = y - l->_y;

	if (x1 < MAP_MID) {
	x1 = (long long)(MAP_MID - x1) * -1;
	}
	else {
	x1 = x1 - MAP_MID;
	}

	if (y1 < MAP_MID) {
	y1 = (long long)(MAP_MID - y1) * -1;
	}
	else {
	y1 = y1 - MAP_MID;
	}


	lprint_MAP(std::string("last block ") + inttostr(l->_x - MAP_MID) + "x" + inttostr(l->_y - MAP_MID) + " " + inttostr(bx) + "x" + inttostr(by));
	*/

	int p = bx + (by << B_BITS);

	if (l->_block[p] == 0)
		l->_block[p] = id;

}




void Map::addBlock(long long __x, long long __y) {

	// lprint_MAP("x " + inttostr(__x) + " y " + inttostr(__y));

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);
 
	lint level = _level;
	auto b = _root;
	lint m = _m;

	
	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {
			// lprint_MAP("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));
			
			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

}


LastBlock *Map::getLastBlock(long long __x, long long __y, bool _add) {

	if (__x < 0) {
		__x /= 1024;
		__x--;
	}
	else {
		__x /= 1024;
	}

	if (__y < 0) {
		__y /= 1024;
		__y--;
	}
	else {
		__y /= 1024;
	}

	lint x = (__x + MAP_MID);
	lint y = (__y + MAP_MID);

	lint level = _level;
	auto b = _root;
	lint m = _m;

	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {

			if (!_add) {
				return NULL;
			}

			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			if (!_add) {
				return NULL;
			}

			b = bb->addBlock(x, y);
		}

		level--;
	}

	return (LastBlock *)b;
}

LastBlock *Map::_getLastBlock(long long __x, long long __y, bool _add) {

	auto x = __x, y = __y;
	lint level = _level;
	auto b = _root;
	lint m = _m;

	while (true) {
		lint dx = b->_x + (_1 << m);
		lint dy = b->_y + (_1 << m);

		if (b->_x > x || b->_y > y || dx <= x || dy <= y) {

			if (!_add) {
				return NULL;
			}

			level++;
			_level++;

			auto e = (_1 << ((B_BITS * level) - 1 + B_BITS));

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, MAP_MID - e, MAP_MID - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS - 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			if (!_add) {
				return NULL;
			}

			b = bb->addBlock(x, y);
		}

		level--;
	}

	return (LastBlock *)b;
}

LastBlock *Map::addID(long long __x, long long __y, lint id) {
	LastBlock *b = getLastBlock(__x, __y, true);
	b->addID(id);
	return b;
}

void Map::delID(long long __x, long long __y, lint id) {
	LastBlock *b = getLastBlock(__x, __y);
	if (b) {
		b->delID(id);
	}
}



MapPointer *Map::getPointer(long long __x, long long __y) {
	MapPointer *m = new MapPointer(this, __x, __y);
	return m;
}


MapPointer::MapPointer(Map *m, long long __x, long long __y) {
	_m = m;
	_x = __x + MAP_MID;
	_y = __y + MAP_MID;
	_l = _m->_getLastBlock(_x, _y, true);
	if (_l) {
		_xo = _x - _l->_x;
		_x = _l->_x;
		_yo = _y - _l->_y;
		_p = _xo + (_yo << B_BITS);
	}
}

CellID MapPointer::get() {
	
	if (_l) {
		return _l->_block[_p];
	}

	return ABSENT_CELL;
}

#define MOVE_RIGHT 1

void MapPointer::move(int dir) {
	switch (dir) {
	case MOVE_RIGHT:
		_xo++;
		_p++;		
		if (_xo >= LB_SIZE) {
			_x += _xo;
			_l = _m->_getLastBlock(_x, _y, true);
			if (_l) {
				_xo = _x - _l->_x;
				_x = _l->_x;
				_yo = _y - _l->_y;
				_p = _xo + (_yo << B_BITS);
			}
		}
	}
}

void MapPointer::moveTo(long long __x, long long __y) {
	_x = __x + MAP_MID;
	_y = __y + MAP_MID;
	_l = _m->_getLastBlock(_x, _y, true);
	if (_l) {
		_xo = _x - _l->_x;
		_x = _l->_x;
		_yo = _y - _l->_y;
		_p = _xo + (_yo << B_BITS);
	}
}


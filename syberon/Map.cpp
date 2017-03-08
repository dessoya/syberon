#include "Map.h"

#include "string.h"
#include "Logger.h"
#include "Utils.h"

#define _1 ((unsigned long long)1)

LastBlock::LastBlock(lint x, lint y) : _x(x), _y(y) {
	_block = new CellID[1 << (LB_BITS + LB_BITS)];
	memset(_block, 0, (1 << (LB_BITS + LB_BITS)) * sizeof(CellID));
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

	_m = (level - 1) * B_BITS + LB_BITS;

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


		lprint(std::string("Block::getBlock ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(x1) + "x" + inttostr(y1) + " " + inttostr(x2) + "x" + inttostr(y2));
	}
	*/


	auto x1 = (x - _x) >> _m;
	auto y1 = (y - _y) >> _m;
	// lprint(std::string("Block::getBlock ") + inttostr(_1 << _m) + " " + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(x1) + "x" + inttostr(y1));
	return _block[x1 + (y1 << B_BITS)];
}

Block *Block::addBlock(lint x, lint y) {

	auto x1 = (x - _x) >> _m;
	auto y1 = (y - _y) >> _m;

	if (_level == 1) {
		return (Block *)(_lblock[x1 + (y1 << B_BITS)] = new LastBlock((x1 << _m) + _x, (y1 << _m) + _y));
	}
	return _block[x1 + (y1 << B_BITS)] = new Block(_level - 1, (x1 << _m) + _x, (y1 << _m) + _y);
}


// cells of basic map
// LB_SIZE * B_SIZE
// middle point = LB_SIZE / 2

Map::Map() {
	_level = 1;

	auto d = (1 << (B_BITS - 1 + LB_BITS));

	_root = new Block(_level, MAP_MID - d, MAP_MID - d);
	_m = LB_BITS + B_BITS;
	
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
		lprint(std::string("out of map ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr(d));
		return;
	}

	lprint(std::string("map bound ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr(d));

	while (level > 0) {


		auto bx = (x - b->_x) >> _m;
		auto by = (y - b->_y) >> _m;

		lprint(std::string("level ") + inttostr(level) + " bx " + inttostr(bx - MAP_MID) + " by " + inttostr(by - MAP_MID));

		level--;
	}

}

void Map::setCell(lint x, lint y, CellID id) {

	// level
	x += MAP_MID;
	y += MAP_MID;

	auto level = _level;
	auto b = _root;
	auto m = _m;

	auto d = b->_x + (_1 << m);

	if (b->_x > x || b->_y > y || d <= x || d <= y) {
		lprint(std::string("out of map ") + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(b->_x - MAP_MID) + " " + inttostr(b->_y - MAP_MID) + " " + inttostr((void *)d));
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

	l->_block[bx + (by << LB_BITS)] = id;
}


CellID Map::getCell(lint x, lint y) {

	// level
	x += MAP_MID;
	y += MAP_MID;

	auto level = _level;
	auto b = _root;
	auto m = _m;

	auto d = b->_x + (_1 << m);

	if (b->_x > x || b->_y > y || d <= x || d <= y) {

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

	return l->_block[bx + (by << LB_BITS)];
}

void Map::addBlock(lint x, lint y) {

	// 
	// level
	x += MAP_MID;
	y += MAP_MID;

	auto level = _level;
	auto b = _root;
	auto m = _m;

	
	while (true) {
		auto d = b->_x + (_1 << m);

		long long x1 = b->_x - MAP_MID;
		long long y1 = b->_y - MAP_MID;
		if (b->_x < MAP_MID) {
			x1 = (long long)(MAP_MID - b->_x) * -1;
		}
		if (b->_y < MAP_MID) {
			y1 = (long long)(MAP_MID - b->_y) * -1;
		}
		lprint(std::string("Map::addBlock ") + inttostr(level) + " " + inttostr(x - MAP_MID) + "x" + inttostr(y - MAP_MID) + " " + inttostr(x1) + " " + inttostr(y1) + " " + inttostr(d - MAP_MID));

		if (b->_x > x || b->_y > y || d <= x || d <= y) {
			lprint("out of bound");
			level++;
			_level++;

			auto e = (_1 << ((B_BITS  * (level - 1)) - 1 + LB_BITS));
			auto e2 = (_1 << ((B_BITS  * level) - 1 + LB_BITS));

			e = e2 - e;

			_m += B_BITS;
			m += B_BITS;

			_root = new Block(level, b->_x - e, b->_y - e);

			// place b at B_BITS >> 1
			auto x1 = _1 << (B_BITS >> 1);
			_root->_block[x1 + (x1 << B_BITS)] = b;

			b = _root;

			continue;
		}
		break;
	}

	while (level > 0 && b) {

		auto bx = (x - b->_x) >> m;
		auto by = (y - b->_y) >> m;

		// lprint(std::string("level ") + inttostr(level) + " bx " + inttostr(bx) + " by " + inttostr(by));
		auto bb = b;
		b = b->getBlock(x, y);

		if (b == NULL) {
			b = bb->addBlock(x, y);
		}

		level--;
	}

}


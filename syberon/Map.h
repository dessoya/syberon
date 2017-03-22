#pragma once
#include <string>
#include <boost/thread/mutex.hpp>

typedef unsigned long long lint;
typedef unsigned short int CellID;
#define ABSENT_CELL 0x0fff
#define MAP_MID 0x8000000000000
// #define MAP_MID 4096
// #define MAP_MID 0x1000

// #define LB_SIZE 64
// #define LB_BITS 6
// #define LB_BITS 5

class LastBlock {
private:

public:
	lint *_ids;
	int _count, _size;

	CellID *_block;
	lint _x, _y;
	LastBlock(lint x, lint y);

	void addID(lint id);
	void delID(lint id);
};

typedef LastBlock *PLastBlock;

// #define B_SIZE 16
// #define B_BITS 4
#define B_BITS 5

class Block;

typedef Block *PBlock;

class Block {
private:
	lint _level;
	lint _m;
	boost::mutex _mutex;

public:
	lint _x, _y;
	union {
		PBlock *_block;
		PLastBlock *_lblock;
	};

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }

	Block(lint level, lint x, lint y);

	Block *getBlock(lint x, lint y);
	Block *addBlock(lint x, lint y);

	void dump(std::string *l = NULL);
};

class Map;

class MapPointer {
public:
	LastBlock *_l;
	long long _x, _y;
	int _xo, _yo, _p;
	Map *_m;

	MapPointer(Map *m, long long __x, long long __y);
	CellID get();
	void move(int dir);
	void moveTo(long long __x, long long __y);
};

class Map {
private:
	boost::mutex _mutex;
	lint _level;
	Block *_root;
public:
	lint _m;
	Map();

	MapPointer *getPointer(long long __x, long long __y);

	CellID getCell(long long __x, long long __y);
	void addBlock(long long __x, long long __y);
	void setCell(long long __x, long long __y, CellID id);

	void addSetCell(long long __x, long long __y, CellID id);
	void addSetFlags(long long __x, long long __y, CellID flags);
	

	void addSetIfEmpty(long long __x, long long __y, CellID id);
	void addSetIfEmptyAndId(long long __x, long long __y, CellID id, CellID id2);
		
	LastBlock *getLastBlock(long long __x, long long __y, bool _add = false);
	LastBlock *_getLastBlock(long long __x, long long __y, bool _add);

	LastBlock *addID(long long __x, long long __y, lint id);
	void delID(long long __x, long long __y, lint id);

	void dump();
	void trace_coords(lint x, lint y);
};

typedef Map WorldMap;
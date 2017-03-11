#pragma once
#include <string>

typedef unsigned long long lint;
typedef unsigned short int CellID;
#define ABSENT_CELL 0xffff
#define MAP_MID 0x800000000000000
// #define MAP_MID 4096
// #define MAP_MID 0x1000

// #define LB_SIZE 64
// #define LB_BITS 6
// #define LB_BITS 5

class LastBlock {
private:

public:
	CellID *_block;
	lint _x, _y;
	LastBlock(lint x, lint y);
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
public:
	lint _x, _y;
	union {
		PBlock *_block;
		PLastBlock *_lblock;
	};

	Block(lint level, lint x, lint y);

	Block *getBlock(lint x, lint y);
	Block *addBlock(lint x, lint y);

	void dump(std::string *l = NULL);
};


class Map {
private:
	lint _level;
	Block *_root;
	lint _m;
public:
	Map();

	CellID getCell(long long __x, long long __y);
	void addBlock(long long __x, long long __y);
	void setCell(long long __x, long long __y, CellID id);
	void addSetCell(long long __x, long long __y, CellID id);
	


	void dump();
	void trace_coords(lint x, lint y);
};

typedef Map WorldMap;
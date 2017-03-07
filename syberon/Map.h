#pragma once

typedef unsigned long long lint;
typedef unsigned short int CellID;
#define ABSENT_CELL 0xffff
#define MAP_MID 0x8000000000000000
// #define MAP_MID 0x1000

// #define LB_SIZE 64
// #define LB_BITS 6
#define LB_BITS 5

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
#define B_BITS 3

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
};


class Map {
private:
	lint _level;
	Block *_root;
	lint _m;
public:
	Map();

	CellID getCell(lint x, lint y);
	void addBlock(lint x, lint y);
	void setCell(lint x, lint y, CellID id);

	void trace_coords(lint x, lint y);
};

typedef Map WorldMap;
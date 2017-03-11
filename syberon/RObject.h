#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/thread/mutex.hpp>
#include "Image.h"

#include "DrawMachine.h"
#include "ImageFont.h"
#include "Map.h"

class RObject {

public:
	int _id;
	int _index;

	RObject();
	virtual ~RObject();

	virtual void draw(DrawMachine *dm) = 0;
};

using namespace ::boost::multi_index;

typedef multi_index_container<
	RObject *,
	indexed_by<
	ordered_unique< member<RObject, int, &RObject::_index> >,
	hashed_unique< member<RObject, int, &RObject::_id> >
	>
> _RObjectSet;


class RObjectSet {

public:
	_RObjectSet _set;
	int _topIndex, _botIndex;

	RObjectSet() : _topIndex(1000), _botIndex (2000) {}
};

// -------------------------------------------------------------------------------

class RRect : public RObject {

private:

	boost::mutex _propMutex;

	int _x, _y;
	int _w, _h;
	unsigned char _r, _g, _b;

public:

	RRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
	void draw(DrawMachine *dm);
	void setProp(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
};

class RBox: public RObject {

private:

	boost::mutex _propMutex;

	int _x, _y;
	int _w, _h;
	unsigned char _r, _g, _b;

public:

	RBox(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
	void draw(DrawMachine *dm);
	void setProp(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
};

class RText : public RObject {
private:
	boost::mutex _propMutex;

	int _x, _y;
	std::wstring _text;
	HFONT _font;
	unsigned char _r, _g, _b;

public:
	RText(int x, int y, std::wstring text, HFONT font, char r, unsigned char g, unsigned char b);
	void draw(DrawMachine *dm);
	void setProp(int x, int y, std::wstring text, HFONT font, char r, unsigned char g, unsigned char b);
};

class RIText : public RObject {
private:
	boost::mutex _propMutex;

	int _x, _y;
	std::wstring _text;
	ImageFont *_font;

public:
	RIText(int x, int y, std::wstring text, ImageFont *font);
	void draw(DrawMachine *dm);
	void setProp(int x, int y, std::wstring text, ImageFont *font);
};

class RImage : public RObject {
private:
	boost::mutex _propMutex;

	int _x, _y;
	Image *_image;
	int _sx, _sy, _sw, _sh;
	bool _useAlpha;

public:
	RImage(int x, int y, Image *image, int sx, int sy, int sw, int sh, bool useAlpha);
	void draw(DrawMachine *dm);
	void setProp(int x, int y, Image *image, int sx, int sy, int sw, int sh, bool useAlpha);
};

typedef struct {

	Image *image;
	int x, y;

} CellInfo;

typedef CellInfo *PCellInfo;

typedef struct {
	int w, h;
	double k;
} ScaleInfo;

class RMap : public RObject {
private:
	boost::mutex _propMutex;

	PCellInfo *_images;
	ScaleInfo *_scaleInfo;
	int _curScale;
	CellID *_map;
	int _vw, _vh, _vwp, _vhp;
	int _cw, _ch;
	int _ox, _oy;
	int _scales, _cells;
	long long _mx, _my, _mxp, _myp;
	WorldMap *_worldMap;

	void _loadFromWorldMap();

public:
	RMap(WorldMap *map, int scales, int cells);
	void draw(DrawMachine *dm);

	void setScaleInfo(int s, int w, int h);
	void setScale(int s);
	double getScaleK() { return _scaleInfo[_curScale].k; }

	void updateCells();

	void setupCellImage(int s, CellID id, Image *image, int x, int y);
	void setupViewSize(int w, int h, bool lock = true);
	void setCoors(long long x, long long y, bool lock = true);
};

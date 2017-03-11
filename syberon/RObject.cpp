#include "RObject.h"
#include "Utils.h"
#include "Logger.h"

#include "logConfig.h"
#ifdef _LOG_ROBJECT
#define lprint_ROBJECT(text) lprint(text)
#else
#define lprint_ROBJECT(text)
#endif
int RObjectIterator = 1;

RObject::RObject() {
	_id = RObjectIterator++;
}

RObject::~RObject() { }

// ----------------------------------------------------------------


RRect::RRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
	: RObject(), _x(x), _y(y), _w(w), _h(h), _r(r), _g(g), _b(b) {

}

void RRect::draw(DrawMachine *dm) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	// lprint("RRect::x " + inttostr(_x));
	dm->Rect(_x, _y, _w, _h, (_r << 16) | (_g << 8) | _b);
}

void RRect::setProp(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	_x = x;
	_y = y;
	_w = w;
	_h = h;
	_r = r;
	_g = g;
	_b = b;
}

RBox::RBox(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
	: RObject(), _x(x), _y(y), _w(w), _h(h), _r(r), _g(g), _b(b) {

}

void RBox::draw(DrawMachine *dm) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	// lprint("RRect::x " + inttostr(_x));
	DWORD c = (_r << 16) | (_g << 8) | _b;
	dm->Rect(_x, _y, _w, 1, c);
	dm->Rect(_x, _y + _h - 1, _w, 1, c);

	dm->Rect(_x, _y, 1, _h, c);
	dm->Rect(_x + _w - 1, _y, 1, _h, c);
}

void RBox::setProp(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	_x = x;
	_y = y;
	_w = w;
	_h = h;
	_r = r;
	_g = g;
	_b = b;
}


RText::RText(int x, int y, std::wstring text, HFONT font, char r, unsigned char g, unsigned char b)
	: RObject(), _x(x), _y(y), _text(text), _font(font), _r(r), _g(g), _b(b) {

}

void RText::draw(DrawMachine *dm) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	dm->Text(_x, _y, _text.c_str(), _font, RGB(_r, _g, _b));
}

void RText::setProp(int x, int y, std::wstring text, HFONT font, char r, unsigned char g, unsigned char b) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_x = x;
	_y = y;
	_text = text;
	_font = font;
	_r = r;
	_g = g;
	_b = b;
}

RIText::RIText(int x, int y, std::wstring text, ImageFont *font)
	: RObject(), _x(x), _y(y), _text(text), _font(font) {

}

void RIText::draw(DrawMachine *dm) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	dm->ImageText(_x, _y, _text.c_str(), _font);
}

void RIText::setProp(int x, int y, std::wstring text, ImageFont *font) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_x = x;
	_y = y;
	_text = text;
	_font = font;
	
}


RImage::RImage(int x, int y, Image *image, int sx, int sy, int sw, int sh, bool useAlpha)
	: RObject(), _x(x), _y(y), _image(image), _sx(sx), _sy(sy), _sw(sw), _sh(sh), _useAlpha(useAlpha) {
}

void RImage::draw(DrawMachine *dm) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	boost::unique_lock<boost::mutex> scoped_lock2(_image->_propMutex);

	dm->DrawImage(_image, _x, _y, _sx, _sy, _sw, _sh, _useAlpha);
}

void RImage::setProp(int x, int y, Image *image, int sx, int sy, int sw, int sh, bool useAlpha) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_x = x;
	_y = y;
	_image = image;
	_sx = sx;
	_sy = sy;
	_sw = sw;
	_sh = sh;
	_useAlpha = useAlpha;
}


#define MAP_W 256
#define MAP_H 192
#define MAX_CELLS 256

char *bline = NULL;
char *gline = NULL;

void _memset(char *b, DWORD c, int l) {
	DWORD *d = (DWORD *)b;
	while (l--) {
		*d = c;
		d++;
	}
}
RMap::RMap(Map *map, int scales, int cells) : _worldMap(map), _scales(scales), _cells(cells) {

	_vwp = _vhp = _mxp = _myp = 0;

	if (bline == NULL) {
		bline = new char[4 * 256];
		_memset(bline, 0x00000000, 256);
	}

	if (gline == NULL) {
		gline = new char[4 * 256];
		_memset(gline, 0x00303030, 256);
	}
	
	_scaleInfo = new ScaleInfo[_scales];

	_map = new CellID[MAP_W * MAP_H];
	memset(_map, 0, sizeof(CellID) * MAP_W * MAP_H);

	_vw = 3; _ox = 0; _mx = 0;
	_vh = 3; _oy = 0; _my = 0;

	_images = new PCellInfo[_scales];

	for (int i = 0; i < _scales; i++) {
		_images[i] = new CellInfo[_cells];
		memset(_images[i], 0, sizeof(CellInfo) * _cells);
	}

}

#define CELL_SIZE 1024
#define CELL_BITS 10

void RMap::setScaleInfo(int s, int w, int h) {
	_scaleInfo[s].w = w;
	_scaleInfo[s].h = h;
	_scaleInfo[s].k = (double)CELL_SIZE / (double)w;
}

void RMap::setScale(int s) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);
	_curScale = s;

	setupViewSize(_vwp, _vhp, false);
	setCoors(_mxp, _myp, false);

}


void RMap::setupCellImage(int s, CellID id, Image *image, int x, int y) {

	_images[s][id].image = image;
	_images[s][id].x = x;
	_images[s][id].y = y;

}

void RMap::setupViewSize(int w, int h, bool lock) {

	boost::unique_lock<boost::mutex> *scoped_lock;
	if(lock) scoped_lock = new boost::unique_lock<boost::mutex>(_propMutex);

	_vwp = w;
	_vhp = h;

	_vw = w / _scaleInfo[_curScale].w + 2;
	_vh = h / _scaleInfo[_curScale].h + 2;

	// lprint("w " + inttostr(_vw) + " h " + inttostr(_vh));

	_loadFromWorldMap();

	if (lock) delete scoped_lock;
}

void RMap::updateCells() {

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_loadFromWorldMap();
}

void RMap::_loadFromWorldMap() {

	// lprint("x " + inttostr(_mx) + " y " + inttostr(_mx) + " w " + inttostr(_vw) + " h " + inttostr(_vh));

	// lets load from worldmap
	for (lint y = 0; y < _vh; y++) {
		for (lint x = 0; x < _vw; x++) {
			_map[x + y * MAP_W] = _worldMap->getCell(_mx + x, _my + y);
		}
	}
}


void RMap::setCoors(long long x, long long y, bool lock) {
	boost::unique_lock<boost::mutex> *scoped_lock;
	if (lock) scoped_lock = new boost::unique_lock<boost::mutex>(_propMutex);

	_mxp = x;
	_myp = y;

	auto s = &_scaleInfo[_curScale];
	x = ((double)x / s->k);
	y = ((double)y / s->k);
	_ox = x % s->w;
	_oy = y % s->h;

	auto _tmx = x / s->w;
	auto _tmy = y / s->h;
	if (_ox < 0) {
		_ox += s->w;
		_tmx--;
	}
	if (_oy < 0) {
		_oy += s->h;
		_tmy--;
	}
	lprint_ROBJECT(std::string("RMap::setCoors ") + inttostr(x) + " " + inttostr(y));
	lprint_ROBJECT(std::string("RMap::setCoors ") + inttostr(_ox) + " " + inttostr(_oy) + " " + inttostr(_tmx) + " " + inttostr(_tmy));
	if (_tmx != _mx || _tmy != _my) {
		_mx = _tmx;
		_my = _tmy;
		_loadFromWorldMap();
	}

	if (lock) delete scoped_lock;
}

void RMap::draw(DrawMachine *dm) {

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	dm->lockDDS();

	unsigned char *p = dm->_surface;
	long l = dm->_pitch;

	auto s = &_scaleInfo[_curScale];
	_cw = s->w;
	_ch = s->h;

	// long l2 = l / 4;
	// lprint(std::string("l ") + inttostr(l) + " l2 " + inttostr(l2));

	for (int y = 0; y < _vh; y++) {

		int ty = y * _ch - _oy;

		if (ty >= dm->_height) break;

		auto c = &_map[MAP_W * y];

		for (int x = 0; x < _vw; x++) {

			// lets draw cell
			auto cid = *c;

			/*
			0 = gline
			ABSENT_CELL = bline
			*/

			bool _add = true;

			int tx = x * _cw - _ox;
			if (tx >= dm->_width) break;
			

			DWORD *imageLine;
			long l3;

			if (cid == 0 || cid == ABSENT_CELL) {
				_add = false;
				if (cid) {
					imageLine = (DWORD *)bline;
				}
				else {
					imageLine = (DWORD *)gline;
				}
			}
			else {
				auto i = &_images[_curScale][cid];
				auto image = i->image;
				l3 = image->_width;
				auto xo = 0, yo = 0;
				if (tx < 0) {
					xo = tx;
				}
				if (ty < 0) {
					yo = ty;
				}
				imageLine = (DWORD *)&image->_data[((i->y - yo) * image->_width + i->x - xo) * 4];
			}

			// fix tx and l1
			int l1 = _cw;
			if (tx < 0) {
				l1 += tx;
				tx = 0;
			}

			if (tx + l1 > dm->_width) {
				l1 = _cw - ((tx + l1) - dm->_width);
			}

			int l11 = _ch;
			if (ty < 0) {
				l11 += ty;
				/*
				if (_add) {
					imageLine -= l3 * ty;
				}
				*/
				// ty = 0;
			}

			if (ty + l11 > dm->_height) {
				l11 = _ch - ((ty + l11) - dm->_height);
			}

			auto sLine = &p[(ty < 0 ? 0 : ty) * l + tx * 4];

			for (int y1 = 0; y1 < l11; y1++) {

				DWORD *imageLine1 = imageLine;
				DWORD *sLine1 = (DWORD *)sLine;

				memcpy(sLine, imageLine, l1 * 4);

				if (_add) {
					imageLine += l3;
				}
				sLine += l;
			}
			c++;
		}
	}

	dm->unlockDDS();
}

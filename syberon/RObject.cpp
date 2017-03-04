#include "RObject.h"
#include "Utils.h"
#include "Logger.h"

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

void RImage::setProp(int x, int y, int sx, int sy, int sw, int sh, bool useAlpha) {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_x = x;
	_y = y;
	_sx = sx;
	_sy = sy;
	_sw = sw;
	_sh = sh;
	_useAlpha = useAlpha;
}


#define MAP_W 256
#define MAP_H 192
#define MAX_CELLS 256

RMap::RMap(int w, int h) : _cw(w), _ch(h) {

	_map = new CellID[MAP_W * MAP_H];
	memset(_map, 0, sizeof(CellID) * MAP_W * MAP_H);

	_vw = 3;
	_vh = 3;

	_images = new CellInfo[MAX_CELLS];
	memset(_images, 0, sizeof(CellInfo) * MAX_CELLS);

}

void RMap::setupCellImage(CellID id, Image *image, int x, int y) {

	_images[id].image = image;
	_images[id].x = x;
	_images[id].y = y;

}

void RMap::setupViewSize(int w, int h) {
	
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_vw = w;
	_vh = h;
}

void RMap::draw(DrawMachine *dm) {

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	dm->lockDDS();

	unsigned char *p = dm->_surface;
	long l = dm->_pitch;

	// long l2 = l / 4;
	// lprint(std::string("l ") + inttostr(l) + " l2 " + inttostr(l2));

	for (int y = 0; y < _vh; y++) {

		int ty = y * _ch;
		if (ty >= dm->_height) break;

		auto c = &_map[MAP_W * y];

		for (int x = 0; x < _vw; x++) {

			// lets draw cell
			auto i = &_images[*c];
			auto image = i->image;
			long l3 = image->_width;

			int tx = x * _cw;
			if (tx >= dm->_width) break;
			
			DWORD *imageLine = (DWORD *)&image->_data[(i->y * image->_width + i->x) * 4];
			auto sLine = &p[ty * l + tx * 4];

			int l1 = _cw;
			if (tx + l1 > dm->_width) {
				l1 = _cw - ((tx + l1) - dm->_width);
			}

			int l11 = _ch;
			if (ty + l11 > dm->_height) {
				l11 = _ch - ((ty + l11) - dm->_height);
			}

			for (int y1 = 0; y1 < l11; y1++) {

				// if (ty + y1 >= dm->_height) break;


				// if (l1 < 1) break;

				DWORD *imageLine1 = imageLine;
				DWORD *sLine1 = (DWORD *)sLine;

				memcpy(sLine, imageLine, l1 * 4);

				/*
				for (int x1 = 0; x1 < l1; x1++) {

					*sLine1 = *imageLine1;

					sLine1 ++;
					imageLine1 ++;

				}
				*/

				imageLine += l3;
				sLine += l;

			}

			c++;
		}

	}

	dm->unlockDDS();
}

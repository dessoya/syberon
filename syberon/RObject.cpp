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

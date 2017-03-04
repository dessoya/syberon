#include "DrawMachine.h"
#include "boost\lexical_cast.hpp"
#include "Logger.h"
#include "Utils.h"
#include "PeriodCorrector.h"

#include <emmintrin.h>  

extern char *DDErrorString(HRESULT hr);

void DrawMachine::setDDS(LPDIRECTDRAWSURFACE7 dds, int w, int h) {

	_dds = dds;
	_width = w;
	_height = h;

	_lock = 0;
}

void DrawMachine::lockDDS() {

	if (_lock > 0) {
		_lock++;
		return;
	}

	_lock++;

	//auto t2 = new TimeCalc();

	DDSURFACEDESC2 DDSDesc;
	DDSDesc.dwSize = sizeof(DDSDesc);

	HRESULT hr = _dds->Lock(NULL, &DDSDesc, DDLOCK_WAIT, NULL);
	if (hr != DD_OK) {
		lprint(std::string("Error DrawMachine::setDDS _dds->Lock ") + DDErrorString(hr));
		return;
	}

	_surface = (unsigned char *)DDSDesc.lpSurface;
	_pitch = DDSDesc.lPitch;

	//lprint(std::string("lock time ") + inttostr(t2->get()));
	//delete t2;
}

void DrawMachine::unlockDDS() {

	if (_lock > 1) {
		_lock--;
		return;
	}

	_lock--;

	//auto t2 = new TimeCalc();

	_dds->Unlock(NULL);

	//lprint(std::string("unlock time ") + inttostr(t2->get()));
	//delete t2;
}


void DrawMachine::Rect(int x, int y, int w, int h, DWORD color) {

	if (x > this->_width || y > this->_height) return;
	DDBLTFX ddbfx;
	RECT rcDest;

	ddbfx.dwSize = sizeof(ddbfx);
	ddbfx.dwFillColor = color;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}

	int xe = x + w;
	if (xe > this->_width) {
		xe = this->_width;
	}

	int ye = y + h;
	if (ye > this->_height) {
		ye = this->_height;
	}

	SetRect(&rcDest, x, y, xe, ye);


//	TimeCalc *t1 = new TimeCalc();
	this->_dds->Blt(&rcDest, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbfx);
//	lprint(std::string("draw rect time ") + inttostr(t1->get()));
//	delete t1;
}

void DrawMachine::Text(int x, int y,  const wchar_t *text, HFONT font, DWORD color) {
	HDC hdc;	

//	TimeCalc *t1 = new TimeCalc();
	_dds->GetDC(&hdc);
	if (hdc == NULL) {
		return;
	}

	SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, color);

	TextOut(hdc, x, y, text, lstrlen(text));

	_dds->ReleaseDC(hdc);
	
//	lprint(std::string("text time ") + inttostr(t1->get()));
//	delete t1;
}

void DrawMachine::ImageText(int x, int y, const wchar_t *text, ImageFont *font) {

	int l = wcslen(text);
	while (l--) {
		int s = *text;
		text++;

		// lprint(std::string("Symbol ") + inttostr((void *)s));

		SymbolInfo *si = font->_symbolMap[s];
		if (si == NULL) continue;

		DrawImage(si->_image, x, y - (si->bl - si->sy), si->x, si->y, si->w, si->h, true);

		x += si->w;
	}
}

void DrawMachine::DrawImage(Image *image, int x, int y, int fromx, int fromy, int w, int h, bool _useAlpha) {

	// lprint(std::string("DrawImage ") + inttostr(x) + " " + inttostr(y) + " " + inttostr(fromx) + " " + inttostr(fromy) + " " + inttostr(w) + " " + inttostr(h))
	if (_useAlpha) {

		// lprint("_useAlpha");

		/*
		DDSURFACEDESC2 DDSDesc;
		DDSDesc.dwSize = sizeof(DDSDesc);

		TimeCalc *t1 = new TimeCalc();

		HRESULT hr = _dds->Lock(NULL, &DDSDesc, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lprint(std::string("Error DrawMachine::DrawImage _dds->Lock ") + DDErrorString(hr));
			return;
		}

		lprint(std::string("lock time ") + inttostr(t1->get()));
		*/

		lockDDS();

	//	TimeCalc *t2 = new TimeCalc();

		unsigned char *p = _surface;
		unsigned char *s = (unsigned char *)image->_data;

		long l = _pitch;
		long ls = image->_width * 4;

		/*
		r = new Color();
		r.A = 1 - (1 - fg.A) * (1 - bg.A);
		if (r.A < 1.0e-6) return r; // Fully transparent -- R,G,B not important
		r.R = fg.R * fg.A / r.A + bg.R * bg.A * (1 - fg.A) / r.A;
		r.G = fg.G * fg.A / r.A + bg.G * bg.A * (1 - fg.A) / r.A;
		r.B = fg.B * fg.A / r.A + bg.B * bg.A * (1 - fg.A) / r.A;

		*/

		// __m128i a;
		// _mm_set_epi8()
		// _mm_subs_epu8();

		unsigned char *d = &p[l * y];
		s = &s[fromy * ls];

		for (int yi = 0; yi < h; yi++) {
			unsigned char *dl = &d[x * 4];
			unsigned char *sl = &s[fromx * 4];
			for (int xi = 0; xi < w; xi++, dl += 4, sl += 4) {

				if (sl[3] == 0) continue;

				if (sl[3] == 255) {
					dl[0] = sl[0];
					dl[1] = sl[1];
					dl[2] = sl[2];
					continue;
				}

				int a = 255 - sl[3];
				dl[0] = min((long(sl[0]) * long(sl[3]) + long(dl[0]) * a) / 255, 255);
				dl[1] = min((long(sl[1]) * long(sl[3]) + long(dl[1]) * a) / 255, 255);
				dl[2] = min((long(sl[2]) * long(sl[3]) + long(dl[2]) * a) / 255, 255);
				

			}
			s += ls;
			d += l;
		}

		// lprint(std::string("apply time ") + inttostr(t2->get()));

		/*
		TimeCalc *t3 = new TimeCalc();
		_dds->Unlock(NULL);
		lprint(std::string("unlock time ") + inttostr(t3->get()));

		delete t1;
		delete t3;
		*/

		unlockDDS();

		// delete t2;

		return;

	}

	RECT rcRect;

	if (fromx != -1) {
		rcRect.left = fromx;
	}
	else {
		rcRect.left = 0;
	}

	if (fromy != -1) {
		rcRect.top = fromy;
	}
	else {
		rcRect.top= 0;
	}

	if (w != -1) {
		rcRect.right = fromx + w;
	}
	else {
		rcRect.right = image->_width;
	}

	if (h != -1) {
		rcRect.bottom = fromy + h;
	}
	else {
		rcRect.bottom = image->_height;
	}

	_dds->BltFast(x, y, image->_dds, &rcRect, DDBLTFAST_SRCCOLORKEY);

}
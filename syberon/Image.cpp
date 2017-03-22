#include "stdio.h"
#include <stdarg.h>  
#include "malloc.h"
#include "ddraw.h"
#include "Logger.h"
#include "boost\lexical_cast.hpp"
#include "Utils.h"
#include "Files.h"
#include "Pack.h"

#include "Image.h"

#include "logConfig.h"
#ifdef _LOG_IMAGE
#define lprint_IMAGE(text) lprint(text)
#else
#define lprint_IMAGE(text)
#endif


#define PNG_BYTES_TO_CHECK 4

void abort_(char *n, ...) {

	va_list args, argsCopy;
	va_start(args, n);
	va_copy(argsCopy, args);

	char *b = new char[1024];
	vsnprintf(b, 1024, n, argsCopy);
	eprint(b);
	throw b;
}

LPDIRECTDRAW7 Image::dd;
extern char *DDErrorString(HRESULT hr);

Image::~Image() {
	delete _data;
	if (_has_dds) {
		_dds->Release();
	}
}

void Image::setPixel(int x, int y, DWORD color) {
	if (x < 0 || y < 0 || x >= _width || y >= _height) return;
	auto p = (DWORD *)&_data[(x + y * _width) * 4];
	/*
	p[0] = b;
	p[1] = g;
	p[2] = r;
	p[3] = 255;
	*/
	*p = color;
}

void Image::init(LPDIRECTDRAW7 dd) {
	Image::dd = dd;
}

Image::Image(_NewEmpty _3, int w, int h) {
	_has_dds = true;

	_width = w;
	_height = h;

	long s = w * h;
	_data = new char[s * 4];
	auto d = _data;
	for (int i = 0; i < s; i++) {
		d[0] = d[1] = d[2] = 1;
		d[3] = 255;
		d += 4;
	}

	createSurface();
	setupBitmap();
}

Image::Image(_LoadFromData _2, Data *data) {
	_has_dds = false;

	_data = data->_data;

	_width = _height = 0;

	_width = *((DWORD *)_data);
	_data += 4;

	_height = *((DWORD *)_data);
	_data += 4;

	createSurface();
	setupBitmap();
}


Image::Image(_LoadFromCache _1, const char *filepath, bool pack) {
	_has_dds = false;

	_width = _height = 0;
	FILE *f = fopen(filepath, "rb");
	fread(&_width, 4, 1, f);
	fread(&_height, 4, 1, f);
	long is = _width * _height * 4;
	_data = new char[is];

	if (pack) {
		long l = 0;
		fread(&l, 4, 1, f);
		char *d = new char[l];
		fread(d, l, 1, f);
		Unpacker u(_data, is, d, l);
	}
	else {
		fread(_data, _width * _height * 4, 1, f);
	}

	fclose(f);

	createSurface();
	setupBitmap();
}

void Image::saveToCache(const char *filepath, bool pack) {

	
	FILE *f = fopen(filepath, "wb");

	if (pack) {
	
		long px = (_width * _height * 4) + 8;
		char *d = new char[px];
		memcpy(d, &_width, 4);
		memcpy(d + 4, &_height, 4);
		memcpy(d + 8, _data, px - 8);
		
		Packer p(0, d, px);
		delete d;

		fwrite(&px, 4, 1, f);
		fwrite(p._data, p._l, 1, f);
	}
	else {
		fwrite(&_width, 4, 1, f);
		fwrite(&_height, 4, 1, f);
		fwrite(_data, _width * _height * 4, 1, f);
	}
	fclose(f);

}


Image::Image(const char *filepath) : _filename(filepath) {
// void read_png_file(char* file_name)	

	//lprint(filepath);
	_has_dds = true;

	char header[PNG_BYTES_TO_CHECK];
	png_structp _ptr;
	png_infop _info_ptr;

	/* open file and test for it being a png */


	FILE *fp;
	bool needClose = false;
	File *ff = NULL;

	if (Files_exists(filepath)) {
		//lprint(filepath + " 11");
		ff = Files_getInfo(filepath);
		//lprint(filepath + " 12");
		ff->lock();

		fp = ff->f;
		fseek(fp, ff->fpos, SEEK_SET);

	}
	else {
		needClose = true;
		fp = fopen(filepath, "rb");
		if (!fp)
			abort_("[read_png_file] File %s could not be opened for reading", filepath);
	}

	//lprint(filepath + " 2");

	fread(header, 1, PNG_BYTES_TO_CHECK, fp);
	if (png_sig_cmp((png_const_bytep)header, (png_size_t)0, PNG_BYTES_TO_CHECK))
		abort_("[read_png_file] File %s is not recognized as a PNG file", filepath);

	_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	_info_ptr = png_create_info_struct(_ptr);
	if (!_info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(_ptr)))
		abort_("[read_png_file] Error during init_io");

	png_init_io(_ptr, fp);
	png_set_sig_bytes(_ptr, PNG_BYTES_TO_CHECK);

	auto _number_of_passes = png_set_interlace_handling(_ptr);

	// png_read_png(_ptr, _info_ptr, PNG_TRANSFORM_EXPAND, NULL);
	png_read_png(_ptr, _info_ptr, 0, NULL);

	_width = png_get_image_width(_ptr, _info_ptr);
	_height = png_get_image_height(_ptr, _info_ptr);
	_color_type = png_get_color_type(_ptr, _info_ptr);
	_bit_depth = png_get_bit_depth(_ptr, _info_ptr);

	
	if (_color_type & PNG_COLOR_MASK_COLOR)
		png_set_bgr(_ptr);

	lprint_IMAGE(filepath);
	lprint_IMAGE("_bit_depth " + boost::lexical_cast<std::string>((int)_bit_depth));
	lprint_IMAGE("_color_type " + boost::lexical_cast<std::string>((int)_color_type));

	 auto _row_pointers = png_get_rows(_ptr, _info_ptr);

	 if (needClose) {
		 fclose(fp);
	 }
	 //lprint(filepath + " 3");


	_data = new char[_width * _height * 4];
	char *p = _data;
	
	for (int y = 0; y < _height; y++) {
		png_byte* row = _row_pointers[y];
		for (int x = 0; x < _width; x++, p+=4) {

			if (_color_type == PNG_COLOR_TYPE_RGB) {
				png_byte* ptr = &(row[x * 3]);

				p[0] = ptr[2];
				p[1] = ptr[1];
				p[2] = ptr[0];
				p[3] = 255;
			}
			else if (_color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
				png_byte* ptr = &(row[x * 4]);

				p[0] = ptr[2];
				p[1] = ptr[1];
				p[2] = ptr[0];
				p[3] = ptr[3];
				// lprint(inttostr((int)ptr[3]));
			}

		}
	}

	createSurface();
	setupBitmap();

	//lprint(filepath + " 4");

	png_destroy_read_struct(&_ptr, NULL, &_info_ptr);

	if (ff) {
		ff->unlock();
	}

	//lprint(filepath + " 5");

}

class st {
	
public:

	int p1, p2;
	int s1, s2, s;
	
	st(int m, int d) {

		
		if (m < d) {
			s1 = d / m;
			s2 = d % m;
			s = m;
			p1 = 0;
			p2 = d / 2;
		}
		else {
			p1 = 0;
			p2 = 0;
			s1 = 0;
			s2 = 1;
			s = m;
		}

		/*
		s1 = d / m;
		s2 = d % m;
		s = m;
		p1 = 0;
		p2 = d / 2;
		*/

		afterInc();
	}

	void afterInc() {
		while (p2 >= s) {
			p2 -= s;
			p1++;
		}
	}

	void step() {
		p1 += s1;
		p2 += s2;
		afterInc();
	}
};

Image::Image(Image *si, int m, int d) {
	_init(si, m, d, 0, 0, si->_width, si->_height);
}

Image::Image(Image *si, int m, int d, int sx, int sy, int sw, int sh) {
	_init(si, m, d, sx, sy, sw, sh);
}

void Image::_init(Image *si, int m, int d, int sx, int sy, int sw, int sh) {

	_has_dds = false;

	_width = sw * m / d;
	_height = sh * m / d;

	// lprint(inttostr(m) + " " + inttostr(d) + " " + inttostr(si->_width) + " " + inttostr(si->_height) + " " + inttostr(_width) + " " + inttostr(_height));

	_data = new char[_width * _height * 4];

	st ys(m, d);
	
	for (int y = 0; y < _height; y++) {

		st xs(m, d);

		for (int x = 0; x < _width; x++) {

			/*
			DWORD *dst = (DWORD *)&_data[(x + y * _width) * 4];
			DWORD *src = (DWORD *)&si->_data[(xs.p1 + (ys.p1 * si->_width)) * 4];

			*dst = *src;
			*/
			auto dst = &_data[(x + y * _width) * 4];
			auto src = &si->_data[(sx + xs.p1 + ((sy + ys.p1) * si->_width)) * 4];

			// lprint(inttostr(x) + " " + inttostr(y) + " " + inttostr(xs.p1) + " " + inttostr(ys.p1));

			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = src[3];

			xs.step();
		}

		ys.step();

	}
}


extern char *DDErrorString(HRESULT hr);

void Image::disableDDS() {

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	if (_dds) {
		_dds->Release();
		_dds = NULL;
	}

	_has_dds = false;
}

void Image::createSurface() {
	if (!_has_dds) return;

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.dwWidth = _width;
	ddsd.dwHeight = _height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	HRESULT hr = dd->CreateSurface(&ddsd, &_dds, NULL);
	if (hr != DD_OK) {
		eprint("Error: image::CreateSurface " + DDErrorString(hr));
	}
}

void Image::restore() {
	if (!_has_dds) return;

	// boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	_propMutex.lock();

	if (_dds->IsLost()) {
		lprint_IMAGE("Image lost " + _filename);
		HRESULT hr;
		do {
			hr = _dds->Restore();
			if (hr != DD_OK) {
				if (hr == DDERR_WRONGMODE) {
					_dds->Release();

					createSurface();
					break;
				}
				eprint(std::string("Error: ") + DDErrorString(hr));
				// Sleep(10);
				break;
			}
		} while (hr != DD_OK);

		_propMutex.unlock();
		setupBitmap();
		return;
	}

	_propMutex.unlock();
}


void Image::setupBitmap() {
	if (!_has_dds) return;

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	DDSURFACEDESC2 DDSDesc;
	DDSDesc.dwSize = sizeof(DDSDesc);

	HRESULT hr = _dds->Lock(NULL, &DDSDesc, DDLOCK_WAIT, NULL);
	if (hr == DD_OK) {

		unsigned char *p = (unsigned char *)DDSDesc.lpSurface;
		DWORD *s = (DWORD *)_data;
		for (int y = 0; y < _height; y++) {
			DWORD *l = (DWORD *)&p[y * DDSDesc.lPitch];

			/*for (int x = 0; x < _width; x++, s++, l++) {
				*l = *s;
			}*/
			memcpy(l, s, _width * 4);
			s += _width;
		}

		_dds->Unlock(NULL);

		DDCOLORKEY          ddck;
		ddck.dwColorSpaceLowValue = 0;
		ddck.dwColorSpaceHighValue = 0;

		_dds->SetColorKey(DDCKEY_SRCBLT, &ddck);
	}
	else {
		eprint("Error: image " + _filename + " lock " + DDErrorString(hr));
	}

}

void Image::draw(Image *image, int x, int y) {

	long fromx = 0, fromy = 0;
	long w = image->_width;
	long h = image->_height;

	if (y >= _height || x >= _width || x + w < 0 || y + h < 0) { return; }

	if (y + h > _height) { h = _height - y; }
	if (x + w > _width) { w = _width - x; }

	if (y < 0) {
		h += y;
		fromy -= y;
		y = 0;
	}

	if (x < 0) {
		w += x;
		fromx -= x;
		x = 0;
	}
	
	unsigned char *p = (unsigned char *)_data;
	unsigned char *s = (unsigned char *)image->_data;

	long l = _width * 4;
	long ls = image->_width * 4;

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

}

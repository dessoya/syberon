#include "stdio.h"
#include <stdarg.h>  
#include "malloc.h"
#include "ddraw.h"
#include "Logger.h"
#include "boost\lexical_cast.hpp"
#include "Utils.h"
#include "Files.h"

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
	throw b;
}

LPDIRECTDRAW7 Image::dd;
extern char *DDErrorString(HRESULT hr);

void Image::init(LPDIRECTDRAW7 dd) {
	Image::dd = dd;
}

Image::Image(const char *filepath) : _filename(filepath) {
// void read_png_file(char* file_name)	

	_has_dds = true;

	char header[PNG_BYTES_TO_CHECK];
	png_structp _ptr;
	png_infop _info_ptr;

	/* open file and test for it being a png */


	FILE *fp;
	bool needClose = false;

	if (Files_exists(filepath)) {
		File *ff = Files_getInfo(filepath);
		fp = ff->f;
		fseek(fp, ff->fpos, SEEK_SET);

	}
	else {
		needClose = true;
		fp = fopen(filepath, "rb");
		if (!fp)
			abort_("[read_png_file] File %s could not be opened for reading", filepath);
	}


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

	png_destroy_read_struct(&_ptr, NULL, &_info_ptr);
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

	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

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

		setupBitmap();
	}
}


void Image::setupBitmap() {
	if (!_has_dds) return;

	DDSURFACEDESC2 DDSDesc;
	DDSDesc.dwSize = sizeof(DDSDesc);

	HRESULT hr = _dds->Lock(NULL, &DDSDesc, DDLOCK_WAIT, NULL);
	if (hr == DD_OK) {

		unsigned char *p = (unsigned char *)DDSDesc.lpSurface;
		DWORD *s = (DWORD *)_data;
		for (int y = 0; y < _height; y++) {
			DWORD *l = (DWORD *)&p[y * DDSDesc.lPitch];

			for (int x = 0; x < _width; x++, s++, l++) {
				*l = *s;
			}
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
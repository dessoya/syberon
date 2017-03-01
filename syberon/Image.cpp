#include "stdio.h"
#include <stdarg.h>  
#include "malloc.h"
#include "ddraw.h"
#include "Logger.h"
#include "boost\lexical_cast.hpp"
#include "Utils.h"
#include "Files.h"

#include "Image.h"

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

	lprint(std::string("_bit_depth ") + boost::lexical_cast<std::string>((int)_bit_depth));
	lprint(std::string("_color_type ") + boost::lexical_cast<std::string>((int)_color_type));

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

extern char *DDErrorString(HRESULT hr);

void Image::createSurface() {
	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.dwWidth = _width;
	ddsd.dwHeight = _height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	HRESULT hr = dd->CreateSurface(&ddsd, &_dds, NULL);
	if (hr != DD_OK) {
		lprint(std::string("Error: image::CreateSurface ") + DDErrorString(hr));
	}
}

void Image::restore() {
	boost::unique_lock<boost::mutex> scoped_lock(_propMutex);

	if (_dds->IsLost()) {
		lprint("Image lost " + _filename);
		HRESULT hr;
		do {
			hr = _dds->Restore();
			if (hr != DD_OK) {
				if (hr == DDERR_WRONGMODE) {
					_dds->Release();

					createSurface();
					break;
				}
				lprint(std::string("Error: image::Restore ") + DDErrorString(hr));
				// Sleep(10);
				break;
			}
		} while (hr != DD_OK);

		setupBitmap();
	}
}


void Image::setupBitmap() {
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
		lprint(std::string("Error: image::Lock ") + DDErrorString(hr));
	}

}
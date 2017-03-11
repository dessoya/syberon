#pragma once

#include <png.h>
#include "ddraw.h"
#include <string>
#include <boost/thread/mutex.hpp>

class Image {
public:

	boost::mutex _propMutex;

	bool _has_dds;
	std::string _filename;
	int _width, _height;
	png_byte _color_type, _bit_depth;

	char *_data;

	LPDIRECTDRAWSURFACE7 _dds;

	Image(const char *filepath);
	Image(Image *si, int m, int d);

	void disableDDS();

	void restore();
	void createSurface();
	void setupBitmap();

	static LPDIRECTDRAW7 dd;
	static void init(LPDIRECTDRAW7 dd);

};
#pragma once

#include <png.h>
#include "ddraw.h"
#include <string>
#include <boost/thread/mutex.hpp>
#include "LuaScript.h"

typedef int _LoadFromCache;
const _LoadFromCache LoadFromCache = 1;

typedef int _LoadFromData;
const _LoadFromData LoadFromData = 2;

typedef int _NewEmpty;
const _NewEmpty NewEmpty = 3;


class Image {
public:

	boost::mutex _propMutex;

	bool _has_dds;
	std::string _filename;
	int _width, _height;
	png_byte _color_type, _bit_depth;

	char *_data;

	LPDIRECTDRAWSURFACE7 _dds;

	Image(_NewEmpty _3, int w, int h);
	Image(_LoadFromData _2, Data *data);

	Image(_LoadFromCache _1, const char *filepath, bool pack);
	void saveToCache(const char *filepath, bool pack);

	Image(const char *filepath);
	Image(Image *si, int m, int d);
	Image(Image *si, int m, int d, int sx, int sy, int sw, int sh);
	void _init(Image *si, int m, int d, int sx, int sy, int sw, int sh);

	~Image();
	void setPixel(int x, int y, DWORD color);
	void draw(Image *image, int x, int y);

	void disableDDS();

	void restore();
	void createSurface();
	void setupBitmap();

	static LPDIRECTDRAW7 dd;
	static void init(LPDIRECTDRAW7 dd);

};
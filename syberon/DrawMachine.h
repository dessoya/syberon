#pragma once

#include "windows.h"
#include "ddraw.h"
#include "Image.h"
#include "ImageFont.h"

class DrawMachine {
public:

	LPDIRECTDRAWSURFACE7 _dds;
	int _width, _height;
	unsigned char *_surface;
	long _pitch;
	long _lock;

	void setDDS(LPDIRECTDRAWSURFACE7 dds, int w, int h);
	void lockDDS();
	void unlockDDS();

	void Rect(int x, int y, int w, int h, DWORD color);
	void ImageText(int x, int y, const wchar_t *text, ImageFont *font);
	void Text(int x, int y, const wchar_t *text, HFONT font, DWORD color);
	void DrawImage(Image *image, int x, int y, int fromx = -1, int fromy = -1, int w = -1, int h = -1, bool _useAlpha = false);
	
};
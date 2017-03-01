#pragma once

#include "Image.h"

class SymbolInfo {
public:
	Image *_image;
	int x, y, w, h, bl, sy;
};

typedef SymbolInfo *PSymbolInfo;

class ImageFont {
public:

	PSymbolInfo *_symbolMap;

	ImageFont();
	void addSymbols(Image *image, const char *symbols);
};
#include "ImageFont.h" 
#include "Logger.h"
#include "Utils.h"

ImageFont::ImageFont() {

	_symbolMap = new PSymbolInfo[256];
	for (int i = 0; i < 256; i++) {
		_symbolMap[i] = NULL;
	}
}

void ImageFont::addSymbols(Image *image, const char *symbols) {

	int fromy = 0;

	do {

		int datay = -1, datah = 0;

		// search data line
		do {

			// lprint(std::string("line ") + inttostr(fromy));
			for (int x = 0; x < image->_width; x++) {
				unsigned char *p = (unsigned char *)&image->_data[(x + fromy * image->_width) * 4];
				// lprint(std::string("x ") + inttostr(x) + " " + inttostr((int)p[0]) + " " + inttostr((int)p[1]) + " " + inttostr((int)p[2]) + " " + inttostr((int)p[3]));
				if (p[3] == 0) continue;
				if (p[0] || p[1] || p[2]) {
					datay = fromy;
					break;
				}
			}

			fromy++;
			if (datay != -1) break;

		} while (fromy < image->_height);

		if (datay == -1) break;

		// search next empty line
		do {

			int c = 0;
			for (int x = 0; x < image->_width; x++) {
				unsigned char *p = (unsigned char *)&image->_data[(x + fromy * image->_width) * 4];
				if (p[3] == 0) continue;
				if (p[0] || p[1] || p[2]) {
					c++;					
				}
			}

			if (c == 0) {
				break;
			}

			fromy++;
			datah++;

		} while (fromy < image->_height);

		// now we have "datay" and "datah"
		// let's scan symbols

		if (datah < 1) {
			break;
		}

		int fromx = 0;
		do {

			int datax = -1, dataw = 0;
			// search data column

			do {
				for (int y = 0; y < datah; y++) {
					unsigned char *p = (unsigned char *)&image->_data[(fromx + (y + datay) * image->_width) * 4];

					if (p[3] == 0) continue;
					if (p[0] || p[1] || p[2]) {
						datax = fromx;
						break;
					}
				}

				fromx++;
				if (datax != -1) break;

			} while (fromx < image->_width);

			if (datax == -1) break;

			do {

				int c = 0;
				for (int y = 0; y < datah; y++) {
					unsigned char *p = (unsigned char *)&image->_data[(fromx + (y + datay) * image->_width) * 4];

					if (p[3] == 0) continue;

					if (p[0] || p[1] || p[2]) {
						c++;
					}
				}

				if (c == 0) {
					break;
				}

				fromx++;
				dataw++;

			} while (fromx < image->_width);

			if (dataw < 1) {
				break;
			}

			char b[2];
			b[0] = *symbols;
			b[1] = 0;
			lprint(std::string("symbol '") + b + "' " + inttostr(datax) + " " + inttostr(datay) + " size " + inttostr(dataw) + "x" + inttostr(datah));
			auto symbolInfo = new SymbolInfo;
			symbolInfo->_image = image;
			symbolInfo->x = datax;
			symbolInfo->y = datay;
			symbolInfo->w = dataw;
			symbolInfo->h = datah;
			symbolInfo->bl = 25;
			symbolInfo->sy = 23;
			unsigned char *symbol = (unsigned char *)symbols;
			_symbolMap[*symbol] = symbolInfo;
			symbols++;

		} while (fromx < image->_width && *symbols);

		// break;

	} while (fromy < image->_height && *symbols);


}
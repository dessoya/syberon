#include "Pack.h"
#include "stdio.h"
#include "windows.h"
#include "zlib.h"
#include "Logger.h"

Packer::Packer(int chunk, char *data, long l, int level) {

	_data = NULL;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if (deflateInit(&strm, level) != Z_OK) {
		eprint("error: deflateInit");
		return;
	}

	strm.avail_in = l;
	strm.next_in = (Bytef *)data;

	_data = new char[l];
	strm.avail_out = l;
	strm.next_out = (Bytef *)_data;

	auto r = deflate(&strm, Z_FINISH);
	if(!(r == Z_OK || r == Z_STREAM_END)) {
		eprint("error: deflate");
	}

	_l = l - strm.avail_out;
	deflateEnd(&strm);

}

Unpacker::Unpacker(char *dest, long destSize, char *src, long srcSize) {

	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	auto ret = inflateInit(&strm);

	strm.avail_in = srcSize;
	strm.next_in = (Bytef *)src;

	strm.avail_out = destSize;
	strm.next_out = (Bytef *)dest;

	ret = inflate(&strm, Z_NO_FLUSH);

	switch (ret) {
	case Z_NEED_DICT:
		ret = Z_DATA_ERROR;
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
		(void)inflateEnd(&strm);
		eprint("error: inflate");
		return;
	}

	(void)inflateEnd(&strm);
}

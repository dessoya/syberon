
#include "stdio.h"
#include "windows.h"
#include "zlib.h"
#include "Files.h"
#include "memory.h"
#include "Logger.h"
#include "Utils.h"

#include <set>

#include "logConfig.h"
#ifdef _LOG_FILES
#define lprint_FILES(text) lprint(text)
#else
#define lprint_FILES(text)
#endif


#define FL_PACK 1
#define FL_ENCODED 2

int unpackFromFile(FILE *source, char **pdata, long *pl);

struct classcomp {
	bool operator() (const File *lhs, const File *rhs) const
	{
		return lhs->_filename < rhs->_filename;
	}
};

typedef std::set<File *, classcomp> FileSet;
FileSet fileSet;

unsigned char *psig = (unsigned char *)"CoOoLLL pROt3Cti0n !!11";
int psig_pos = 0;

void _encode(char *b, int l, int pos = -1) {
	unsigned char *bb = (unsigned char *)b;

	bool setBack = true;
	if (pos != -1) {
		setBack = false;
	}
	else {
		pos = psig_pos;
	}
	while (l--) {
		*bb ^= psig[pos];
		pos++;
		if (pos >= strlen((char *)psig)) {
			pos = 0;
		}
		bb++;
	}
	if (setBack) {
		psig_pos = pos;
	}
}

int Files_openPack(const char *filename) {
	FILE *f = fopen(filename, "rb");
	lprint(std::string("open pack ") + filename);

	char b[256];
	int cnt = 0;

	boost::mutex *_mutex = new boost::mutex;

	while (true) {
		fread(&b, 1, 1, f);
		_encode(b, 1);
		if (*b == 0) break;
	
		int sz = *b;
		// lprint(inttostr(sz));

		fread(b, sz, 1, f);
		_encode(b, sz);
		b[sz] = 0;
		std::string fn(b);

		fread(b, 1, 1, f);
		_encode(b, 1);
		int flags = *b;

		DWORD dsz;
		fread((char *)&dsz, 4, 1, f);
		_encode((char *)&dsz, 4);

		lprint("file: " + fn + " size " + inttostr(dsz) + " flags " + inttostr(flags));

		sz = ftell(f);

		File *ff = new File(fn);
		ff->_mutex = _mutex;

		ff->f = f;
		ff->_type = FT_FILE;
		ff->fpos = sz;
		ff->cnt = cnt;
		ff->_len = dsz;
		ff->fflags = flags;

		fileSet.insert(ff);

		fseek(f, sz + dsz, SEEK_SET);
		cnt++;

	}

}

boost::mutex _fileSetMutex;

bool Files_exists(const char *filename) {

	// lprint(filename + " before lock");
	boost::unique_lock<boost::mutex> scoped_lock(_fileSetMutex);
	// lprint(filename + " after lock");

	auto it = fileSet.find(new File(std::string(filename)));

	if (it == fileSet.end()) {
		lprint_FILES(filename + " false");
		// lprint(filename + " false");
		return false;
	}
	lprint_FILES(filename + " true");
	// lprint(filename + " true");

	return true;
}

File *Files_getInfo(const char *filename) {

	boost::unique_lock<boost::mutex> scoped_lock(_fileSetMutex);

	auto it = fileSet.find(new File(std::string(filename)));

	if (it == fileSet.end()) {
		return NULL;
	}

	return *it;
}


bool Files_saveFile(const char *filename, const char *dfilename) {

	auto it = fileSet.find(new File(std::string(filename)));

	if (it == fileSet.end()) {
		return false;
	}

	File *f = *it;

	long l = f->_len;
	lprint("Files_saveFile " + std::string(filename) + " " + inttostr(l) + " " + inttostr(f->fpos));
	char *data = new char[l];
	f->lock();
	fseek(f->f, f->fpos, SEEK_SET);
	fread(data, l, 1, f->f);
	f->unlock();

	if (f->fflags & FL_ENCODED) {
		lprint("encode");
		psig_pos = f->cnt % strlen((char *)psig);
		_encode(data, l, psig_pos);
	}

	if (f->fflags & FL_PACK) {
		lprint("unpack");

		FILE *ff = fopen(dfilename, "wb+");
		fwrite(data, l, 1, ff);
		fseek(ff, 0, SEEK_SET);
		delete data;

		lprint(std::string("unpackFromFile ") + inttostr(unpackFromFile(ff, &data, &l)));

		fclose(ff);
	}

	FILE *ff = fopen(dfilename, "wb");
	fwrite(data, l, 1, ff);
	fclose(ff);	
	delete data;

	
	return true;
}

#define CHUNK 16384
int unpackFromFile(FILE *source, char **pdata, long *pl) {

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;


	char *data = NULL;
	long l = 0;

	/* decompress until deflate stream ends or end of file */
	do {

		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			lprint("ferror(source)");
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {

			strm.avail_out = CHUNK;
			strm.next_out = out;

			ret = inflate(&strm, Z_NO_FLUSH);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}

			have = CHUNK - strm.avail_out;

			/*
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
			}*/

			char *n = new char[l + have];
			if (data) {
				memcpy(n, data, l);
				memcpy(n + l, out, have);
				delete data;
			}
			else {
				memcpy(n, out, have);
			}

			l += have;
			data = n;

		} while (strm.avail_out == 0);


		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);

	lprint(std::string("unpack done"));
	*pdata = data;
	*pl = l;

	return 0;
}

int Files_loadPack(const char *filename) {

	/*
	if (ret != Z_STREAM_END) {
		return 1;
	}
	*/

	auto it = fileSet.find(new File(std::string(filename)));

	if (it == fileSet.end()) {
		return 0;
	}

	{
		File *f = *it;

		long l = f->_len;
		lprint("Files_loadPack " + std::string(filename) + " " + inttostr(l) + " " + inttostr(f->fpos));
		char *data = new char[l];
		fseek(f->f, f->fpos, SEEK_SET);
		fread(data, l, 1, f->f);

		if (f->fflags & FL_ENCODED) {
			lprint("encode");
			psig_pos = f->cnt % strlen((char *)psig);
			_encode(data, l);
		}


		FILE *ff = fopen("_tmp.pack", "wb");
		fwrite(data, l, 1, ff);
		fclose(ff);
		delete data;
	}

	// let's parse
	lprint(std::string("parse data"));

	char *data;
	long l;

	lprint(std::string("open pack ") + filename);
	FILE *source = fopen("_tmp.pack", "rb");

	unpackFromFile(source, &data, &l);
	fclose(source);
	remove("_tmp.pack");

	while (*data) {
		int fl = (int)*data;
		data++;
		char buf[1024];
		memcpy(buf, data, fl);
		buf[fl] = 0;
		File *f = new File(std::string(buf));
		data += fl;
		DWORD l = *((DWORD *)data);
		data += 4;
		f->setData(data, l);
		data += l;

		fileSet.insert(f);
	}

	/*
	auto it = fileSet.begin();
	while (it != fileSet.end()) {
		File *f = *it;
		lprint(f->_filename);
		it++;
	}
	*/

	return 0;

}

File *Files_get(const char *filename) {

	auto it = fileSet.find(new File(std::string(filename)));

	if (it != fileSet.end()) {
		return *it;
	}
	return NULL;
}


/*
int def(FILE *source, FILE *dest, int level) {
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, level);
	if (ret != Z_OK)
		return ret;

	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		do {

			strm.avail_out = CHUNK;
			strm.next_out = out;

			ret = deflate(&strm, flush);   

			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

										
	} while (flush != Z_FINISH);
										
	(void)deflateEnd(&strm);
	return Z_OK;
}


int inf(FILE *source, FILE *dest) {

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	do {

		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		do {

			strm.avail_out = CHUNK;
			strm.next_out = out;

			ret = inflate(&strm, Z_NO_FLUSH);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}

			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}

		} while (strm.avail_out == 0);


	} while (ret != Z_STREAM_END);

	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

*/
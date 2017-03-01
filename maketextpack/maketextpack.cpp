// maketextpack.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "maketextpack.h"
#include <windows.h>
#include "stdio.h"
#include "D:\zh\zlib-1.2.11\zlib.h"

#pragma comment(lib, "D:\\zh\\zlib-1.2.11\\contrib\\vstudio\\vc14\\x64\\ZlibStatDebug\\zlibstat.lib")

#define FL_PACK 1
#define FL_ENCODED 2

unsigned char *psig = (unsigned char *)"CoOoLLL pROt3Cti0n !!11";

int psig_pos = 0;
void _encode(char *b, int l) {
	unsigned char *bb = (unsigned char *)b;

	while (l--) {
		*bb ^= psig[psig_pos];
		psig_pos++;
		if (psig_pos >= strlen((char *)psig)) {
			psig_pos = 0;
		}
		bb++;
	}
}

int psig_pos2;
void _encode2(char *b, int l) {
	unsigned char *bb = (unsigned char *)b;

	while (l--) {
		*bb ^= psig[psig_pos2];
		psig_pos2++;
		if (psig_pos2 >= strlen((char *)psig)) {
			psig_pos2 = 0;
		}
		bb++;
	}
}

bool needPack(const char *filename) {

	if (
		strcmp("resource\\Roboto-Regular.ttf", filename) == 0
		) return true;

	return false;
}

bool needEncode(const char *filename) {

	if (
		strcmp("lua.pack", filename) == 0
		|| strcmp("resource\\Roboto-Regular.ttf", filename) == 0
		) return true;

	return false;
}

#define CHUNK 16384
int def(char *data, long l, char *destname) {
	FILE *dest = fopen(destname, "wb");
	int level = 9;
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, level);
	if (ret != Z_OK)
		return ret;

	/* compress until end of file */
	do {

		// strm.avail_in = fread(in, 1, CHUNK, source);

		int r = CHUNK;
		if (l < r) {
			r = l;
		}
		strm.avail_in = r;
		memcpy(in, data, r);
		l -= r;
		data += r;

		/*
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		*/

		flush = l == 0 ? Z_FINISH : Z_NO_FLUSH;

		strm.next_in = in;
		/* run deflate() on input until output buffer not full, finish
		compression if all of source has been read in */
		do {

			strm.avail_out = CHUNK;
			strm.next_out = out;

			ret = deflate(&strm, flush);    /* no bad return value */
			// assert(ret != Z_STREAM_ERROR);  /* state not clobbered */


			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		// assert(strm.avail_in == 0);     /* all input will be used */

										/* done when last data in file processed */
	} while (flush != Z_FINISH);
	// assert(ret == Z_STREAM_END);        /* stream will be complete */
										/* clean up and return */
	(void)deflateEnd(&strm);
	fclose(dest);
	return Z_OK;
}


void lprint(FILE *f,char *t) {
	int s = strlen(t);
	fwrite(t, s, 1, f);
	fflush(f);
}

char *tchar2char(wchar_t *t) {
	char *s = new char[wcslen(t) + 1];
	wcstombs(s, t, wcslen(t) + 1);
	return s;
}

wchar_t *char2tchar(char *s) {
	size_t size = strlen(s) + 1;
	wchar_t *wbuf = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wbuf, size, s, size - 1);
	return wbuf;
}

char *makecopy(char *s) {
	char *c = new char[strlen(s) + 1];
	memcpy(c, s, strlen(s) + 1);
	return c;
}
struct _Item;
typedef struct _Item {
	char *filename;
	long size;
	long csize;
	struct _Item *next;
} Item;

FILE *o;

Item *readdir(char *dir, char *wc = NULL, Item *list = NULL, Item **end = NULL) {

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	char b[1024];
	sprintf(b, "%s\\*", dir);

	Item *root = list;

	hFind = FindFirstFile(char2tchar(b), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		sprintf(b, "Error: FindFirstFile(%s)\n", dir);
		lprint(o, b);
		return 0;
	}

	do {

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

			char *filename = tchar2char(ffd.cFileName);
			if (filename[0] != '.') {
				sprintf(b, "%s\\%s", dir, filename);
				readdir(b, wc, list, &list);
			}

		}
		else {
			// check for .lua
			char *filename = tchar2char(ffd.cFileName);
			bool accept = true;

			if (wc) {
				accept = false;
				if (strlen(filename) > strlen(wc) && strcmp(wc, &filename[strlen(filename) - strlen(wc)]) == 0) {
					accept = true;
				}
			}

			if(accept) {
				if (list == NULL) {
					root = list = new Item;
				}
				else {
					Item *i = new Item;
					list->next = i;
					list = i;
				}
				sprintf(b, "%s\\%s", dir, filename);
				list->size = ffd.nFileSizeLow;
				list->filename = makecopy(b);
				list->next = NULL;
			}

		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if (end) {
		*end = list;
	}
	return root;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

	o = fopen("work.log", "wb");
	lprint(o, "start\n");

	// TCHAR szDir[MAX_PATH];
	// DWORD dwError = 0;

	char *dir = "D:\\zh\\cpp\\syberon\\lua";
	Item *list = readdir(dir, ".lua");
	char b[1024];

	long tsize = 0;
	Item *l = list;
	while (list) {
		char *r = &list->filename[strlen(dir) + 1];
		list->csize = 1 + strlen(r) + 4 + list->size;
		tsize += list->csize;
		sprintf(b, "%s\n", r);
		lprint(o, b);
		list = list->next;
	}

	char *data = new char[tsize];
	char *p = data;
	list = l;
	while (list) {
		char *r = &list->filename[strlen(dir) + 1];
		*p = strlen(r);
		p++;
		memcpy(p, r, strlen(r));
		p += strlen(r);
		DWORD *d = (DWORD *)p;
		*d = (DWORD)list->size;
		p += 4;

		FILE *f = fopen(list->filename, "rb");
		fread(p, list->size, 1, f);
		fclose(f);

		p += list->size;
		
		list = list->next;
	}

	tsize++;
	*p = 0;

	// encode
	// _encode(b, tsize);

	sprintf(b, "tsize %d\n", tsize);
	lprint(o, b);

	def(data, tsize, "D:\\zh\\cpp\\data\\lua.pack");

	
	dir = "D:\\zh\\cpp\\data";
	list = readdir(dir);
	l = list;

	FILE *fp = fopen("data.pack", "wb");

	int cnt = 0;
	while (list) {

		char *r = &list->filename[strlen(dir) + 1];

		sprintf(b, "%s\n", r);
		lprint(o, b);

		b[0] = strlen(r);
		_encode(b, 1);
		fwrite(b, 1, 1, fp);

		strcpy(b, r);
		_encode(b, strlen(r));
		fwrite(b, strlen(r), 1, fp);

		sprintf(b, "sz %d\n", strlen(r));

		unsigned char flags = 0;
		char *filedata = new char[list->size];
		DWORD sz = list->size;

		FILE *fs = fopen(list->filename, "rb");
		auto rd = fread(filedata, 1, sz, fs);
		// sprintf(b, "readed %d\n", rd);
		// lprint(o, b);
		fclose(fs);

		if (needPack(r)) {

			lprint(o, "* pack\n");

			flags |= FL_PACK;
			def(filedata, sz, "tmp.z");

			fs = fopen("tmp.z", "rb");

			fseek(fs, 0, SEEK_END);
			sz = ftell(fs);

			sprintf(b, "size %d\n", sz);
			lprint(o, b);

			fseek(fs, 0, SEEK_SET);

			auto rd = fread(filedata, 1, sz, fs);
			// sprintf(b, "readed %d\n", rd);
			// lprint(o, b);

			fclose(fs);
		}

		if (needEncode(r)) {

			lprint(o, "* encode\n");

			flags |= FL_ENCODED;
			psig_pos2 = cnt % strlen((char *)psig);
			_encode2(filedata, sz);
		}

		b[0] = flags;
		_encode(b, 1);
		fwrite(b, 1, 1, fp);

		int szz = sz;
		sprintf(b, "f %d, s %d\n", (int)flags, (int)szz);
		lprint(o, b);
		_encode((char *)&sz, 4);
		fwrite(&sz, 4, 1, fp);

		fwrite(filedata, szz, 1, fp);
		delete filedata;

		cnt ++;
		list = list->next;
	}

	b[0] = 0;
	_encode(b, 1);
	fwrite(b, 1, 1, fp);

	fclose(fp);


	lprint(o, "end\n");
	fclose(o);

	return 0;
}
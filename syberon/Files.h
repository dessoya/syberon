#pragma once

#include <string>
#include <boost/thread/mutex.hpp>


#define FT_NONE 0
#define FT_MEMORY 1
#define FT_FILE 2

class File {

public:

	boost::mutex *_mutex;

	void lock() { if(_mutex)_mutex->lock(); }
	void unlock() { if (_mutex)_mutex->unlock();  }

	std::string _filename;

	FILE *f;
	int fpos;
	int cnt;
	int fflags;

	char *_data;
	long _len;
	int _type;

	File(std::string filename) : _mutex(NULL), _type(FT_NONE), _filename(filename), _data(NULL), _len(0) { }

	void setData(char *d, long l) { _data = d; _len = l; _type = FT_MEMORY; }

};

int Files_loadPack(const char *filename);
File *Files_get(const char *filename);
bool Files_exists(const char *filename);
File *Files_getInfo(const char *filename);
bool Files_saveFile(const char *filename, const char *dfilename);
int Files_openPack(const char *filename);

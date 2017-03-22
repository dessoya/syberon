#include "Logger.h"
#include <stdio.h>
#include <vector>
#include <time.h>
#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>

// --------------------------------------------------------------------

boost::mutex _writeMutex;

struct _ThreadDesc;

typedef struct _ThreadDesc {

	std::string *name;
	DWORD threadId;
	struct _ThreadDesc *next;

} ThreadDesc;

ThreadDesc *threadNames = NULL;
ThreadDesc *threadNamesLast = NULL;

void Logger::setThreadName(const char *name) {
	auto id = GetCurrentThreadId();

	ThreadDesc *td = threadNames;
	while (td) {
		if (td->threadId == id) {
			break;
		}
		td = td->next;
	}
	if (td) {
		delete td->name;
		td->name = new std::string(name);
		return;
	}

	auto tid = GetCurrentThreadId();
	auto n = new std::string(name);

	auto tt = new ThreadDesc;
	tt->threadId = tid;
	tt->next = NULL;
	tt->name = n;

	if (threadNamesLast) {
		threadNamesLast->next = tt;
		threadNamesLast = tt;
	}
	else {
		threadNames = threadNamesLast = tt;
	}
}

Logger::Logger(std::string filepath) : _filepath (filepath) {

	_file = NULL;
}

void Logger::print(const char *string) {

	{
		boost::unique_lock<boost::mutex> *scoped_lock = new boost::unique_lock<boost::mutex>(_writeMutex);

		if (_file == NULL) {
			_file = fopen(_filepath.c_str(), "a+b");
			delete scoped_lock;
			scoped_lock = NULL;
			this->print("\n\nstarted");
		}

		if (scoped_lock) {
			delete scoped_lock;
		}
	}
	{
		boost::unique_lock<boost::mutex> scoped_lock(_writeMutex);
		auto id = GetCurrentThreadId();

		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "[%Y-%m-%d %H:%M:%S] ", timeinfo);

		std::vector<std::string> strs;
		boost::split(strs, std::string(string), boost::is_any_of("\n"));
		char sb[80];

		ThreadDesc *td = threadNames;
		while (td) {
			if (td->threadId == id) {
				break;
			}
			td = td->next;
		}

		int bl1 = strlen(buffer);

		if (td) {
			if (td->name->length() > 18) {
				sprintf(sb, "[%-18s] ", td->name->c_str());
				fwrite(buffer, bl1, 1, _file);
				fwrite(sb, strlen(sb), 1, _file);
				fwrite("\n", 1, 1, _file);
				sprintf(sb, "[%18d] ", (int)id);
			}
			else {
				sprintf(sb, "[%-18s] ", td->name->c_str());
			}
		}
		else {
			sprintf(sb, "[%18d] ", (int)id);
		}

		int bl2 = strlen(sb);
		for (std::vector<std::string>::iterator it = strs.begin(); it != strs.end(); ++it) {
			std::string s = *it;
			fwrite(buffer, bl1, 1, _file);
			fwrite(sb, bl2, 1, _file);
			fwrite(s.c_str(), s.length(), 1, _file);
			fwrite("\n", 1, 1, _file);
		}

		fflush(_file);
	}
}


// --------------------------------------------------------------------

Logger *Logger::loggers[10];

void Logger::setupLogFilepath(int logId, std::string filepath) {
	Logger::loggers[logId] = new Logger(filepath);
}


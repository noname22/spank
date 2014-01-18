// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

enum Severity {
	LOG_DEBUG,
	LOG_EXTRA_VERBOSE,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

#define LOG(_msg, _severity) do{\
	std::ostringstream _tmpStr;\
	_tmpStr << _msg;\
	Log::getInstance()->log(__FILE__, __FUNCTION__, __LINE__, _tmpStr.str(), _severity);\
} while(0)

#define LASSERT(_v, _msg) if(!(_v)) { LOG(_msg, LOG_FATAL); exit(1); }

class Log{
	Log();
	static Log* logInstance;
	int logLevel;
	std::string getSeverityName(int severity);

	public:
	void restrictDebugOutputToFiles(std::vector<std::string> files);
	void log(std::string file, std::string function, int line, std::string msg, int severity);
	void setLogLevel(int inLogLevel);
	static Log* getInstance();
	std::vector<std::string> restrictFiles;
};

#endif

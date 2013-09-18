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

//#include <SDL/SDL_thread.h>

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

//#include "settings.h"

enum errorlevels{
	LOG_DEBUG,
	LOG_EXTRA_VERBOSE,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

//#define AP_LOG_NUMSTRS

#define LOG_LOGLEVEL LOG_INFO

#define LOGI Log::getInstance()

#define LOGIT(string, errorlevel) LOGI->log(__FILE__, __FUNCTION__, string, errorlevel)

/*#define LOG(string, errorlevel){						\
	SDL_SemWait(LOGI->semaphore);						\
	LOGI->logString.str("");							\
	LOGI->logString << string;							\
	LOGIT(LOGI->logString.str(), errorlevel);			\
	SDL_SemPost(LOGI->semaphore);						\
}*/

#define LOG(string, errorlevel) do{				\
	std::ostringstream _tmpStr;					\
	_tmpStr.str("");							\
	_tmpStr << string;							\
	LOGIT(_tmpStr.str(), errorlevel);			\
} while(0)


class Log{
	public:

	std::ofstream logFile;
	
	void log(std::string file, std::string function, std::string logThis, int errorlevel);
	void setLogLevel(int inLogLevel);
	static Log* getInstance();
	//SDL_sem* semaphore;
	
	private:

	static Log* logInstance;
	Log();
	int logLevel;

	std::string getErrorLevelName(int errorlevel);
};

#endif


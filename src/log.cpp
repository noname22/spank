// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "log.h"
#include <iostream>

Log* Log::logInstance = NULL;

Log* Log::getInstance()
{
        if(logInstance == NULL){
                logInstance = new Log();
        }
        return logInstance;
}

void Log::setLogLevel(int inLogLevel)
{
	logLevel = inLogLevel;
}

Log::Log()
{
	//semaphore = SDL_CreateSemaphore(1);
	logLevel = LOG_LOGLEVEL;
//	logFile.open("log.txt");
}

void Log::log(std::string file, std::string function, std::string logThis, int errorlevel){
	if(errorlevel >= logLevel){
		if(errorlevel >= LOG_WARNING || errorlevel == LOG_DEBUG){
			
			std::cerr
				<< "[ "
				<< getErrorLevelName(errorlevel) 
				<< " ] " 
				<< logThis 
			#ifdef DEBUG_VERBOSE
				<< "\t(" << file << ": " << function << ")"
			#endif
				<< std::endl;
				//std::cout << logThis << std::endl;
		}else{
			std::cout << logThis << std::endl;
		}
	}
}

std::string Log::getErrorLevelName(int errorlevel)
{
	switch(errorlevel){
		case LOG_DEBUG:
			return "Debug";
			break;
		case LOG_VERBOSE:
			return "Verbose";
			break;
		case LOG_INFO:
			return "Info";
			break;
		case LOG_WARNING:
			return "Warning";
			break;
		case LOG_ERROR:
			return "Error";
			break;
		case LOG_FATAL:
			return "Fatal Error";
			break;
		default:
			return "Unkown";
			break;
	}
}

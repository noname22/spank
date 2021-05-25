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
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

Log* Log::logInstance = NULL;

Log* Log::getInstance()
{
	if(logInstance == NULL)
		logInstance = new Log();

	return logInstance;
}

void Log::setLogLevel(int inLogLevel)
{
	logLevel = inLogLevel;
}

Log::Log()
{
	logLevel = LOG_INFO;
}

void Log::restrictDebugOutputToFiles(std::vector<std::string> files)
{
	for(std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		if(*it != "")
			restrictFiles.push_back(*it);
}

void Log::log(std::string file, std::string function, int line, std::string msg, int severity)
{
	// if debug output is restricted to certain files, make sure that the output is from the correct file
	if(severity == LOG_DEBUG && restrictFiles.size() != 0){
		bool found = false;
		for(std::vector<std::string>::iterator it = restrictFiles.begin(); it != restrictFiles.end(); it++){
			if(*it == file){
				found = true;
				break;
			}
		}

		if(!found)
			return;
	}
	
	if(severity >= logLevel){
		std::ostream* out = severity >= LOG_WARNING ? &std::cerr : &std::cout;

		if(severity >= LOG_WARNING || logLevel <= LOG_EXTRA_VERBOSE)
		{
			auto time = std::time(nullptr);
			*out << "[ " << std::left << std::setw(7) << getSeverityName(severity) << " ] " << std::put_time(std::localtime(&time), "%F %T%z") << " ";
		}

		if(logLevel <= LOG_EXTRA_VERBOSE){
			std::stringstream ss;
			ss << file << ":" << line << " " << function;
			*out << std::left << std::setw(50) << ss.str();
		}

		*out << msg << std::endl;
	}
}

std::string Log::getSeverityName(int severity)
{
	std::string names[] = { "Debug", "Extra", "Verbose", "Info", "Warning", "Error", "Fatal" };

	if(severity >= 0 && severity <= LOG_FATAL)
		return names[severity];

	return "???";
}

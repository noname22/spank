// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef FILES_H
#define FILES_H

#include <string>
#include <ctime>
#include <utility>
#include <vector>

#include "project.h"

class Files
{
	public:
		// Pure virtual functions

		virtual bool listDir(std::string dir) = 0;
		
		virtual time_t getDate(std::string file) = 0;
		virtual bool fileExists(std::string file) = 0;
		
		virtual int isDir(std::string path) = 0;
		
		virtual bool createDir(std::string dir) = 0;
		virtual bool removeDir(std::string dir) = 0;
		virtual std::string getHomeDir() = 0;
		virtual std::string getTmpDirStr() = 0;
		virtual std::string getGlobalTmpDir() = 0;
		virtual bool checkRecompilePp(std::string src) = 0;
		virtual void markRecompilePp(std::string src) = 0;
		virtual void wait() = 0;
		virtual std::pair<std::string, std::string> pathSplit(std::string path) = 0;
		virtual bool copy(std::string from, std::string to) = 0;

		virtual std::string dirName(std::string filename) = 0;
		virtual std::string baseName(std::string filename) = 0;
		virtual std::string combinePath(std::vector<std::string> p) = 0;

		// Actual functions

		Files();
		virtual ~Files();
		
		virtual bool checkRecompile(std::string src, std::string obj);

		virtual void initializeTmpDir();
		virtual std::string getTmpDir();
		virtual void prepareTmpDir(std::string dir);
		virtual bool erase(std::string fileName);

		virtual std::string strFromFile(std::string fileName);

		virtual bool fileFromStr(std::string fileName, std::string str);

	private:
		bool once;

};

#endif

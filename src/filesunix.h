// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef FILESUNIX_H
#define FILESUNIX_H

#include "files.h"
#include "tools.h"

class FilesUnix: public Files
{
	public:

		void genSourceFileList(std::string dir);
		
		time_t getDate(std::string file);
		bool fileExists(std::string file);
		
		int isDir (std::string path);
		bool createDir(std::string dir);
		std::string getHomeDir();
		std::string getTmpDirStr();
		std::string getGlobalTmpDir();
		bool removeDir(std::string dir);
		bool copy(std::string from, std::string to);
		
		std::pair<std::string, std::string> pathSplit(std::string path);
		
		void wait();

		std::string dirName(std::string filename);
		std::string baseName(std::string filename);
		
		std::string combinePath(std::vector<std::string> p);
		std::string realpath(std::string filename);
		int chdir(std::string dir);

	private:
		bool find(std::string what, std::string where, std::string result);
};

#endif


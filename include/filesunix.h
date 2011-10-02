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


class FilesUnix: public Files
{
	public:

		bool listDir(std::string dir);
		
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

		bool checkRecompilePp(std::string src);
		void markRecompilePp(std::string src);
		
		std::string dirName(std::string filename);
		std::string baseName(std::string filename);
		
		std::string combinePath(std::vector<std::string> p);

	private:
		bool find(std::string what, std::string where, std::string result);

		bool writeMd5(std::string src, std::string md5sum);
		/*std::string readMd5(std::string sumFile);*/
		std::string getMd5(std::string src);

};

#endif


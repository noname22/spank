// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "files.h"

#include <fstream>
#include <cstdio>
#include <cstdlib>

#include "log.h"

Files::Files()
{
	once = false;
}

Files::~Files()
{
}

bool Files::fileFromStr(std::string fileName, std::string str)
{
	std::ofstream file(fileName.c_str());
	file << str;
	file.close();
	return true;
}

std::string Files::strFromFile(std::string fileName)
{
	std::string ret;

	std::ifstream in(fileName.c_str());

	int tmp;
	while((tmp = in.get()) != EOF){
		ret.push_back((char)tmp);
	}

	in.close();

	return ret;
}

bool Files::checkRecompile(std::string src, std::string obj)
{
	if(getDate(src) <= getDate(obj)){
		return false;
	}
	return true;
}

std::string Files::getTmpDir()
{
	std::string tmp;

	if(PROJECT->getValueStr("tmpdir") != ""){
		tmp = PROJECT->getValueStr("tmpdir");
		//LOG("Using user specified temp-dir: " << tmp, LOG_DEBUG);
	}else if(PROJECT->getValueStr("action") == "export"){
		tmp = getGlobalTmpDir();
		//LOG("Exporting, so using global temp-dir: " << tmp, LOG_DEBUG);
	}else{
		tmp = getTmpDirStr();
		//LOG("Using default temp-dir: " << tmp, LOG_DEBUG);
	}

	if(!once || !isDir(tmp)){
		prepareTmpDir(tmp);
		once = true;
	}

	return tmp;
}


void Files::prepareTmpDir(std::string dir)
{
	if(!createDir(dir)){
		LOG("Couldn't create tmp-dir", LOG_FATAL);
		exit(1);
	}

	if(!listDir(dir)){
		LOG("Couldn't locate all sources files", LOG_FATAL);
		exit(1);
	}
}

bool Files::erase(std::string fileName)
{
	if(remove(fileName.c_str()) == 0){
		return true;
	}
	return false;
}


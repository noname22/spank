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
	
void Files::initializeTmpDir()
{
	std::string tmp;

	if(PROJECT->getValueStr("tmpdir") != ""){
		tmp = PROJECT->getValueStr("tmpdir");
	}else if(PROJECT->getValueStr("action") == "export"){
		tmp = getGlobalTmpDir();
	}else{
		tmp = getTmpDirStr();
	}

	if(!isDir(tmp)){
		prepareTmpDir(tmp);
		LOG("created directory: " << tmp, LOG_VERBOSE);
	}

	PROJECT->setValue("tmpdir", tmp);
}

std::string Files::getTmpDir()
{
	return PROJECT->getValueStr("tmpdir");
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


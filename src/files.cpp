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
#include "macros.h"
#include "tools.h"

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
	LOG("files check recompile '" << src << "' ... '" << obj << "'", LOG_DEBUG);	

	if(!fileExists(obj)){
		LOG("object doesn't exist -> recompile", LOG_DEBUG);
		return true;
	}

	auto srcDate = getDate(src);
	auto objDate = getDate(obj);

	LOG("source date: " << srcDate, LOG_DEBUG);
	LOG("object date: " << objDate, LOG_DEBUG);

	if(srcDate <= objDate){
		LOG("source is older than object -> don't recompile", LOG_DEBUG);
		return false;
	}

	LOG("object is older than source -> recompile", LOG_DEBUG);

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
		AssertEx(createDir(tmp), FilesException, "Could not create temp directory: " << tmp);
		LOG("created directory: " << tmp, LOG_EXTRA_VERBOSE);
	}

	PROJECT->setValue("tmpdir", tmp);
}

std::string Files::getTmpDir()
{
	return PROJECT->getValueStr("tmpdir");
}

bool Files::erase(std::string fileName)
{
	if(remove(fileName.c_str()) == 0){
		return true;
	}
	return false;
}

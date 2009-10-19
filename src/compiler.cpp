// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "compiler.h"
#include "system.h"
#include "tools.h"

#include <cstring>
#include <cstdlib>

int Compiler::countLines(std::string name)
{
	std::ifstream file(name.c_str());
	char tmp[8];
	int count=0;

	while(file.good()){
		file.getline(tmp, 8);
		if(strlen(tmp) != 0){
			count++;	
		}
	}	
	return count;
}

bool Compiler::link()
{
	if(!localLink()){
		LOG("Returning error", LOG_ERROR);
		return false;
	}

	LOG("Running post-build scripts", LOG_VERBOSE);
	return Tools::executeAll("postbuildscript");
}

bool Compiler::compile()
{
	LOG("Running pre-build scripts", LOG_VERBOSE);

	if(!Tools::executeAll("prebuildscript")){
		return false;
	}

	return localCompile();
}

bool Compiler::clean()
{
	LOG("Running on clean scripts", LOG_VERBOSE);
	if(!Tools::executeAll("oncleanscript")){
		return false;
	}

	return localClean();
}

bool Compiler::localClean()
{
	std::string tmp = FILES->getTmpDir();
	std::string target = PROJECT->getValueStr("target", 0);

	if(FILES->isDir(tmp.c_str())){
		// TODO HACK
		std::string cmd("rm -rf ");
		cmd.append(tmp);
		system(cmd.c_str());
	}
	
	if(FILES->fileExists(target)){
		remove(target.c_str());
	}
	return true;
}

bool Compiler::checkLibs()
{
	LOG("Compiler doesn't check libs", LOG_DEBUG);
	return true;
}

bool Compiler::checkExclude(std::string src)
{
	for(int i=0; i < PROJECT->getNumValues("exclude"); i++){
		if(src == PROJECT->getValueStr("exclude", i)){
			LOG("Excluding " << src << " from build.", LOG_VERBOSE);
			return true;
		}
	}
	return false;
}

Compiler::~Compiler()
{
}

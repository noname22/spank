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
#include "macros.h"

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

	LOG("Running post-build scripts", LOG_EXTRA_VERBOSE);
	return Tools::executeAll("postbuildscript");
}

std::string Compiler::genCFlags(std::string filename, bool includeLibs, std::string language)
{
	std::string hyphen = PROJECT->getValueBool("addhyphen") ? " -" : " ";
	return PROJECT->getValueStr("cflags", hyphen, hyphen, " ");
}

bool Compiler::buildDeps()
{
	std::string cfgVal = "depends";

	if(PROJECT->getNumValues(cfgVal) > 0)
		LOG("Building external projects", LOG_VERBOSE);

	for(int i = 0; i < PROJECT->getNumValues(cfgVal); i++){
		std::string dep = PROJECT->getValueStr(cfgVal, i);
		LOG(dep, LOG_VERBOSE);

		std::string currPath = FILES->realpath(".");

		LASSERT(FILES->chdir(dep) == 0, "couldn't change into dependency directory into: " << dep);

		std::string err;
		FORMSTR(cmd, PROJECT->getValueStr("spank") << " -verbosity 3 -dep_printinfo yes " << PROJECT->getValueStr("depaction"));
		Tools::execute(cmd, 0, &err, false);
		LOG("err: " << err, LOG_DEBUG);
	
		StrVec lines = Tools::splitString(err, '\n');

		for(StrVec::iterator it = lines.begin(); it != lines.end(); it++){
			StrVec infoTypes = Tools::makeStrVector("target", "targettype", "cflags", "ldflags");

			for(StrVec::iterator it2 = infoTypes.begin(); it2 != infoTypes.end(); it2++){
				FORMSTR(lineStart, *it2 << ":");
				std::string val = Tools::restOfString(*it, lineStart);
				if(val != ""){
					FORMSTR(valName, "_dep_" << *it2)
					LOG(valName << " -> " << Tools::trim(val), LOG_DEBUG);
					PROJECT->addValue(valName, Tools::trim(val));
				}
			}
		} 

		if(FILES->chdir(currPath) != 0){
			LOG("couldn't change into directory: " << currPath, LOG_FATAL);
			exit(1);
		}
	}

	return true;
}

bool Compiler::compile()
{
	if(!buildDeps())
		return false;

	LOG("Running pre-build scripts", LOG_EXTRA_VERBOSE);

	if(!Tools::executeAll("prebuildscript"))
		return false;

	return localCompile();
}

bool Compiler::clean()
{
	LOG("Running on clean scripts", LOG_EXTRA_VERBOSE);
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
			LOG("Excluding " << src << " from build.", LOG_EXTRA_VERBOSE);
			return true;
		}
	}
	return false;
}

Compiler::~Compiler()
{
}

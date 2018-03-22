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
#include <sstream>

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

void Compiler::link()
{
	localLink();
	LOG("Running post-build scripts", LOG_EXTRA_VERBOSE);
	AssertEx(Tools::executeAll("postbuildscript"), CompilerException, "could not run post-build scripts");
}

std::string Compiler::genCFlags(std::string filename, bool includeLibs, std::string language)
{
	std::string hyphen = PROJECT->getValueBool("addhyphen") ? " -" : " ";
	return PROJECT->getValueStr("cflags", hyphen, hyphen, " ");
}

void Compiler::buildDeps()
{
	std::string cfgVal = "depends";

	if(PROJECT->getNumValues(cfgVal) > 0)
		LOG("Building external projects", LOG_VERBOSE);

	for(int i = 0; i < PROJECT->getNumValues(cfgVal); i++){
		std::string str = PROJECT->getValueStr(cfgVal, i);
		std::vector<std::string> vals = Tools::splitString(str, ':', 1);

		AssertEx(vals.size() > 0, CompilerException, "malformed dependency description (" << str << ")");
		
		std::string dep = vals[0];
		LOG(dep, LOG_VERBOSE);

		std::string currPath = FILES->realpath(".");
		AssertEx(currPath != "", CompilerException, "could not determine current directory!");

		AssertEx(FILES->chdir(dep) == 0, CompilerException, "couldn't change into dependency directory into: " << dep);

		std::string err;
		std::string extra = vals.size() > 1 ? vals[1] : "";

		FORMSTR(cmd, 
			PROJECT->getValueStr("spank") << 
			" -verbosity 3 -dep_printinfo yes " << 
			" -host '"              << Tools::escape(PROJECT->getValueStr("host")) << "'" <<
			" -binary_prefix '"     << Tools::escape(PROJECT->getValueStr("binary_prefix")) << "'" <<
			" -binary_suffix '"     << Tools::escape(PROJECT->getValueStr("binary_suffix")) << "'" <<
			" -lib-shared_prefix '" << Tools::escape(PROJECT->getValueStr("lib-shared_prefix")) << "'" <<
			" -lib-shared_suffix '" << Tools::escape(PROJECT->getValueStr("lib-shared_suffix")) << "'" <<
			" -inst_prefix '"       << Tools::escape(PROJECT->getValueStr("inst_prefix")) << "'" <<
			" -fpic '"              << Tools::escape(PROJECT->getValueStr("fpic")) << "'" <<

			" " << PROJECT->getValueStr("depaction") << 
			" " << extra);

		if(Tools::execute(cmd, 0, &err, false) != 0){
			std::cerr << err;
			ThrowEx(CompilerException, "failed to build external project: " << dep);
		}

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

		AssertEx(FILES->chdir(currPath) == 0, CompilerException, "couldn't change into directory: " << currPath);
	}
}

void Compiler::compile()
{
	FILES->genSourceFileList(PROJECT->getValueStr("tmpdir"));
	localCompile();
}

void Compiler::clean()
{
	LOG("Running on clean scripts", LOG_EXTRA_VERBOSE);
	AssertEx(Tools::executeAll("oncleanscript"), CompilerException, "Could not execute on-clean scripts");
	localClean();
}

void Compiler::localClean()
{
	std::string tmp = FILES->getTmpDir();
	std::string target = PROJECT->getValueStr("target", 0);

	if(FILES->isDir(tmp.c_str())){
		FILES->removeDir(tmp);
	}
	
	if(FILES->fileExists(target)){
		remove(target.c_str());
	}
}

bool Compiler::checkLibs()
{
	LOG("Compiler doesn't check libs", LOG_DEBUG);
	return true;
}

Compiler::~Compiler()
{
}

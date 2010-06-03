// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "compilers/compilergcc.h"

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#include "system.h"
#include "settings.h"
#include "tools.h"
#include "macros.h"

// Makes a list of all source files in the project.
// If rcCheck is true it only returns the files that need recompilation
CompilerGcc::CompilerGcc()
{
	hasPkgConfig = false;
}

std::vector<CList> CompilerGcc::compileList(bool rcCheck)
{
	std::vector<CList> cList;

	char line[SPANK_MAX_LINE];	
	std::string open = FILES->getTmpDir();
	std::string objListName = open;

	open.append("/filelist");
	std::ifstream list(open.c_str());
	
	objListName.append("/objectlist");
	std::ofstream objectList(objListName.c_str());

	int count=0, srcCount=0;
	
	cList.clear();
	std::vector<std::string> dupCheck;
	bool noDup=true;

	while(list.good()){
		srcCount++;
		list.getline(line, SPANK_MAX_LINE);
		if(strlen(line) != 0){
			
			std::string obj = FILES->getTmpDir();
			obj.append("/");
			obj.append(Tools::nameEnc(".o", line));

			noDup = true;
			for(int i=0; i < (int)dupCheck.size(); i++){
				if(obj == dupCheck.at(i)){
					noDup = false;
				}
			}
			
			if(noDup){
				dupCheck.push_back(obj);
				std::string src = line;
		
				if(!checkExclude(src)){	
					objectList << obj << std::endl;
			
					if(!rcCheck || checkRecompile(src, obj)){
						CList tmp;
						
						count++;
						std::string call;

						call = PROJECT->getValueStr("compiler", 0);
						call.append(" ");
						call.append(Tools::genCFlags());

						for(int p = 0; p < PROJECT->getNumValues("lib"); p++){
							call.append(pkgGetFlags(PROJECT->getValueStr("lib", p), true));
						}

						if(PROJECT->getValueStr("targettype") == "lib-shared"){
							call.append(PROJECT->getValueStr("fpic"));
							call.append(" ");
						}

						call.append("-c -o ");
						call.append(obj);
						call.append(" ");
						call.append(src);
						
						tmp.call = call;
						tmp.src = src;
						tmp.obj = obj;
						
						cList.push_back(tmp);
					}
				}
			}
		}
	}

	// TODO HACK
	if(srcCount == 1){
		LOG("Nothing to do.", LOG_INFO);
		exit(0);
	}

	return cList;	
}

bool CompilerGcc::checkRecompile(std::string src, std::string obj)
{
	// wrapper from files-function depending on method
	// Rename it? To what?

	if(PROJECT->getValueStr("rccheck", 0) == "pp"){
		return (FILES->checkRecompilePp(src));
	}else{
		return FILES->checkRecompile(src, obj);
	}
}

void CompilerGcc::markRecompile(std::string src, std::string obj)
{
	FILES->erase(obj);
	if(PROJECT->getValueStr("rccheck", 0) == "pp"){
		FILES->markRecompilePp(src);
	}
}

std::string CompilerGcc::getLdCall(bool rlCheck)
{
	std::stringstream call;
	bool link=false;

	std::string target = PROJECT->getValueStr("target");
	std::string targettype = PROJECT->getValueStr("targettype");

	if(targettype == "lib-static"){
		call << PROJECT->getValueStr("ar") << " rcs lib" << target << ".a ";
	}else if(targettype == "lib-shared"){
		call << PROJECT->getValueStr("compiler") << " -shared -o " << target << " ";
	}else{
		if(targettype != "binary"){
			LOG("unknown target type: '" << targettype << "', assuming binary", LOG_WARNING);
			targettype = "binary";
		}
		call << PROJECT->getValueStr("compiler") << " -o " << target << " ";
	}
	
	
	std::string open = FILES->getTmpDir();
	
	open.append("/objectlist");
	std::ifstream list(open.c_str());
	
	bool first=true;

	char line[SPANK_MAX_LINE];

	while(list.good()){
		list.getline(line, SPANK_MAX_LINE);

		// If the target is newer than all the object files, don't link
		if(!rlCheck || FILES->checkRecompile(line, target)){
			link = true;
		}

		if(strlen(line) != 0){
			if(first){
				first = false;
			}else{
				call << " ";
			}
			call << line;
		}
	}

	if(PROJECT->getValueBool("addhyphen")){
		call << " " << PROJECT->getValueStr("ldflags", "-", " -", " ");
	}else{
		call << " " << PROJECT->getValueStr("ldflags", " ", " ", " ");
	}

	if(targettype == "binary"){
		
		for(int p = 0; p < PROJECT->getNumValues("lib"); p++){
			call << pkgGetFlags(PROJECT->getValueStr("lib", p), false);
		}
		
		LOG("ldcall: " << call.str(), LOG_DEBUG);
	}
		
	if(link){
		return call.str();
	}else{
		return "";
	}
}

std::string CompilerGcc::pkgGetFlags(std::string lib, bool cflags)
{
	std::string call;

	if(cflags){
		LOG("Getting cflags from pkg-config", LOG_DEBUG);
		call = "--cflags ";
	}else{
		LOG("Getting ldflags from pkg-config", LOG_DEBUG);
		call = "--libs ";
	}

	call.append(lib);

	if(!hasPkgConfig){
		LOG("No pkg-config, ignoring lib", LOG_DEBUG);
		return "";
	}

	if(!pkgCall(call)){
		LOG("There was an error calling pkg-config", LOG_ERROR);
		LOG("Ignoring library '" << lib << "'", LOG_WARNING);
		return "";
	}else{
		std::string result = FILES->getTmpDir();
		std::string ret;

		result.append("/pkgresult");
		std::ifstream in(result.c_str());
		getline(in, ret);
		ret.append(" ");
		in.close();
		LOG("pkg-config returned this as flags: " << ret, LOG_DEBUG);
		return ret;
	}
}

bool CompilerGcc::pkgCall(std::string switches)
{
	std::stringstream call;
	call << PROJECT->getValueStr("pkg-config") << " " << switches << " 2> " << FILES->getTmpDir() << "/pkgresult >> " << FILES->getTmpDir() << "/pkgresult";
	LOG("Calling pkg-config with: '" << call.str().c_str() << "'", LOG_DEBUG);
	int ret = system(call.str().c_str());
	LOG("pkg-config returned " << ret, LOG_DEBUG);
	return !ret;
}

bool CompilerGcc::checkLibs()
{
	if(PROJECT->getNumValues("lib") > 0){
		if(!pkgCall("--atleast-pkgconfig-version=0.20")){
			LOG("Spank requires pkg-config 0.20 or higher to handle library dependencies", LOG_ERROR);
			LOG("Ignoring all libraries", LOG_WARNING);
			hasPkgConfig = false;
			return true;
		}else{
			hasPkgConfig = true;
		}
	}

	for(int i=0; i < PROJECT->getNumValues("lib"); i++){
		if(!pkgCall(PROJECT->getValueStr("lib", i))){
			LOG("Missing library '" << PROJECT->getValueStr("lib", i) << "'", LOG_ERROR);
			return false;
		}
	}
	return true;
}

bool CompilerGcc::localLink()
{
	std::string call = getLdCall(true);
	if(call != ""){
		LOG("Linking...", LOG_INFO);
		LOG(call, LOG_VERBOSE);
		if(!system(call.c_str())){
			return true;
		}else{
			return false;
		}
	}else{
		LOG("Target up to date.", LOG_INFO);
		return true;
	}
}
	
std::string CompilerGcc::getPercent(int current, int of)
{
	std::string ret;
	std::stringstream tmp;
	tmp << ((int)(((double)(current+1) / (double)of * 100 )));

	if(tmp.str().length() < 3){
		for(int i=0; i < 3 - (int)tmp.str().length(); i++){
			ret.append(" ");
		}
	}
	ret.append(tmp.str());
	return ret;
}

bool CompilerGcc::localCompile()
{
	LOG("Checking dependencies...", LOG_INFO);
	if(!checkLibs()){
		return false;
	}

	LOG("Preparing...", LOG_INFO);
	std::vector<CList> cList = compileList();

	if(cList.size() > 0){
		LOG("Compiling...", LOG_INFO);
	}else{
		LOG("Nothing to compile.", LOG_VERBOSE);
		return true;
	}
	
	ForkResult handle;
	PidList pidList;

	int numJobs = PROJECT->getValueInt("jobs") - 1;
	bool ret = true;

	LOG("Compiling with " << numJobs + 1 << " jobs.", LOG_VERBOSE);

	for(int i=0; i < (int)cList.size(); i++){
		LOG("[ " << getPercent(i, (int)cList.size()) << " %] " << Tools::stripSrcDir(cList.at(i).src), LOG_INFO);
		
		LOG(cList.at(i).call.c_str(), LOG_VERBOSE);
	
		if(FILES->fileExists(cList.at(i).obj)){
			LOG("Removing old object file: '" << cList.at(i).obj << "'", LOG_DEBUG);
			FILES->erase(cList.at(i).obj);
		}
	
		Tools::forkDo(cList.at(i).call.c_str(), pidList, i);
		//Tools::forkDo("../returner/returner 2" ,pidList, i);
		handle = Tools::wait(pidList, numJobs);	

		if(!handle.noResult && handle.error){
			LOG("Compilation failed, removing associated files", LOG_DEBUG);
			markRecompile(cList.at(handle.id).src, cList.at(handle.id).obj);
			ret = false;
			break;
		}

		/*if(system(cList.at(i).call.c_str())){
			LOG("Compilation failed, removing associated files", LOG_DEBUG);
			markRecompile(cList.at(i).src, cList.at(i).obj);
			return false;
		}*/
	}

	while(!handle.done){
		handle = Tools::wait(pidList, 0);
		if(!handle.noResult && handle.error){
			LOG("Compilation failed, removing associated files", LOG_DEBUG);
			markRecompile(cList.at(handle.id).src, cList.at(handle.id).obj);
			ret = false;
			break;
		}
	}

	return ret;
}

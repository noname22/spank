// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "compilers/compilervala.h"

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <cstring>

#include "system.h"
#include "settings.h"
#include "tools.h"

bool CompilerVala::localLink()
{
	return true;
}

// link = compile with vala
std::string CompilerVala::getLdCall(bool rlCheck)
{
	std::vector<CList> cList;

	char line[SPANK_MAX_LINE];	
	std::string open = FILES->getTmpDir();

	open.append("/filelist");
	std::ifstream list(open.c_str());
	
	std::stringstream call;
	
	std::vector<std::string> dupCheck;
	bool noDup=true;

	std::string compiler = PROJECT->getValueStr("compiler", 0);

	std::string cflags;

	if(PROJECT->getValueBool("addhyphen")){
		cflags = PROJECT->getValueStr("cflags", "-", " -", " ");
	}else{
		cflags = PROJECT->getValueStr("cflags", " ", " ", " ");
	}

	std::string target   = PROJECT->getValueStr("target", 0);

	call << compiler << " " << cflags << " -o " << target;

	LOG("Files in project:", LOG_INFO);

	while(list.good()){
		list.getline(line, SPANK_MAX_LINE);
		std::string src = line;
		if(strlen(line) != 0){
			noDup = true;
			std::string src = line;
			for(int i=0; i < (int)dupCheck.size(); i++){
				if(src == dupCheck.at(i)){
					noDup = false;
				}
			}
			
			if(noDup){
				if(!checkExclude(src)){	
					call << " " << src;
					LOG(src, LOG_INFO);
				}
				dupCheck.push_back(src);
			}
		}
	}

	return call.str();
}

std::vector<CList> CompilerVala::compileList(bool rcCheck)
{
	std::vector<CList> tmp;
	return tmp;
}

bool CompilerVala::localCompile()
{
	std::string call = getLdCall(false);
	LOG("Compiling and linking...", LOG_INFO);
	LOG(call, LOG_VERBOSE);
	if(call != "" && system(call.c_str()) == 0){
		return true;
	}
	return false;
}




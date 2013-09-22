// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "compilergcc.h"

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <set>

#include "system.h"
#include "settings.h"
#include "tools.h"
#include "macros.h"

CompilerGcc::CompilerGcc()
{
	hasPkgConfig = false;
}

// Makes a list of all source files in the project.
std::set<std::string> CompilerGcc::getSourceList()
{
	std::ifstream listFile(FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), "filelist")).c_str());
	std::set<std::string> list;

	while(listFile.good()){
		std::string line;
		getline(listFile, line);

		if(line != "" && !checkExclude(line))
			list.insert(line);
		
	}

	listFile.close();

	return list;
}

// Returns a vector with compile commands to be executed, eg gcc -c file.c -o file.o
// If rcCheck is true it only returns the files that need recompilation
std::vector<CList> CompilerGcc::compileList(bool rcCheck)
{
	std::vector<CList> cList;
	std::ofstream objectList(FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), "objectlist")).c_str());
	std::set<std::string> list = getSourceList();

	for(std::set<std::string>::iterator it = list.begin(); it != list.end(); it++){
		std::string obj = FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), Tools::nameEnc(".o", *it)));
		objectList << obj << std::endl;

		if(!rcCheck || checkRecompile(*it, obj)){
			CList tmp;
			
			std::stringstream call;

			call << PROJECT->getValueStr("compiler", 0) << " " << genCFlags(*it) << " -c " << *it << " ";

			if(PROJECT->getValueStr("targettype") == "lib-shared")
				call << PROJECT->getValueStr("fpic") << " ";

			call << "-o " << obj;
			
			tmp.call = call.str();
			tmp.src = *it;
			tmp.obj = obj;
			
			cList.push_back(tmp);
		}
	}

	// TODO HACK
	if(list.size() == 0){
		LOG("Nothing to do.", LOG_INFO);
		exit(0);
	}

	return cList;	
}

void CompilerGcc::setIncludePaths(std::string filename)
{
	static std::string lastLang = "";
	std::string language = guessLanguage(filename);

	// re-scan include paths if there's a new language type
	// since c/c++ have different built in include paths

	if(lastLang != language){
		incPaths[0].clear();
		incPaths[1].clear();
		lastLang = language;
	}

	if(incPaths[Bracket].size())
		return;

	std::string dashx = guessLanguage(filename) == "c++" ? " -x c++" : "";

	FORMSTR(cmd, "echo | " << PROJECT->getValueStr("pp") << " " << genCFlags(filename) << dashx << " - -v");
	std::string out;

	Tools::execute(cmd, 0, &out);
	std::stringstream s(out);

	int panic = 0x10000, c = 0, i = -1;
	try{
		// Read include directories
		for(;;){
			if(c++ > panic){ 
				LOG("No End of search list found after " << c << " lines, bailing." , LOG_EXTRA_VERBOSE);
				throw std::runtime_error("Unexpected output from the preprocessor");
			}

			std::string line = Tools::getLineStream(s);

			LOG(line, LOG_DEBUG);

			if(line == "#include <...> search starts here:" || line == "#include \"...\" search starts here:"){ i++; continue; }
			if(line == "End of search list."){ break; }
			if(i >= 0){ incPaths[i].push_back(line.substr(1)); }
			
			if(!s.good()){
				throw std::runtime_error("EOF without End of search list");
			}
		}
	}
	
	catch(std::runtime_error e)
	{
		LOG(e.what(), LOG_FATAL);
		exit(1);
	}

	LOG("Found include paths: ", LOG_DEBUG);
	for(i = 0; i < 2; i++){
		for(std::vector<std::string>::iterator it = incPaths[i].begin(); it != incPaths[i].end(); it++){
			LOG((*it), LOG_DEBUG);
		}
	}
}

std::string CompilerGcc::lookUpIncludeFile(std::string src, std::string filename, bool quoted)
{
	IncPathType order[2] = {Bracket, Quoted};

	std::vector<std::string> incPaths[2] = this->incPaths;
	//LOG(src << " -> " << FILES->dirName(src), LOG_DEBUG);
	incPaths[Quoted].insert(incPaths[0].begin(), FILES->dirName(src)); 	

	if(quoted){
		order[0] = Quoted; order[1] = Bracket;
	}

	for(int i = 0; i < 2; i++){
		for(std::vector<std::string>::iterator it = incPaths[order[i]].begin(); it != incPaths[order[i]].end(); it++){
			FORMSTR(test, (*it) << "/" << filename);
			LOG("Trying: " << test, LOG_DEBUG);
			if(FILES->fileExists(test)){
				LOG("exists", LOG_DEBUG);
				return test;
			}
		}
	}

	throw std::runtime_error("no file");
}

// checks if a file needs recompilation by parsing all include directives found and checking the date of the files
// TODO line breaks in C++ style comments, '\'
// TODO trigraph line breaks, although they could probably be safely ignored

bool CompilerGcc::checkRecompileRecursive(std::vector<std::string> stack, std::string src, std::string obj, int depth)
{
	stack.push_back(src);

	LOG("now checking: " << src << " for object " << obj << " at depth " << depth, LOG_DEBUG);

	if(FILES->checkRecompile(src, obj)){
		return true;
	}

	if(depth > SPANK_MAX_RECURSE){
		LOG("Recusive recompile checker exceeded maximum depth (" << SPANK_MAX_RECURSE << ")", LOG_WARNING);
		LOG("Circular include?", LOG_WARNING);
		return false;
	}

	setIncludePaths(src);

	std::fstream f(src.c_str());

	int lineNum = 1;
	bool inComment = false;

	while(f.good()){
		bool isInclude = false, localFirst = false;

		std::string line = Tools::getLineStream(f);


		// Cut out C style comments
		
		size_t commentStart = line.find("/*");
		size_t commentEnd = line.find("*/");

		if(inComment){
			commentStart = 0;
		}else if(commentStart != std::string::npos){
			inComment = true;
		}

		if(inComment){
			if(commentEnd == std::string::npos){
				commentEnd = line.size() - 3;
			}else{
				inComment = false;
			}

			//line = line.substr(commentStart, commentEnd + 2);
			line.erase(commentStart, commentEnd + 2 - commentStart);
		}

		std::stringstream s(line);

		std::string parse;

		s >> parse;

		if(parse == "#"){
			// check for: "#    include"
			s >> parse;
			if(parse == "include"){
				isInclude = true;
			}
		}else if(parse == "#include"){
			// check for: "#include"
			isInclude = true;
		}
	
		if(isInclude){
			parse = Tools::getLineStream(s);

			size_t first = parse.find_first_of("\"<"); 
			size_t last = parse.find_last_of("\">"); 

			//if(first || last) ... wat. why was this here?
			if(parse.size() < 3 || first == std::string::npos || last == std::string::npos){
				LOG("Recursive recompile checker couldn't parse include directive, syntax error in " << src << ":" << lineNum << "?", LOG_WARNING);
				LOG("Line: " << line, LOG_EXTRA_VERBOSE);
				continue;
			}

			if(parse.at(first) == '\"'){
				localFirst = true;
			}

			std::string filename = parse.substr(first + 1, last - 2);
			try { filename = lookUpIncludeFile(src, filename, localFirst); }

			catch(...){
				LOG("Recursive compile checker coulnd't find the included file: '" << filename << "' (at line " << lineNum << ")", LOG_EXTRA_VERBOSE);
			}

			// Check for circluar includes
			// TODO windows implications with case insensitivity
			try {
				for(std::vector<std::string>::iterator it = stack.begin(); it != stack.end(); it++){
					if(filename == (*it)){
						throw std::runtime_error("circular");
					}
				}

				if(checkRecompileRecursive(stack, filename, obj, depth + 1)){
					return true;
				}
			} catch (std::runtime_error) {
				LOG("The file " << filename << " is includes itself (directly or indirectly).", LOG_EXTRA_VERBOSE);
				LOG("current include stack: ", LOG_EXTRA_VERBOSE);
				for(std::vector<std::string>::iterator it = stack.begin(); it != stack.end(); it++){
					LOG(*it, LOG_EXTRA_VERBOSE);
				}
				LOG("then trying to include: " << filename, LOG_EXTRA_VERBOSE);
			}
		}

	
		lineNum++;	
	} 

	return false;
}

bool CompilerGcc::checkRecompile(std::string src, std::string obj)
{
	// wrapper from files-function depending on method
	// Rename it? To what?

	std::string language = guessLanguage(src);

	if(PROJECT->getValueStr("rccheck") == "recursive" && (language == "c++" || language == "c")){
		std::vector<std::string> stack;
		return checkRecompileRecursive(stack, src, obj);
	}

	return FILES->checkRecompile(src, obj);
}

void CompilerGcc::markRecompile(std::string src, std::string obj)
{
	FILES->erase(obj);
}

std::string CompilerGcc::getLdCall(bool rlCheck)
{
	std::stringstream call;
	bool link=false;

	std::string target = PROJECT->getValueStr("target");
	std::string targettype = PROJECT->getValueStr("targettype");

	if(targettype == "lib-static"){
		call << PROJECT->getValueStr("ar") << " rcs " << target;
	}else if(targettype == "lib-shared"){
		call << PROJECT->getValueStr("linker") << " -shared -o " << target;
	}else{
		if(targettype != "binary"){
			LOG("unknown target type: '" << targettype << "', assuming binary", LOG_WARNING);
			targettype = "binary";
		}
		call << PROJECT->getValueStr("linker") << " -o " << target;
	}
	
	
	std::string open = FILES->getTmpDir();
	
	open.append("/objectlist");
	std::ifstream list(open.c_str());
	
	char line[SPANK_MAX_LINE];

	while(list.good()){
		list.getline(line, SPANK_MAX_LINE);

		// If the target is newer than all the object files, don't link
		if(!rlCheck || FILES->checkRecompile(line, target)){
			link = true;
		}

		if(strlen(line) != 0){
			call << " " << line;
		}
	}

	if(PROJECT->getValueBool("addhyphen")){
		call << " " << PROJECT->getValueStr("ldflags", "-", " -", " ");
	}else{
		call << " " << PROJECT->getValueStr("ldflags", " ", " ", " ");
	}

	if(targettype != "lib-static" && PROJECT->getNumValues("lib"))
		call << "`" << PROJECT->getValueStr("pkg-config") << PROJECT->getValueStr("lib", " --libs ", " ", "` ");
	
	if(targettype != "lib-static" && PROJECT->getNumValues("lib-static"))
		call << "`" << PROJECT->getValueStr("pkg-config") << PROJECT->getValueStr("lib-static", " --static --libs ", " ", "` ");

	call << PROJECT->getValueStr("_dep_ldflags", " ", " ", " ", true); 

	LOG("ldcall: " << call.str(), LOG_DEBUG);
		
	if(link){
		return call.str();
	}else{
		return "";
	}
}

// TODO, remove this
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
	if(PROJECT->getNumValues("lib") > 0 || PROJECT->getNumValues("lib-static")){
		if(!pkgCall("--atleast-pkgconfig-version=0.20")){
			LOG("Spank requires pkg-config 0.20 or higher to handle library dependencies", LOG_ERROR);
			LOG("Ignoring all libraries", LOG_WARNING);
			hasPkgConfig = false;
			return true;
		}else{
			hasPkgConfig = true;
		}
	}

	bool ret = true;

	for(int i=0; i < PROJECT->getNumValues("lib"); i++){
		if(!pkgCall(PROJECT->getValueStr("lib", i))){
			LOG("Missing library '" << PROJECT->getValueStr("lib", i) << "'", LOG_ERROR);
			ret = false;
		}
	}
	
	for(int i=0; i < PROJECT->getNumValues("lib-static"); i++){
		if(!pkgCall(PROJECT->getValueStr("lib-static", i))){
			LOG("Missing library '" << PROJECT->getValueStr("lib-static", i) << "'", LOG_ERROR);
			ret = false;
		}
	}
	return ret;
}

bool CompilerGcc::localLink()
{
	if(PROJECT->getValueStr("compilation-strategy") == "single-call")
		return true;
	
	std::string call = getLdCall(true);
	if(call != ""){
		LOG("Linking...", LOG_VERBOSE);
		LOG(call, LOG_EXTRA_VERBOSE);
		if(!system(call.c_str())){
			return true;
		}else{
			return false;
		}
	}else{
		LOG("Target up to date.", LOG_VERBOSE);
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
	std::string strategy = PROJECT->getValueStr("compilation-strategy");

	if(strategy == "single-call")
		return compileSingleCall();
	else if (strategy == "amalgamate")
		return compileAmalgamate();

	return compileFileByFile();
}

std::string CompilerGcc::guessLanguage(std::string filename)
{
	if(PROJECT->getValueStr("language") != "none")
		return PROJECT->getValueStr("language");

	// TODO guess more languages than c and c++ (if it's ever needed)
	std::string lowername = Tools::toLower(filename);

	if(Tools::endsWith(lowername, ".cpp") || Tools::endsWith(lowername, ".cxx") || Tools::endsWith(lowername, ".cc"))
		return "c++";

	if(Tools::endsWith(lowername, ".c"))
		return "c";
	
	return "none";
}

std::string CompilerGcc::genCFlags(std::string filename, bool includeLibs, std::string language)
{
	if(language == "")
		language = PROJECT->getValueStr("language");

	std::string hyphen = PROJECT->getValueBool("addhyphen") ? "-" : "";
	std::stringstream flags;
	
	for(int i = 0; i < PROJECT->getNumValues("cflags"); i++){
		std::string flag = PROJECT->getValueStr("cflags", i);

		// .c/std=c99 <- only if extension is .c
		std::string ext = Tools::restOfString(flag, ".");
		if(ext != ""){
			unsigned pos = ext.find("/");
			LASSERT(pos != std::string::npos, "per extension cflag expects syntax .[ext]/[flag], but no slash found");

			//LOG("only with extensions: " << ext.substr(0, pos), LOG_DEBUG);

			if(Tools::endsWith(filename, ext.substr(0, pos))){
				flags << hyphen << ext.substr(pos + 1) << " ";
				//LOG("sflag: " << ext.substr(pos + 1), LOG_DEBUG);	
			}
		}else{
			flags << flag << " ";
			//LOG("flag: " << flag, LOG_DEBUG);	
		}
	}

	flags << PROJECT->getValueStr("_dep_cflags", " ", " ", " ");

	if(PROJECT->getValueBool("spankdefs") || PROJECT->getValueStr("spankdefs") == "extra"){
		flags << "-D'SPANK_NAME=\"" << PROJECT->getValueStr("name") << "\"' ";
	}

	if(PROJECT->getValueStr("spankdefs") == "extra"){
		std::string compiler = PROJECT->getValueStr("compiler");

		flags << "-D'SPANK_TARGET_PLATFORM=\"" << PROJECT->getValueStr("target_platform") << "\"' ";
		flags << "-DSPANK_TARGET_PLATFORM_" << Tools::toUpper(PROJECT->getValueStr("target_platform")) << " ";

		flags << "-DSPANK_COMPILER_GCC ";
		flags << "-DSPANK_ENV_UNIX ";

		flags << "-D'SPANK_BINNAME=\"" << PROJECT->getValueStr("target") << "\"' ";
		flags << "-D'SPANK_VERSION=\"" << PROJECT->getValueStr("version") << "\"' ";
		flags << "-D'SPANK_HOMEPAGE=\"" << PROJECT->getValueStr("homepage", " - ") << "\"' ";
		flags << "-D'SPANK_AUTHOR=\"" << PROJECT->getValueStr("author", ", ") << "\"' ";
		flags << "-D'SPANK_EMAIL=\"" << PROJECT->getValueStr("email", " - ") << "\"' ";
		flags << "-D'SPANK_PREFIX=\"" << PROJECT->getValueStr("inst_prefix") << "\"' ";
	}

	flags << "-x " << language << " ";

	if(PROJECT->getNumValues("lib")){
		flags << " `" << PROJECT->getValueStr("pkg-config") <<
			PROJECT->getValueStr("lib", includeLibs ? " --cflags --libs " : " --cflags ", " ", "`");
	}
	
	if(PROJECT->getNumValues("lib-static")){
		flags << " `" << PROJECT->getValueStr("pkg-config") << 
			PROJECT->getValueStr("lib-static", includeLibs ? " --static --cflags --libs " : " --static --cflags ", " ", "`");
	}

	return flags.str();
}

bool CompilerGcc::compileAmalgamate()
{
	LOG("Amalgamating and compiling sources...", LOG_VERBOSE);

	std::string compiler = PROJECT->getValueStr("compiler");
	std::string language = PROJECT->getValueStr("language");

	LASSERT(language != "none", "Amalgamate doesn't work with language auto detection, must specify c or c++ explicitly");
	LASSERT(language == "c++" || language == "c", "Amalgamating compilation only works with c and c++");

	std::set<std::string> list = getSourceList();
	
	// TODO HACK
	if(list.size() == 0){
		LOG("Nothing to do.", LOG_INFO);
		exit(0);
	}
	
	std::ofstream objectList(FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), "objectlist")).c_str());
	std::string object = FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), "all_sources.o"));
	objectList << object << std::endl;

	std::stringstream vfile;

	std::set<std::string>::iterator it = list.begin();
	for(it = list.begin(); it != list.end(); it++)
		vfile << "#include \\\"" << *it << "\\\"\\n";

	LOG(vfile.str(), LOG_DEBUG);
	std::stringstream call;	
	call << "`which echo` -e \"\\n" << vfile.str() << "\" | " << compiler << " " << genCFlags(*(--it)) << " -c " << " - ";

	if(PROJECT->getValueStr("targettype") == "lib-shared")
		call << PROJECT->getValueStr("fpic") << " ";

	call << "-o " << object;

	return Tools::execute(call.str(), 0, 0, false) == 0;
}

bool CompilerGcc::compileSingleCall()
{
	LOG("Compiling and linking in single call...", LOG_VERBOSE);

	std::ifstream listFile(FILES->combinePath(Tools::makeStrVector(FILES->getTmpDir(), "filelist")).c_str());
	std::set<std::string> list;

	while(listFile.good()){
		std::string line;
		getline(listFile, line);

		if(line != "")
			list.insert(line);
	}

	listFile.close();

	std::string hyphen = PROJECT->getValueBool("addhyphen") ? " -" : " ";
	std::string ldFlags = PROJECT->getValueStr("ldflags", hyphen, hyphen, " ");
	
	std::stringstream call;
	call << PROJECT->getValueStr("compiler", 0) << " ";

	std::set<std::string>::iterator it;
	for(it = list.begin(); it != list.end(); it++)
		call  << *it << " ";

	// TODO generates cflags from last file, could theoretically support any number of file types etc.
	// multiple -std=, -x
	call << genCFlags(*(--it), true) << " " << ldFlags << " "; 

	if(PROJECT->getValueStr("targettype") == "lib-shared"){
		call << PROJECT->getValueStr("fpic") << " ";
	}

	call << " -o " << PROJECT->getValueStr("target");

	return Tools::execute(call.str(), 0, 0, false) == 0;
}

bool CompilerGcc::compileFileByFile()
{
	LOG("Checking dependencies...", LOG_VERBOSE);
	if(!checkLibs()){
		return false;
	}

	LOG("Preparing...", LOG_VERBOSE);
	std::vector<CList> cList = compileList();

	if(cList.size() > 0){
		LOG("Compiling...", LOG_VERBOSE);
	}else{
		LOG("Nothing to compile.", LOG_EXTRA_VERBOSE);
		return true;
	}
	
	ForkResult handle;
	PidList pidList;

	int numJobs = PROJECT->getValueInt("jobs") - 1;
	bool ret = true;

	LOG("Compiling with " << numJobs + 1 << " jobs.", LOG_EXTRA_VERBOSE);

	for(int i=0; i < (int)cList.size(); i++){
		LOG("[ " << getPercent(i, (int)cList.size()) << " %] " << Tools::stripSrcDir(cList.at(i).src), LOG_INFO);
		
		LOG(cList.at(i).call.c_str(), LOG_EXTRA_VERBOSE);
	
		if(FILES->fileExists(cList.at(i).obj)){
			LOG("Removing old object file: '" << cList.at(i).obj << "'", LOG_DEBUG);
			FILES->erase(cList.at(i).obj);
		}
	
		Tools::forkDo(cList.at(i).call.c_str(), pidList, i);
		handle = Tools::wait(pidList, numJobs);	

		if(!handle.noResult && handle.error){
			LOG("Compilation failed, removing associated files", LOG_DEBUG);
			markRecompile(cList.at(handle.id).src, cList.at(handle.id).obj);
			ret = false;
			break;
		}
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

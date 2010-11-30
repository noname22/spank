// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "tools.h"

#include <cstdlib>
#include <cstring>

// TODO: no unix dependencies in tools
#include <string>
#include <algorithm>

#include "project.h"
#include "macros.h"
#include "system.h"
#include "settings.h"

// UNIX stuff

void Tools::forkDo(std::string cmd, PidList& pidList, int id)
{
	pid_t pid = fork();
	if(pid == 0){
		LOG("New process: " << cmd, LOG_DEBUG);
		int exitCode = system(cmd.c_str());
		LOG("Process exited with code: " << exitCode, LOG_DEBUG); 
		LOG("(" << cmd << ")", LOG_DEBUG);
		exit(!!exitCode);
	}

	ForkResult p;
	p.pid = pid;
	p.id = id;
	p.cmd = cmd;
	pidList.push(p);
}

std::string Tools::getLineSs(std::stringstream& stream)
{
	char buffer[SPANK_MAX_LINE];
	stream.getline(buffer, SPANK_MAX_LINE);
	return std::string(buffer);
}

ForkResult Tools::wait(PidList& pidList, unsigned int max)
{
	ForkResult result;

	if(pidList.size() == 0){
		result.done = true;
		return result;
	}

	if(pidList.size() > max){
		int status;

		ForkResult& p = pidList.front();

		LOG("Waiting for pid: " << p.pid, LOG_DEBUG);
		waitpid(p.pid, &status, 0);
	
		result.pid = p.pid;
		result.id = p.id;
		result.noResult = false;

		if(WIFEXITED(status)){
			LOG("Process with cmd: " << p.cmd, LOG_DEBUG);
			LOG("Process exited normally and returned " << WEXITSTATUS(status), LOG_DEBUG);
			LOG("(" << status << ")", LOG_DEBUG);
			if(WEXITSTATUS(status) == 0){
				LOG("Process exited successfully", LOG_DEBUG);
				result.error = false;
			}else{
				LOG("Process exited with an error code", LOG_DEBUG);
			}
		}
		
		pidList.pop();
	}

	return result;
}

// non-UNIX stuff

std::string Tools::deEscape(std::string str)
{
	std::string out;
	char tmp;

	for(int i=0; i < (int)str.size(); i++){
		tmp = str.at(i);
		if(tmp == '\\'){
			if(i + 1 < (int)str.size()){
				i++;
				tmp = str.at(i);
			}else{
				return out;
			}
		}
		out.push_back(tmp);
	}
	return out;
}

bool Tools::executeAll(std::string configItem, std::string prefix, bool fake)
{
	for(int i=0; i < PROJECT->getNumValues(configItem); i++){
		if(PROJECT->getValueStr(configItem, i) != ""){
			FORMSTR(cmd, prefix << PROJECT->getValueStr(configItem, i));
			LOG("Executing script: '" << cmd << "'", LOG_VERBOSE);
			if(fake){
				LOG("[not executing] ..." << cmd, LOG_INFO);
			}else{
				if(system(cmd.c_str()) != 0){
					LOG("Script: '" << cmd << "' returned non-zero.", LOG_ERROR);
					return false;
				}
			}
		}
	}
	return true;
}

std::string Tools::genCFlags()
{
	std::string flags;
	
	if(PROJECT->getValueBool("addhyphen")){
		flags = PROJECT->getValueStr("cflags", "-", " -", " ");
	}else{
		flags = PROJECT->getValueStr("cflags", " ", " ", " ");
	}

	if(PROJECT->getValueBool("spankdefs", 0)){

		std::string compiler = PROJECT->getValueStr("compiler");

		if(compiler.find("gcc") != std::string::npos || compiler.find("g++") != std::string::npos){ // TODO: less ghetto compiler detection
			flags.append("-DSPANK_COMPILER_GCC ");
			// the only supported enviroment as of yet
			flags.append("-DSPANK_ENV_UNIX ");

			ADDSTR(flags, "-D'SPANK_NAME=\"" << PROJECT->getValueStr("name") << "\"' ");
			ADDSTR(flags, "-D'SPANK_BINNAME=\"" << PROJECT->getValueStr("target") << "\"' ");
			ADDSTR(flags, "-D'SPANK_VERSION=\"" << PROJECT->getValueStr("version") << "\"' ");
			ADDSTR(flags, "-D'SPANK_HOMEPAGE=\"" << PROJECT->getValueStr("homepage", " - ") << "\"' ");
			ADDSTR(flags, "-D'SPANK_AUTHOR=\"" << PROJECT->getValueStr("author", ", ") << "\"' ");
			ADDSTR(flags, "-D'SPANK_EMAIL=\"" << PROJECT->getValueStr("email", " - ") << "\"' ");
			ADDSTR(flags, "-D'SPANK_PREFIX=\"" << PROJECT->getValueStr("inst_prefix") << "\"' ");

		}

		if(PROJECT->getNumValues("lib")){
			flags.append(" `");
			flags.append(PROJECT->getValueStr("pkg-config"));
			flags.append(PROJECT->getValueStr("lib", " --cflags ", " ", "`"));
		}

	}
	return flags;
}

// TODO is this even used anywhere?
std::string Tools::genLdFlags()
{
	std::string ret;

	if(PROJECT->getNumValues("lib")){
		ret = "`";
		ret.append(PROJECT->getValueStr("pkg-config"));
		ret.append(PROJECT->getValueStr("lib", " --libs ", " ", "`"));
	}

	ret.append(PROJECT->getValueStr("ldflags", "-", " -", " "));
	return ret;
}

std::string Tools::stripSrcDir(std::string name)
{
	std::string sourcedir;

	if(PROJECT->getValueBool("stripsrc")){
		for(int i=0; i < PROJECT->getNumValues("sourcedir"); i++){
			if(
				//name.find( (sourcedir = PROJECT->getValueStr("sourcedir", i)) ) != std::string::npos &&
				name.substr(0, PROJECT->getValueStr("sourcedir", i).length()) == (sourcedir = PROJECT->getValueStr("sourcedir", i))
			){
				name.erase(0, sourcedir.size());
				if(name.at(0) == '/'){
					name.erase(0, 1);
				}
				return name;
			}
		}
	}
	return name;
}
int Tools::execute(std::string cmd, std::string stdFile, std::string errFile)
{
	FORMSTR(tmpCmd, cmd << " >" << stdFile << " 2>" << errFile);
	LOG("Executing: " << tmpCmd, LOG_DEBUG);
	return system(tmpCmd.c_str());
}

std::string Tools::joinStrings(std::vector<std::string> & strs, std::string separator)
{
	std::string ret;
	bool first = true;

	for(std::vector<std::string>::iterator it = strs.begin(); it != strs.end(); it++){
		if(first){
			first = false;
		}else{
			ret.append(separator);
		}

		ret.append(*it);
	}

	return ret;
}
		
int Tools::execute(std::string cmd, std::string* out, std::string* err)
{
	FORMSTR(outFile, FILES->getTmpDirStr() << "/cmd_out"); // TODO not very thread safe etc.
	FORMSTR(errFile, FILES->getTmpDirStr() << "/cmd_err"); // TODO not very thread safe etc.

	std::string tmpCmd;

	if(err || out){
		SETSTR(tmpCmd, cmd << " >" << outFile << " 2>" << errFile);
	}else{
		SETSTR(tmpCmd, cmd << " > /dev/null 2> /dev/null");
	}

	LOG("Executing: " << tmpCmd, LOG_DEBUG);

	int ret = system(tmpCmd.c_str());

	if(out){
		*out = FILES->strFromFile(outFile);
	}

	if(err){
		*err = FILES->strFromFile(errFile);
	}

	return ret;
}

std::string Tools::toLower(std::string in)
{
	std::transform(in.begin(), in.end(), in.begin(), ::tolower);
	return in;
}

std::string Tools::filenameify(std::string str)
{
	std::string ret, allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_=";
	for(std::string::iterator it = str.begin(); it != str.end(); it++){
		if(allowed.find(*it) != std::string::npos){
			ret.push_back(*it);
		}else{
			ret.push_back('_');
		}
	}

	return ret;
}

std::string Tools::nameEnc(std::string ext, std::string name)
{
	std::string ret;
	std::string cmp;

	for(std::string::iterator iter=name.begin(); iter != name.end(); iter++){
		if((cmp = *iter) == "/"){
			ret.append("___");
		}else{
			ret.append(cmp);
		}
	}

	ret.append(ext);
	return ret;
}

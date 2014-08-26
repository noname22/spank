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
#include <fstream>

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

std::string Tools::getLineStream(std::istream& stream)
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
			LOG("Executing script: '" << cmd << "'", LOG_EXTRA_VERBOSE);
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

std::vector<std::string> Tools::splitString(std::string str, char separator, int max)
{
	std::vector<std::string> ret;

	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, separator)) {
		ret.push_back(item);
		
		if(max >= 0 && (int)ret.size() >= max){
			std::istreambuf_iterator<char> eos;
			std::string s(std::istreambuf_iterator<char>(ss), eos);
			ret.push_back(s);
			break;
		}
	}

	return ret;
}

std::string Tools::trim(const std::string s)
{
        return ltrim(rtrim(s));
}

std::string Tools::ltrim(std::string s)
{
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

std::string Tools::rtrim(std::string s)
{
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

std::string Tools::restOfString(std::string str, std::string startsWith)
{
	if(str.length() > startsWith.length() && str.substr(0, startsWith.length()) == startsWith)
		return str.substr(startsWith.length());

	return "";
}
		
int Tools::execute(std::string cmd, std::string* out, std::string* err, bool supress)
{
	FORMSTR(outFile, FILES->getTmpDirStr() << "/cmd_out"); // TODO not very thread safe etc.
	FORMSTR(errFile, FILES->getTmpDirStr() << "/cmd_err"); // TODO not very thread safe etc.

	std::stringstream tmpCmd;

	tmpCmd << cmd;

	if(out || supress)
		tmpCmd << " > '" << (out ? outFile : "/dev/null") << "'";

	if(err || supress)
		tmpCmd << " 2> '" << (err ? errFile : "/dev/null") << "'";
		

	LOG("Executing: " << tmpCmd.str(), LOG_DEBUG);

	int ret = system(tmpCmd.str().c_str());

	if(out)
		*out = FILES->strFromFile(outFile);

	if(err)
		*err = FILES->strFromFile(errFile);

	return ret;
}

std::string Tools::toLower(std::string in)
{
	std::transform(in.begin(), in.end(), in.begin(), ::tolower);
	return in;
}

std::string Tools::toUpper(std::string in)
{
	std::transform(in.begin(), in.end(),in.begin(), ::toupper);
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

std::vector<std::string> Tools::makeStrVector(std::string a)
{
	std::vector<std::string> ret;
	ret.push_back(a);
	return ret;
}

std::vector<std::string> Tools::makeStrVector(std::string a, std::string b)
{
	std::vector<std::string> ret;
	ret.push_back(a);
	ret.push_back(b);
	return ret;
}

std::vector<std::string> Tools::makeStrVector(std::string a, std::string b, std::string c)
{
	std::vector<std::string> ret;
	ret.push_back(a);
	ret.push_back(b);
	ret.push_back(c);
	return ret;
}

std::vector<std::string> Tools::makeStrVector(std::string a, std::string b, std::string c, std::string d)
{
	std::vector<std::string> ret;
	ret.push_back(a);
	ret.push_back(b);
	ret.push_back(c);
	ret.push_back(d);
	return ret;
}


bool Tools::endsWith(std::string str, std::string suffix)
{
	if(suffix.size() > str.size())
		return false;

	return std::mismatch(suffix.rbegin(), suffix.rend(), str.rbegin()).first == suffix.rend();
}

void Tools::saveTempValue(std::string key, std::string value)
{
	std::string filename = FILES->combinePath(makeStrVector(FILES->getTmpDir(), key));
	LOG("saving tempdata to file: " << filename, LOG_DEBUG);
	std::ofstream file(filename.c_str());
	file << value;
	file.close();
}

bool Tools::loadTempValue(std::string key, std::string& value)
{
	std::string filename = FILES->combinePath(makeStrVector(FILES->getTmpDir(), key));

	if(!FILES->fileExists(filename))
		return false;

	std::ifstream file(filename.c_str());

	if(file.bad()){
		LOG("load of temp data failed: " << key, LOG_DEBUG);
		return false;
	}

	// TODO handle multiple lines
	getline(file, value);

	file.close();

	return true;
}

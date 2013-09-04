// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "filesunix.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <limits.h>

#include <fstream>
#include <cstdlib>
#include <cstring>

#include "project.h"
#include "tools.h"
#include "macros.h"

// Invokes the preprocessor and pipes the result through md5sum
bool FilesUnix::writeMd5(std::string src, std::string file)
{
	// TODO: should be implemented in the compiler

	FORMSTR(cppOut, getTmpDir() << "/cpp_out");
	FORMSTR(call, PROJECT->getValueStr("pp") << " -nostdinc -nostdinc++ " << Tools::genCFlags() << " " << src);

	bool ret = !Tools::execute(call, cppOut);

	SETSTR(call, "md5sum " << cppOut << " | cut -b -32");
	Tools::execute(call, file);

	return ret;
}

std::string getUserHomeDir()
{
	struct passwd *passwd;
	passwd = getpwuid ( getuid());
	return passwd->pw_dir;	
}

std::pair<std::string, std::string> FilesUnix::pathSplit(std::string path)
{
	//path = Tools::deEscape(path);
	size_t slash;

	if((slash = path.rfind("/")) == std::string::npos){
		return make_pair("", path);
	}

	return make_pair(path.substr(0, slash + 1), path.substr(slash + 1));
}


// Returns the home directory of spank
std::string FilesUnix::getHomeDir()
{
	return PROJECT->getValueStr("homedir");
}

// Creates the home directory if it doesn't exist
bool FilesUnix::createDir(std::string dir)
{
#if 0
	if(fileExists(dir)){
		if(!isDir(dir)){
			LOG(
				"Spank tried to create a directory where there already is a file on the system ('" << 
				dir << "'). Please remove it or change the configuration.", LOG_FATAL
			);
			return false;
		}else{
			LOG("Dir exists (" << dir << ")", LOG_VERBOSE);
			return true;
		}
	}
	
	LOG("Creating directory ('" << dir << "')", LOG_VERBOSE);
	if(mkdir(dir.c_str(), 0700)){
		LOG("Couldn't create directory.", LOG_FATAL);
		return false;
	}
	return true;
#endif

// BAH, who needs proper code when you can do stupid hacks?
// Like I want to implement mkdir -p when there's such an easy hack available...

	FORMSTR(cmd, "mkdir -p \"" << dir << "\"");
	Tools::execute(cmd);
	return true;
}

void FilesUnix::markRecompilePp(std::string src)
{
	std::string tmp = getTmpDir();
	tmp.append("/");
	tmp.append(Tools::nameEnc(".pp.md5", src));
	erase(tmp);
}

bool FilesUnix::removeDir(std::string dir)
{
	FORMSTR(cmd, "rm -rf \"" << dir << "\"");
	return !Tools::execute(cmd);
}

bool FilesUnix::copy(std::string from, std::string to)
{
	FORMSTR(cmd, "cp \"" << from << "\" \"" << to << "\"");
	return !Tools::execute(cmd);
}

bool FilesUnix::checkRecompilePp(std::string src)
{
	LOG("Checking if " << src << " needs recompiling...", LOG_VERBOSE);

	bool notBuilt=false, upToDate=false;

	FORMSTR(md5name, getTmpDir() << Tools::nameEnc(".pp.md5", src)); // The final filename of the recompile check checksum
	FORMSTR(md5tmp, getTmpDir() << "md5tmp"); // The temporary md5sum to check against saved checksum (above)
	FORMSTR(oName, getTmpDir() << Tools::nameEnc(".o", src)); // Target object file

	// if the preprocessor returns an error just continue, should be taken care of by the md5 diff and markRecompilePp
	if(!writeMd5(src, md5tmp)){
		LOG("The preprocessor returned an error: continuing ...", LOG_VERBOSE);
		//return true;
	}

	std::string md5 = strFromFile(md5tmp);

	LOG("MD5 sum of preprocessed '" << src << "' is '" << md5 << "'", LOG_DEBUG);
	LOG(strFromFile(md5name), LOG_DEBUG);
	
	if(!fileExists(oName)){
		LOG("'" << src << "' hasn't been built.", LOG_VERBOSE);
		notBuilt = true;
	}

	if(md5 == strFromFile(md5name) /* readMd5(md5name)*/ ){
		LOG(src << " is up to date.", LOG_VERBOSE);
		upToDate = true;
	}else{
		LOG(src << " needs recompiling.", LOG_VERBOSE);
		LOG("( " << md5 << " != " << strFromFile(md5name) << " )", LOG_VERBOSE);
		if(!copy(md5tmp, md5name)){
			LOG("Couldn't copy hash file", LOG_ERROR);
		}
	}

	return !upToDate || notBuilt;
}

void FilesUnix::wait()
{
	usleep(10000);
}

bool FilesUnix::find(std::string what, std::string where, std::string result)
{
	std::string findThis("find ");
	findThis.append(where);
	findThis.append(" -iname \"");
	findThis.append(what); 
	findThis.append("\" >> ");
	findThis.append(result);

	LOG(findThis, LOG_VERBOSE);

	if(system(findThis.c_str()) == 0){
		return true;
	}
	return false;
}


bool FilesUnix::genSourceFileList(std::string dir)
{
	LOG("creating a filelist", LOG_DEBUG);
	std::string filelist = dir;
	filelist.append("/filelist");
	remove(filelist.c_str());

	for(int i2=0; i2 < PROJECT->getNumValues("sources"); i2++){
		std::string sources = PROJECT->getValueStr("sources", i2);
		if(sources.find("/") != std::string::npos){
			if(fileExists(sources)){
				std::ofstream out(filelist.c_str());
				out << sources << std::endl;
				out.close();
			}else{
				LOG("Can't find: " << sources, LOG_ERROR);
				return false;
			}
		}else{
			for(int i=0; i < PROJECT->getNumValues("sourcedir"); i++){
				if(!find(sources, PROJECT->getValueStr("sourcedir", i), filelist)){
					return false;
				}
			}
		}
	}
	return true;
}

int FilesUnix::isDir (std::string path)
{
  struct stat stats;
  return stat (path.c_str(), &stats) == 0 && S_ISDIR (stats.st_mode);
}

std::string FilesUnix::getGlobalTmpDir()
{
	FORMSTR(name, PROJECT->getValueStr("target") << "." << getFullProjectPath());
	std::string tmp = "/tmp/";
	tmp.append(Tools::nameEnc(".tempfiles", name));
	return tmp;
}

std::string FilesUnix::getTmpDirStr()
{
	FORMSTR(name, PROJECT->getValueStr("target") << "." << getFullProjectPath());
	FORMSTR(tmp, getHomeDir() << "/" << Tools::nameEnc(".tempfiles", name) << "/");
	return tmp;
}

time_t FilesUnix::getDate(std::string file)
{
	struct stat getStat;
	getStat.st_mtime = 0;

	stat(file.c_str(), &getStat);

	return getStat.st_mtime;
}

bool FilesUnix::fileExists(std::string file)
{
	if(access(file.c_str(), 0) == F_OK){
		return true;
	}else{
		return false;
	}
}
	
std::string FilesUnix::combinePath(std::vector<std::string> p)
{
	std::string ret;
	
	for(std::vector<std::string>::iterator it = p.begin(); it != p.end(); it++){
		ret.append(*it);
		ret.push_back('/');
	}
	ret.erase(ret.size() - 1);

	return ret;
}

std::string FilesUnix::dirName(std::string filename)
{
	char* n = dirname(strdup(filename.c_str()));
	std::string ret = n;
	free(n);
	return ret;
}

std::string FilesUnix::baseName(std::string filename)
{
	char* n = basename(strdup(filename.c_str()));
	std::string ret = n;
	free(n);
	return ret;
}

std::string FilesUnix::getFullProjectPath()
{
	char buffer[PATH_MAX];
	if(!realpath(".", buffer)){
		LOG("could not get path to project directory", LOG_FATAL);
		exit(1);
	}
	return buffer;
}

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
#include <sstream>
#include <cstdlib>
#include <cstring>

#include "project.h"
#include "tools.h"
#include "macros.h"

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
			LOG("Dir exists (" << dir << ")", LOG_EXTRA_VERBOSE);
			return true;
		}
	}
	
	LOG("Creating directory ('" << dir << "')", LOG_EXTRA_VERBOSE);
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

void FilesUnix::wait()
{
	usleep(10000);
}

bool FilesUnix::find(std::string what, std::string where, std::string result)
{
	std::stringstream call;
	call << PROJECT->getValueStr("find") << " " << where << " -iname '" << what << "' >> '" << result << "'";

	LOG(call.str(), LOG_DEBUG);
	return system(call.str().c_str()) == 0;
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
	FORMSTR(name, PROJECT->getValueStr("target") << "." << PROJECT->getValueStr("projectpath"));
	std::string tmp = "/tmp/";
	tmp.append(Tools::nameEnc(".tempfiles", name));
	return tmp;
}

std::string FilesUnix::getTmpDirStr()
{
	FORMSTR(name, PROJECT->getValueStr("target") << "." << PROJECT->getValueStr("projectpath"));
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

std::string FilesUnix::realpath(std::string filename)
{
	char buffer[PATH_MAX];

	if(!::realpath(filename.c_str(), buffer)){
		LOG("realname() for " << filename << " failed.", LOG_EXTRA_VERBOSE);
		return "";
	}

	return buffer;
	
}

int FilesUnix::chdir(std::string dir)
{
	return ::chdir(dir.c_str());
}

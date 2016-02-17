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
	const char* home = getenv("HOME");
	
	if(home == NULL)
		throw std::runtime_error("HOME environment variable not set");
	
	return std::string(home);
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
	return Tools::execute(Str("rm -rf '" << dir << "'")) == 0;
}
		
bool FilesUnix::removeFile(const std::string& path)
{
	return system(Str("rm '" << path << "'").c_str()) == 0;
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


void FilesUnix::genSourceFileList(std::string dir)
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
				FORMSTR(msg, "Can't find: " << sources);
				throw FilesException(msg);
			}
		}else{
			for(int i=0; i < PROJECT->getNumValues("sourcedir"); i++){
				if(!find(sources, PROJECT->getValueStr("sourcedir", i), filelist)){
					FORMSTR(msg, "Could not locate all source files in directory: " << PROJECT->getValueStr("sourcedir", i));
					throw FilesException(msg);
				}
			}
		}
	}
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

std::string FilesUnix::getAbsoluteExecutablePath(std::string filename)
{
	std::string path;
	if(Tools::execute(Str("which '" << filename << "'"), &path) != 0)
		throw FilesException(Str("could not locate binary: " << filename));

	if(path.size() < 2)
		throw FilesException(Str("could not locate binary: " << filename));
	
	return realpath(path.substr(0, path.size() - 1));
}

std::string FilesUnix::realpath(std::string filename)
{
	std::string name;
	std::string err;
	
	std::string cmd = Str("readlink -n -f -- '" << filename << "'");

	int ret = Tools::execute(cmd, &name, &err);

	if(ret != 0)
		throw std::runtime_error(Str("realpath failed to execute command: " << cmd << ", error returned: " << err << ", exit code: " << ret));
	
	std::cout << filename << " -> " << name << std::endl;
	
	return name;
}

int FilesUnix::chdir(std::string dir)
{
	LOG("chdir: " << dir, LOG_EXTRA_VERBOSE);
	return ::chdir(dir.c_str());
}

std::string FilesUnix::genSystemTempFileName(const std::string& prefix)
{
	int32_t rNum = 0;
	std::ifstream f("/dev/urandom");

	if(!f.good()){
		throw FilesException("could not read from /dev/urandom");
	}

	f.read((char*)&rNum, 4);
	f.close();

	return combinePath(Tools::makeStrVector("/tmp", Str(prefix << rNum)));
}

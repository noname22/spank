// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef TOOLS_H
#define TOOLS_H

#include <queue>

// TODO: no unix dependencies in tools
#include <unistd.h>

#include <string>
#include <sstream>		
#include <vector>
#include <set>
#include <map>

#define Str(_what) [&]() -> std::string {std::stringstream _tmp; _tmp << _what; return _tmp.str(); }()

typedef std::vector<std::string> StrVec;
typedef std::set<std::string> StrSet;
typedef std::map<std::string, std::string> StrStrMap;

class Tools
{
	public:
	static std::string nameEnc(std::string ext, std::string name);
	static std::string stripSrcDir(std::string name);
	static std::string toLower(std::string in);
	static std::string toUpper(std::string in);
	static std::string filenameify(std::string str);
	static std::string getLineStream(std::istream& stream);
	static std::string trim(std::string str);
	static std::string rtrim(std::string str);
	static std::string ltrim(std::string str);
	static bool endsWith(std::string str, std::string suffix);

	static int execute(std::string cmd, std::string* std = NULL, std::string* out = NULL, bool supress = true);
	static int execute(std::string cmd, std::string stdFile, std::string errFile = "/dev/null");
	static std::string deEscape(std::string str);
	static std::string joinStrings(std::vector<std::string> & strs, std::string separator = " ");
	static std::vector<std::string> splitString(std::string str, char separator = ' ', int max = -1);
	static std::string restOfString(std::string str, std::string startsWith);

	static std::vector<std::string> makeStrVector(std::string a);
	static std::vector<std::string> makeStrVector(std::string a, std::string b);
	static std::vector<std::string> makeStrVector(std::string a, std::string b, std::string c);
	static std::vector<std::string> makeStrVector(std::string a, std::string b, std::string c, std::string d);
	
	static std::vector<int> threadedExecute(const std::vector<std::string>& commands);
	static bool executeAll(std::string configItem, std::string prefix = "", bool fake = false);
	static void saveTempValue(std::string key, std::string value);
	static bool loadTempValue(std::string key, std::string& value);

	static bool checkExclude(const std::string src);
	static StrSet getSourceList();
};

#endif


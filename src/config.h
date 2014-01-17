// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "log.h"

enum{
	VAR_INTERNAL,
	VAR_PROJECT,	
	VAR_CMDLINE
};

enum{
	C_SET,
	C_ADD
};

class ConfigItem{
	public:
	ConfigItem();
	std::string variable;
	std::vector< std::string > value;
	int type;
};	

class Config{
	std::map<std::string, ConfigItem> configItems;
	bool setAddValue(int action, std::string variable, std::string value, int type);

	std::string getEntryTypeName(int type);
	
	public:
	Config();

	bool setValue(std::string variable, std::string value, int type = VAR_INTERNAL);
	bool addValue(std::string variable, std::string value, int type = VAR_INTERNAL);
	bool delValue(std::string variable);
	bool overlayValue(std::string variable, std::string value);

	void dumpConfig();

	bool fromCmdLine(int argc, const char* const* argv);

	bool loadConfig(std::string filename, int depth=0);
	std::string getValueStr(std::string variable, int index = 0, int depth = 0); // depth should be considered private, used for recursion

	std::string getValueStr(std::string variable, std::string separator);
	std::string getValueStr(std::string variable, std::string addBefore, std::string separator, std::string addAfter = "", bool reverse = false);

	bool getValueBool(std::string variable, int index = 0);
	int getValueInt(std::string variable, int index = 0);
	int getNumValues(std::string variable);
};

#endif

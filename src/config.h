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
#include <stdexcept>
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

class ConfigException : public std::runtime_error {
	public:
	ConfigException(std::string str) : std::runtime_error(str) {}
};

class ConfigItem {
	public:
	ConfigItem();
	std::string key;
	std::vector< std::string > value;
	int type;
};

class Section {
	public:
	std::string name;
	std::string source;
	std::string inherits;
	bool isPublic;
	bool isDefault;
};

class Config{
	std::map<std::string, ConfigItem> configItems;

	bool setAddValue(int action, std::string key, std::string value, int type);
	static bool parseSection(std::string line, Section& section);

	std::string getEntryTypeName(int type);
	
	public:
	Config();

	bool setValue(std::string key, std::string value, int type = VAR_INTERNAL);
	bool addValue(std::string key, std::string value, int type = VAR_INTERNAL);
	bool delValue(std::string key);
	bool overlayValue(std::string key, std::string value);

	void dumpConfig();

	bool fromCmdLine(int argc, const char* const* argv);

	bool loadConfig(std::string filename, std::string section = "default", int depth = 0, std::string lastFilename = "");
	std::string getValueStr(std::string key, int index = 0, int depth = 0); // depth should be considered private, used for recursion

	std::string getValueStr(std::string key, std::string separator);
	std::string getValueStr(std::string key, std::string addBefore, std::string separator, std::string addAfter = "", bool reverse = false);

	bool getValueBool(std::string key, int index = 0);
	int getValueInt(std::string key, int index = 0);
	int getNumValues(std::string key);

	bool containsValue(std::string key, std::string value);

	std::vector<std::string> getValues(std::string key);
	
	std::vector<Section> getSectionList();

	static bool listHasSection(std::vector<Section> sections, std::string section);
	static Section getDefaultSectionFromList(std::vector<Section> sections);
	static void printSectionList(std::vector<Section> sections);
};

#endif

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
	private:
		std::vector< ConfigItem > configItems;
		int lookUp(std::string variable);
	
		bool setAddValue(int action, std::string variable, std::string value, int type);

		std::string getEntryTypeName(int type);
		void printItem(unsigned int index, int loglevel);
	
	public:
		bool setValue(std::string variable, std::string value, int type);
		bool setValue(std::string variable, std::string value);
		bool addValue(std::string variable, std::string value, int type);
		bool addValue(std::string variable, std::string value);
		bool delValue(std::string variable);
		bool overlayValue(std::string variable, std::string value);

		void dumpConfig();

		bool fromCmdLine(int argc, const char* const* argv);

		bool loadConfig(std::string filename);
		std::string getValueStr(std::string variable, int index = 0, int depth = 0); // depth should be considered private, used for recursion

		std::string getValueStr(std::string variable, std::string separator);
		std::string getValueStr(std::string variable, std::string addBefore, std::string separator);
		std::string getValueStr(std::string variable, std::string addBefore, std::string separator, std::string addAfter);

		bool getValueBool(std::string variable, int index = 0);
		int getValueInt(std::string variable, int index = 0);
		int getNumValues(std::string variable);
		Config();
};

#endif

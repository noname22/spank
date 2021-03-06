// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include "settings.h"
#include "tools.h"
#include "macros.h"

ConfigItem::ConfigItem()
{
	type = VAR_INTERNAL;
}

Config::Config()
{
}

bool Config::fromCmdLine(int argc, const char* const* argv)
{
	std::stringstream parse;
	std::string tmp;

	bool expect = false;
	bool addVar = false;
	bool actionSet = false;
	std::string current;

	for(int i = 1; i < argc; i++)
	{
		std::string tmp(argv[i]);

		if((tmp.substr(0,2) == "--" && tmp.length() > 4)){
			addVar = true;
		}else if((tmp.substr(0,1) == "-" && tmp.length() > 3)){
			addVar = true;
		}else{
			addVar= false;
		}

		if(addVar){
			if(expect){
				if(!addValue(current, "true", VAR_CMDLINE)){
					return false;
				}
			}
			
			if(tmp.substr(0,2) == "--"){
				current = tmp.substr(2);
			}else{
				current = tmp.substr(1);
			}
			expect = true;
		}else{
			if(expect){
				if(!addValue(current, Tools::deEscape(tmp), VAR_CMDLINE)){
					return false;
				}
				expect = false;
			}else{
				if(tmp.substr(0, 1) != "-"){
					if(!actionSet){
						setValue("action", tmp, VAR_CMDLINE);
						actionSet = true;
					}else{
						// Something written after action is parsed as a project
						setValue("extraarg", tmp);
					}
				}
			}
		}
	}

	if(expect){
		if(current == "help" || current == "showconfig")
		{
			addValue(current, "true", VAR_CMDLINE);
			return true;
		}
		else
		{
			LOG("Expected a value for command line parameter: '" << current << "'", LOG_ERROR);
			return false;
		}
	}

	return true;
}
		
std::string Config::getEntryTypeName(int type)
{
	switch(type)
	{
		case VAR_INTERNAL:
			return "  built in  ";
		case VAR_PROJECT:
			return "project file";
		case VAR_CMDLINE:
			return "command line";
		default:
			return "  unknown   ";
	}
}

void Config::dumpConfig()
{
	unsigned maxKeyLen = 0;

	for(auto& item : configItems)
		maxKeyLen = item.first.size() > maxKeyLen ? item.first.size() : maxKeyLen;
	
	LOG("", LOG_INFO);
	LOG("Current config", LOG_INFO);
	LOG("", LOG_INFO);
	LOG("[   origin   ] " << std::left << std::setw(maxKeyLen + 1) << "item" << 
		"\"value 1\" (, \"value 2\", \"value 3\", ..., \"value n\")", LOG_INFO);

	LOG("", LOG_INFO);


	for(auto& item : configItems)
	{
		LOG("[" << getEntryTypeName(item.second.type) << "] " << std::left << std::setw(maxKeyLen + 1) << 
			item.first << Tools::joinStrings(item.second.value, ", ", "\""), LOG_INFO);
	}

	LOG("", LOG_INFO);
}

bool Config::setValue(std::string key, std::string value, int type){
	return setAddValue(C_SET, key, value, type);
}

// Add a value only if it doesn't exist
bool Config::overlayValue(std::string key, std::string value)
{
	if(configItems.count(key) == 0){
		ConfigItem tmp;
		tmp.value.push_back(value);
		tmp.key = key;
		configItems[key] = tmp;
		return true;
	}
	return false;
}

bool Config::setAddValue(int action, std::string key, std::string value, int type){
	bool deleted=false;

	if(key == "homedir"){
		LOG(key << " " << value, LOG_DEBUG);
	}

	bool exists = configItems.count(key) != 0;

	if(exists && (
			(action == C_SET && configItems[key].type <= type) || 
			(action == C_ADD && configItems[key].type < type)))
	{
		delValue(key);
		deleted = true;
	}

	if(configItems.count(key) != 0){
		if(configItems[key].type <= type){
			configItems[key].value.push_back(value);
			return true;
		}
	}
	
	else{
		if(type == VAR_INTERNAL || deleted || key.substr(0, 5) == "user_"){
			ConfigItem tmp;
			tmp.value.push_back(value);
			tmp.key = key;
			tmp.type = type;
			configItems[key] = tmp;
			return true;
		}
		
		else{
			ThrowEx(ConfigException, "Unknown option in project file: '" << key << "'");
		}
	}

	return exists;
}
		
bool Config::addValue(std::string key, std::string value, int type)
{
	return setAddValue(C_ADD, key, value, type);
}

bool Config::delValue(std::string key)
{
	bool ret = configItems.count(key) != 0;
	configItems.erase(key);
	return ret;
}
std::string Config::getValueStr(std::string key, std::string separator){
	return getValueStr(key, "", separator, "");
}

std::string Config::getValueStr(std::string key, std::string addBefore, std::string separator, std::string addAfter, bool reverse)
{
	std::string ret;
	if(configItems.count(key) != 0 && getValueStr(key, 0) != ""){
		ret.append(addBefore);
		bool first = true;

		int start = reverse ? getNumValues(key) - 1 : 0;
		int end = reverse ? -1 : getNumValues(key);

		for(int i = start; i != end; i += reverse ? -1 : 1){
			if(first){
				first = false;
			}else{
				ret.append(separator);
			}
			ret.append(getValueStr(key, i));
		}
		ret.append(addAfter);
	}
	return ret;
}

bool Config::parseSection(std::string line, Section& sec)
{
	std::stringstream ss(line);

	char section[512] = {0};
	char inherits[512] = {0};

	int ret = sscanf(line.c_str(), "[%511[^]:] : %511[^]]]", section, inherits);

	if(ret < 1)
		return false;
	
	sec.isPublic = section[0] != '-';
	sec.isDefault = section[0] == '*';

	sec.name = (!sec.isPublic || sec.isDefault) ? section + 1 : section;
	sec.inherits = inherits;

	return true;
}

bool Config::loadConfig(std::string filename, std::string section, int depth, std::string lastFilename)
{
	if(depth > 255){
		LOG("Are you perhaps including two project files from eachother, or is a build configuration directly or indirectly inheriting itself?", LOG_INFO);
		ThrowEx(ConfigException, "Too many includes or too deep build configuration inheritance (255).");
	}

	std::ifstream in(filename.c_str());
	std::istringstream parse;
	std::string get;
	int i = 0, lineCount = 0, itemsInsertedCount = 0;
	bool goodItem = false;
	ConfigItem item;

	std::string parsingSection = "default";

	if(!in.good())
		return false;
	
	LOG("Loading project file " << filename, filename != lastFilename ? LOG_VERBOSE : LOG_DEBUG);

	while( in.good() ){
		i=0;
		item.key = "";
		goodItem = false;

		item.value.clear();
		std::getline(in, get);
		lineCount++;	

		Section sec;

		// if parseSection parsed a section line, continue with next line
		if(parseSection(get, sec)){
			parsingSection = sec.name;

			if(sec.name == section && sec.inherits != "")
				loadConfig(filename, sec.inherits, depth + 1, filename);

			continue;
		}

		// only load data in the currently active section
		if(parsingSection != section)
			continue;

		parse.str(get);

		while( parse >> get ){
			if(get.length() > 0 && get.at(0) == '#')
				break;

			if(get.length() > 0 && get.at(0) == '"'){
				bool missingQuote = true;
				bool done = false;

				get = get.substr(1, get.length());

				if(get.at(get.length() - 1) == '"'){
					get = get.substr(0, get.length() - 1);
					done = true;
				}

				while(!done && parse.good()){
					char c;
					if((c = parse.get()) == '"'){
						missingQuote = false;
						break;
					}else{
						get.push_back(c);
					}
				}

				AssertEx(!(!done && missingQuote), ConfigException, 
					"Unterminated quotation (\") @ " << filename << ":" << lineCount);
			}

			if(i == 0){
				item.key = get;
			}else{
				item.value.push_back(get);
				goodItem = true;
			}
			i++;
		}
		
		if(goodItem){
			itemsInsertedCount++;

			for(int x=0; x < (int)item.value.size(); x++){
				// overwrite previous value if * is prepended
				bool overwrite = item.key.at(0) == '*';
				item.key = item.key.substr(overwrite ? 1 : 0);

				AssertEx(!(!setAddValue(overwrite ? C_SET : C_ADD, item.key, item.value.at(x), VAR_PROJECT) && configItems.count(item.key) != 0), 
					ConfigException, "Syntax error in project file @ " << filename << ":" << lineCount);
			}

			if(item.key == "include"){
				for(std::vector<std::string>::iterator it = item.value.begin(); it != item.value.end(); it++){
					AssertEx(loadConfig(*it, "default", depth + 1, filename), ConfigException,
						"Could not include file: " << *it << " @ " << filename << ":" << lineCount);
				}
			}
		}
		
		parse.clear();		
	}

	AssertEx(itemsInsertedCount > 0, ConfigException, 
		"Empty configuration for: " << section << " in file: " << filename);

	return true;
}

bool Config::getValueBool(std::string key, int index){
	std::string tmp = getValueStr(key, index);
	if(tmp == "true" || tmp == "yes" || tmp == "1" || tmp == "si" || tmp == "ja" || tmp == "da" || tmp == "oui" || tmp == "jawohl"){
		return true;
	}else{
		return false;
	}
}

std::string Config::getValueStr(std::string key, int index, int depth)
{
	//LOG("Looking up: " << key, LOG_DEBUG);

	if(depth > SPANK_MAX_RECURSE){
		LOG("Recurse limit (" << SPANK_MAX_RECURSE << ") exceeded in Config::getValueStr(), probably because of key self referencing (or indirect self referencing).", LOG_ERROR);
		LOG("Might be in: " << key << "[" << index << "]", LOG_INFO);

		return "";
	} 

	if(configItems.count(key) != 0){
		if((unsigned int)index <= configItems[key].value.size()){
			std::string ret = configItems[key].value.at(index);

			//LOG("Parsing string: " << ret, LOG_DEBUG);

			size_t start = 0;

			while((start = ret.find("$("), start) != std::string::npos){
				size_t stop = ret.find(")", start);

				//LOG("start: " << start, LOG_DEBUG);
				//LOG("stop: " << stop, LOG_DEBUG);

				if(stop == std::string::npos){
					LOG("Error parsing key refernce: expected ')' near '" << ret << "'", LOG_ERROR);
				}

				std::string ref = ret.substr(start + 2, stop - start - 2);
				//LOG("reference: " << ref, LOG_DEBUG);
				std::string rep = getValueStr(ref, 0, depth + 1);
				//LOG("replacement value: " << rep, LOG_DEBUG);

				ret.replace(start, stop - start + 1, rep);
				//LOG("resulting string: " << ret, LOG_DEBUG);
			}

			return ret;
		}
	}

	char* env = getenv(key.c_str());
	if(env){
		return env;
	}

	ThrowEx(ConfigException, "Couldn't find config key: '" << key << "'");

	return "";
}

int Config::getValueInt(std::string key, int index)
{
	if(configItems.count(key) != 0){
		if((unsigned int)index <= configItems[key].value.size()){
			return atoi(configItems[key].value.at(index).c_str());
		}
	}

	LOG("Couldn't find config key: '" << key << "'", LOG_FATAL);
	return -1;
}

int Config::getNumValues(std::string key)
{
	if(configItems.count(key) != 0){
		if(configItems[key].value.size() == 1 && configItems[key].value.at(0) == ""){
			return 0;
		}
		return configItems[key].value.size();
	}
	return 0;
}

std::vector<std::string> Config::getValues(std::string key)
{
	std::vector<std::string> ret;

	for(int i = 0; i < getNumValues(key); i++)
		ret.push_back(getValueStr(key, i));
	
	return ret;
}

std::vector<Section> Config::getSectionList()
{
	int count = getNumValues("project");
	std::vector<Section> ret;
	
	for(int i = 0; i < count; i++){
		std::string filename = getValueStr("project", i);

		try {
			std::ifstream in(filename.c_str());
			std::string line;

			while(getline(in, line).good()){
				Section sec;
				if(parseSection(line, sec)){
					sec.source = filename;
					ret.push_back(sec);
				}
			}

			in.close();
		}

		catch (std::ifstream::failure& ex){
			LOG("could not load file: " << filename, LOG_VERBOSE);
		}
	}

	return ret;
}
	
bool Config::listHasSection(std::vector<Section> sections, std::string name)
{
	for(std::vector<Section>::iterator it = sections.begin(); it != sections.end(); it++){
			if(it->name == name)
				return true;
	}
	return false;
}

Section Config::getDefaultSectionFromList(std::vector<Section> sections)
{
	for(std::vector<Section>::iterator it = sections.begin(); it != sections.end(); it++){
			if(it->isDefault)
				return *it;
	}

	Section sec;
	return sec;
}

void Config::printSectionList(std::vector<Section> sections)
{
	LOG("Available build configurations:", LOG_INFO);
	for(std::vector<Section>::iterator it = sections.begin(); it != sections.end(); it++){
		if(it->isPublic)
			LOG("  " << it->name << (it->isDefault ? " (default)" : ""), LOG_INFO);
	}
}

bool Config::containsValue(std::string key, std::string value)
{
	int nv = getNumValues(key);
	
	for(int i = 0; i < nv; i++)
		if(getValueStr(key, i) == value)
			return true;
	
	return false;
}

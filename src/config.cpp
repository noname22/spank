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

	bool first=true;

	for(int i=0; i < argc; i++){
		if(first){
			first = false;
		}else{
			parse << " ";
		}
		parse << argv[i];
	}

	first = true;
	bool expect = false;
	bool addVar = false;
	bool actionSet = false;
	std::string current;

	parse >> tmp;

	while(parse.good()){
		parse >> tmp;

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
						FORMSTR(tmp2, tmp << ".spank");
						setValue("project", tmp2, VAR_CMDLINE); 
						FORMSTR(tmp3, "spank/" << tmp << ".spank");
						addValue("project", tmp3, VAR_CMDLINE); 
					}
				}
			}
		}
	}

	if(expect){
		if(!addValue(current, "true", VAR_CMDLINE)){
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

void Config::printItem(unsigned int index, int loglevel)
{
	std::ostringstream tmp;
	if(index < configItems.size()){
		tmp 	<< "[" << getEntryTypeName(configItems.at(index).type)  << "] " 
			<< configItems.at(index).variable 
			<< "(" << configItems.at(index).value.size() << ")" << " =\t";

		bool first = true;

		for(int i=0; i < (int)configItems.at(index).value.size(); i++){
			if(first){
				first = false;
			}else{
				tmp << ", ";
			}
			tmp << "\"" << configItems.at(index).value.at(i) << "\"";
		}

		LOG(tmp.str(), loglevel);
	}else{
		LOG("Trying to get config item with index out of bounds.", LOG_WARNING);
	}
	
}

void Config::dumpConfig()
{
	LOG("", LOG_INFO);
	LOG("Current config", LOG_INFO);
	LOG("", LOG_INFO);
		LOG("[   origin   ] item(num vals) =\t\"value 1\"(, \"value 2\", \"value 3\", ..., \"value n\")", LOG_INFO);
	LOG("", LOG_INFO);
	for(int i=0; i < (int)configItems.size(); i++){
		printItem(i, LOG_INFO);
	}
	LOG("", LOG_INFO);
}

bool Config::setValue(std::string variable, std::string value, int type){
	return setAddValue(C_SET, variable, value, type);
}

bool Config::setValue(std::string variable, std::string value)
{
	return setValue(variable, value, VAR_INTERNAL);
}

// Add a value only if it doesn't exist
bool Config::overlayValue(std::string variable, std::string value)
{
	if(lookUp(variable) == -1){
		ConfigItem tmp;
		tmp.value.push_back(value);
		tmp.variable = variable;
		configItems.push_back(tmp);
		return true;
	}
	return false;
}

bool Config::setAddValue(int action, std::string variable, std::string value, int type){
	int pos = lookUp(variable);
	bool deleted=false;

	if(variable == "homedir"){
		LOG(variable << " " << value, LOG_DEBUG);
	}

	if(action == C_SET && pos != -1 && configItems.at(pos).type <= type){
		delValue(variable);
		deleted = true;
	}

	if(action == C_ADD && pos != -1 && configItems.at(pos).type < type){
		delValue(variable);
		deleted = true;
	}

	if((pos = lookUp(variable)) !=-1){
		if(configItems.at(pos).type <= type){
			configItems.at(pos).value.push_back(value);
			return true;
		}
	}else{
		if(type == VAR_INTERNAL || deleted || variable.substr(0, 5) == "user_"){
			ConfigItem tmp;
			tmp.value.push_back(value);
			tmp.variable = variable;
			tmp.type = type;
			configItems.push_back(tmp);
			return true;
		}else{
			LOG("Unknown option: '" << variable << "'", LOG_ERROR);
		}
	}
	return false;
}
		
bool Config::addValue(std::string variable, std::string value, int type)
{
	return setAddValue(C_ADD, variable, value, type);
}

bool Config::addValue(std::string variable, std::string value)
{
	return addValue(variable, value, VAR_INTERNAL);
}

bool Config::delValue(std::string variable)
{
	std::vector< ConfigItem >::iterator iter;
	
	for(iter = configItems.begin(); iter != configItems.end(); iter++){
		if((*iter).variable == variable){
			configItems.erase(iter);
			return true;
		}
	}
	return false;
}
std::string Config::getValueStr(std::string variable, std::string separator){
	return getValueStr(variable, "", separator, "");
}


std::string Config::getValueStr(std::string variable, std::string addBefore, std::string separator)
{
	return getValueStr(variable, addBefore, separator, "");
}

std::string Config::getValueStr(std::string variable, std::string addBefore, std::string separator, std::string addAfter)
{
	std::string ret;
	if(lookUp(variable) != -1 && getValueStr(variable, 0) != ""){
		ret.append(addBefore);
		bool first = true;
		for(int i=0; i < getNumValues(variable); i++){
			if(first){
				first = false;
			}else{
				ret.append(separator);
			}
			ret.append(getValueStr(variable, i));
		}
		ret.append(addAfter);
	}
	return ret;
}

bool Config::loadConfig(std::string filename, int depth)
{
	if(depth > 255){
		LOG("Too many includes (255). Are you perhaps including two project files from eachother?", LOG_ERROR);
		return false;
	}

	std::ifstream in(filename.c_str());
	std::istringstream parse;
	std::string get;
	int i=0, lineCount=0;
	bool goodItem = false;
	ConfigItem item;

	if( !in.good() ){
		//LOG("Can't read from the file, is it empty?", LOG_WARNING);

		return false;
	}	
	
	LOG("Loading project file " << filename, LOG_INFO);

	//configItems.clear();
	
	while( in.good() ){
		i=0;
		item.variable = "";
		goodItem = false;

		item.value.clear();
		std::getline(in, get);
		lineCount++;	

		parse.str(get);
		
//		LOG("Parsing the line: " << parse.str(), LOG_DEBUG);
		
		while( parse >> get ){
			if(get.length() > 0 && get.at(0) == '#'){
				break;
			}

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

				if(!done && missingQuote){
					LOG("Syntax error in project file '" << filename << "', line: " << lineCount, LOG_FATAL);
					LOG("Unterminated quotation (\")", LOG_FATAL);
					exit(1);
				}
			}

			if(i == 0){
				transform(get.begin(), get.end(), get.begin(), tolower);
				item.variable = get;
//				LOG("Variable: " << get, LOG_DEBUG);
			}else{
//				LOG("Adding value: " << get, LOG_DEBUG);
				item.value.push_back(get);
				goodItem = true;
			}
			i++;
		}
		
		if(goodItem){
//			LOG("Adding this good item", LOG_DEBUG);
			//configItems.push_back(item);
			for(int x=0; x < (int)item.value.size(); x++){
				if(!addValue(item.variable, item.value.at(x), VAR_PROJECT) && lookUp(item.variable) == -1){
					LOG("Syntax error in project file '" << filename << "', line: " << lineCount, LOG_FATAL);
					exit(1);
				}
			}

			if(item.variable == "include"){
				for(std::vector<std::string>::iterator it = item.value.begin(); it != item.value.end(); it++){
					if(!loadConfig(*it, depth + 1)){
						LOG("In file: " << filename << ", line: " << lineCount << " couldn't include '" << *it << "'", LOG_FATAL);
						exit(1);
					}
				}
			}
		}
		
		parse.clear();		
	}
	return true;
}

int Config::lookUp(std::string variable){
	std::string get = variable;
	transform(get.begin(), get.end(), get.begin(), tolower);
	for(unsigned int i=0; i < configItems.size(); i++){
		if(configItems.at(i).variable == get){
			return i;
		}
	}
	return -1;
}

bool Config::getValueBool(std::string variable, int index){
	std::string tmp = getValueStr(variable, index);
	if(tmp == "true" || tmp == "yes" || tmp == "1" || tmp == "si" || tmp == "ja" || tmp == "da" || tmp == "oui" || tmp == "jawohl"){
		return true;
	}else{
		return false;
	}
}

std::string Config::getValueStr(std::string variable, int index, int depth)
{
	//LOG("Looking up: " << variable, LOG_DEBUG);

	if(depth > SPANK_MAX_RECURSE){
		LOG("Recurse limit (" << SPANK_MAX_RECURSE << ") exceeded in Config::getValueStr(), probably because of variable self referencing (or indirect self referencing).", LOG_ERROR);
		LOG("Might be in: " << variable << "[" << index << "]", LOG_INFO);

		return "";
	} 

	int item;
	item = lookUp(variable);
	if(item != -1){
		if((unsigned int)index <= configItems.at(item).value.size()){
			std::string ret = configItems.at(item).value.at(index);

			//LOG("Parsing string: " << ret, LOG_DEBUG);

			size_t start = 0;

			while((start = ret.find("$("), start) != std::string::npos){
				size_t stop = ret.find(")", start);

				//LOG("start: " << start, LOG_DEBUG);
				//LOG("stop: " << stop, LOG_DEBUG);

				if(stop == std::string::npos){
					LOG("Error parsing variable refernce: expected ')' near '" << ret << "'", LOG_ERROR);
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

	char* env = getenv(variable.c_str());
	if(env){
		return env;
	}

	LOG("Couldn't find config variable: '" << variable << "'", LOG_WARNING);
	return "";
}

int Config::getValueInt(std::string variable, int index)
{
	int item;
	item = lookUp(variable);
	if(item != -1){
		if((unsigned int)index <= configItems.at(item).value.size()){
			return atoi(configItems.at(item).value.at(index).c_str());
		}
	}

	LOG("Couldn't find config variable " << variable, LOG_WARNING);
	return -1;
}

int Config::getNumValues(std::string variable)
{
	int item = lookUp(variable);
	if(item != -1){
		if(configItems.at(item).value.size() == 1 && configItems.at(item).value.at(0) == ""){
			return 0;
		}
		return configItems.at(item).value.size();
	}
	return 0;
}


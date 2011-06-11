#include <cstdlib>

#include "installers/installerunix.h"
#include "project.h"
#include "tools.h"
#include "macros.h"

void InstallerUnix::setPrefix()
{
	if(PROJECT->getValueStr("inst_prefix") == ""){
		PROJECT->setValue("inst_prefix", "/usr/local/");
	}
}

bool InstallerUnix::install(bool fake)
{
	FORMSTR(mkdir, PROJECT->getValueStr("inst_sudo") <<  " mkdir -p ");
	FORMSTR(cp, PROJECT->getValueStr("inst_sudo") <<  " cp ");
	FORMSTR(scriptPrefix, PROJECT->getValueStr("inst_sudo") <<  " ");

	LOG("Making directories", LOG_VERBOSE);
	if(!Tools::executeAll("inst_mkdir", mkdir, fake)){
		LOG("Couldn't create all directories", LOG_FATAL);
		return false;
	}

	LOG("Executing pre-copy scripts", LOG_VERBOSE);
	if(!Tools::executeAll("inst_prescript", scriptPrefix, fake)){
		LOG("Failed to execeute one or more pre-copy install script", LOG_FATAL);
		return false;
	}

	LOG("Copying files", LOG_VERBOSE);
	for(int i=0; i < PROJECT->getNumValues("inst_copy"); i += 2){

		std::string from = PROJECT->getValueStr("inst_copy", i);
		std::string to = PROJECT->getValueStr("inst_copy", i + 1);

		FORMSTR(cmd, cp << from << " " << to);

		LOG("Installing: " << from << " -> " << to, LOG_INFO);

		if(fake){
			LOG("[not executing] ... " << cmd, LOG_INFO);
		}else{
			if(system(cmd.c_str()) != 0){
				LOG("Couldn't copy file: '" << cmd << "' returned non-zero.", LOG_FATAL);
				return false;
			}
		}
	}
	
	LOG("Executing post-copy scripts", LOG_VERBOSE);
	if(!Tools::executeAll("inst_postscript", scriptPrefix, fake)){
		LOG("Failed to execeute one or more pre-copy install script", LOG_FATAL);
		return false;
	}

	return true;
}

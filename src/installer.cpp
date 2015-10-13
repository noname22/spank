#include "installer.h"
#include "project.h"

bool Installer::install(bool fake)
{
	return false;
}

void Installer::setPrefix()
{
}

bool Installer::checkInstallable()
{
	if(
		!(PROJECT->getNumValues("inst_copy") == 1 && PROJECT->getValueStr("inst_copy") == "") &&
		!(PROJECT->getNumValues("inst_prescript") == 1 && PROJECT->getValueStr("inst_prescript") == "") &&
		!(PROJECT->getNumValues("inst_postscript") == 1 && PROJECT->getValueStr("inst_postscript") == "")
	){
		LOG("Nothing to install.", LOG_ERROR);
		return false;
	}
	
	if(PROJECT->getNumValues("inst_copy") & 1){
		LOG("inst_copy from/to mismatch (it's inst_copy <from> <to> (<from> <to> ... ), nothing else)", LOG_FATAL);
		return false;
	}

	return true;
}

// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include "system.h"

#include <string>
#include <cstdlib>

#include "project.h"
#include "macros.h"

template<> FilesUnix* SFilesUnix::instance = 0;

template<> CompilerGcc* SCompilerGcc::instance = 0;
template<> CompilerMcs* SCompilerMcs::instance = 0;
template<> CompilerVala* SCompilerVala::instance = 0;

template<> Export* SExport::instance = 0;
template<> ExportShell* SExportShell::instance = 0;
template<> ExportMakefile* SExportMakefile::instance = 0;

template<> Installer* SInstaller::instance = 0;
template<> InstallerUnix* SInstallerUnix::instance = 0;
template<> InstallerDeb* SInstallerDeb::instance = 0;

Files* System::getFilesInstance()
{
	return (Files*)SFilesUnix::getInstance();
}

Compiler* System::getCompilerInstance()
{
	std::string compilertype = PROJECT->getValueStr("compilertype");

	if(compilertype == "gcc")
		return (Compiler*)SCompilerGcc::getInstance();

	else if(compilertype == "mcs")
		return (Compiler*)SCompilerMcs::getInstance();

	else if(compilertype == "vala")
		return SCompilerVala::getInstance();

	ThrowEx(SystemException, "Unknown compiler type: '" << compilertype << "'");
	return NULL;
}

Export* System::getExportInstance()
{
	std::string exporter = PROJECT->getValueStr("exporter", 0);
	if(exporter == "sh"){
		return (Export*)SExportShell::getInstance();
	}
	
	else if(exporter == "makefile"){
		return (Export*)SExportMakefile::getInstance();
	}

	else{
		LOG("No such exporter: '" << exporter << "'", LOG_ERROR);
		return SExport::getInstance();
	}
}

Installer* System::getInstallerInstance()
{
	std::string installer = PROJECT->getValueStr("installer");

	if(installer == "unix"){
		LOG("Using installer: unix", LOG_EXTRA_VERBOSE);
		return SInstallerUnix::getInstance();
	}

	if(installer == "deb"){
		LOG("Using installer: deb", LOG_EXTRA_VERBOSE);
		return SInstallerDeb::getInstance();
	}

	else{
		LOG("No such installer: '" << installer << "'", LOG_ERROR);
		return SInstaller::getInstance();
	}
}

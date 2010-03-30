// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include <fstream>
#include <cstdlib>

#include "installers/installerdeb.h"
#include "system.h"
#include "project.h"
#include "tools.h"
#include "macros.h"

void InstallerDeb::setPrefix()
{
	if(PROJECT->getValueStr("inst_prefix") == ""){
		PROJECT->setValue("inst_prefix", "/usr/");
	}
}

#define IF_EXISTS_ADD(__c, __desc) if(PROJECT->getValueStr(__c) != ""){control << __desc << ": " << PROJECT->getValueStr(__c) << std::endl;}

bool InstallerDeb::writeControlFile()
{
	FORMSTR(cFile, controlDir << "/control");
	LOG("Writing debian control file to: " << cFile, LOG_VERBOSE);

	std::ofstream control(cFile.c_str());

	arch = PROJECT->getValueStr("inst_arch");

	if(arch == "host"){
		arch = getArch();
	}

	control << "Package: " << Tools::filenameify(Tools::toLower(PROJECT->getValueStr("name"))) << std::endl;
	control << "Version: " << Tools::filenameify(Tools::toLower(PROJECT->getValueStr("version"))) << std::endl;
	control << "Architecture: " << arch << std::endl;
	control << "Maintainer: " << PROJECT->getValueStr("inst_maintainer") << " <" << PROJECT->getValueStr("inst_maintainer_email") << ">" << std::endl;

	IF_EXISTS_ADD("inst_deb_depends", "Depends");
	IF_EXISTS_ADD("inst_deb_replaces", "Replaces");
	IF_EXISTS_ADD("inst_deb_conflicts", "Conflicts");
	IF_EXISTS_ADD("inst_deb_section", "Section");
	IF_EXISTS_ADD("inst_deb_priority", "Priority");
	IF_EXISTS_ADD("inst_deb_installedsize", "Installed-Size");
	IF_EXISTS_ADD("inst_deb_originalmaintainer", "Original-Maintainer");
	IF_EXISTS_ADD("inst_deb_source", "Source");

	control << "Description: " << PROJECT->getValueStr("description") << std::endl;

	int num;
	if((num = PROJECT->getNumValues("description")) > 1){
		for(int i = 1; i < num; i++){
			control << " " << PROJECT->getValueStr("description", i) << std::endl;
		}
	}

	control.close();

	return true;
}

bool InstallerDeb::writeMd5sumFile()
{
	LOG("Writing md5sum file to: " << controlDir << "/md5sums", LOG_VERBOSE);
	FORMSTR(cmd, "cd " << dataDir << " && md5sum " << Tools::joinStrings(fileList));
	Tools::execute(cmd, "../control/md5sums");
	return true;
}

bool InstallerDeb::copyFiles()
{
	LOG("Copying files", LOG_VERBOSE);
	for(int i=0; i < PROJECT->getNumValues("inst_copy"); i += 2){

		std::string from = PROJECT->getValueStr("inst_copy", i);
		std::string baseTo = PROJECT->getValueStr("inst_copy", i + 1);

		// TODO 
		// bit of a hack. I want to report inst_prefix as (default) "/usr" to the application as SPANK_PREFIX
		// but "usr" to the tools (md5sum for instance)

		if(baseTo[0] == '/'){
			baseTo.erase(0, 1);
		}

		FORMSTR(to, dataDir << "/" << baseTo);

		FILES->createDir(FILES->pathSplit(to).first);
		if(!FILES->copy(from, to)){
			LOG("Couldn't copy file: " << from << " to " << to, LOG_FATAL);
			/* TODO gah, exceptions */
			exit(1);
			return false;
		}

		fileList.push_back(baseTo);
	}

	return true;
}

bool InstallerDeb::install(bool fake)
{
	if(!checkInstallable()){
		return false;
	}

	fileList.clear();

	// Create deb dir in project temp dir	
	ADDSTR(debDir, FILES->getTmpDirStr() << "/deb");

	FILES->removeDir(debDir);
	FILES->createDir(debDir);
	
	// Create control dir in deb dir
	ADDSTR(controlDir, debDir << "/control");
	if(!FILES->fileExists(controlDir)){
		FILES->createDir(controlDir);
	}
	
	// Create data dir in deb dir
	ADDSTR(dataDir, debDir << "/data");
	if(!FILES->fileExists(dataDir)){
		FILES->createDir(dataDir);
	}

	FORMSTR(binaryFile, debDir << "/debian-binary");

	writeControlFile();
	copyFiles();
	writeMd5sumFile();

	LOG("Writing debian binary file to: " << binaryFile, LOG_VERBOSE);
	FILES->fileFromStr(binaryFile, "2.0\n");

	LOG("Compressing data.", LOG_VERBOSE);
	
	FORMSTR(dataCmd, PROJECT->getValueStr("tar") << " -czv --owner root --group root -C " << dataDir << " -f " << debDir << "/data.tar.gz .");
	Tools::execute(dataCmd);
	
	LOG("Compressing control.", LOG_VERBOSE);
	
	FORMSTR(controlCmd, PROJECT->getValueStr("tar") << " -czv --owner root --group root -C " << controlDir << " -f " << debDir << "/control.tar.gz .");
	Tools::execute(controlCmd);

	LOG("Assembling package.", LOG_VERBOSE);

	std::string packageName;

	if(PROJECT->getValueStr("targettype") != "binary"){
		packageName = PROJECT->getValueStr("inst_libpackagename");
	}else{
		packageName = PROJECT->getValueStr("inst_packagename");
	}

	FORMSTR(pkgName, Tools::toLower(packageName) << "-" << Tools::toLower(PROJECT->getValueStr("version")) << "_" << arch << ".deb");
	pkgName = Tools::filenameify(pkgName);

	FORMSTR(packageCmd,
		"cd " << debDir << " && " <<
		PROJECT->getValueStr("ar") << " rcs " << pkgName << " debian-binary control.tar.gz data.tar.gz"
	);

	Tools::execute(packageCmd);
	
	FORMSTR(pkgPath, debDir << "/" << pkgName);
	FILES->copy(pkgPath, pkgName);

	LOG("Finished building package: " << pkgName, LOG_INFO);

	return true;
}

std::string InstallerDeb::getArch()
{
	std::string out; 

	if(!Tools::execute("dpkg-architecture -qDEB_BUILD_ARCH", &out)){
		return out.substr(0, out.length() - 1);
	}
		
	LOG("Failed to execute dpkg-architecture.", LOG_ERROR);
	LOG("Please install dpkg-architecture or specify inst_arch in your project file (not 'host').", LOG_INFO);
	LOG("Defaulting to i386", LOG_WARNING);

	return "i386";
}

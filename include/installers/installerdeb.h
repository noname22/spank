// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef INSTALLERDEB_H
#define INSTALLERDEB_H

#include <vector>

#include "installer.h"

class InstallerDeb: public Installer
{
	public:
		bool install(bool fake);
		void setPrefix();

	private:
		std::string getArch();
		bool writeControlFile();
		bool copyFiles();
		bool writeMd5sumFile();

		std::vector<std::string> fileList;
		std::string debDir, controlDir, dataDir, arch;
};

#endif


// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef INSTALLER_H
#define INSTALLER_H

#include <stdexcept>

class InstallerException : public std::runtime_error {
	public:
	InstallerException(std::string str) : std::runtime_error(str) {}
};

class Installer
{
	public:
		virtual bool install(bool fake);
		virtual void setPrefix();

	protected:
		bool checkInstallable();
		
};

#endif

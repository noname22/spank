// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef INSTALLERUNIX_H
#define INSTALLERUNIX_H

#include "installer.h"

class InstallerUnix: public Installer
{
	public:
		bool install(bool fake);
		void setPrefix();
};

#endif

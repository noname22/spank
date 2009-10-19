// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef EXPORTSHELL_H
#define EXPORTSHELL_H

#include "export.h"

class ExportShell: public Export
{
	public:
		bool exp(std::string fileName);
};

#endif

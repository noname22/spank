// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef EXPORTMAKEFILE_H
#define EXPORTMAKEFILE_H

#include "export.h"

class ExportMakefile: public Export
{
	public:
		bool exp(std::string fileName);
};

#endif


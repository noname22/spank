// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef EXPORT_H
#define EXPORT_H

#include <string>

class Export
{
	public:
		virtual bool exp(std::string fileName);
		virtual ~Export();
};

#endif

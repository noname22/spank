// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef COMPILERVALA_H
#define COMPILERVALA_H

#include "compiler.h"
#include <vector>
#include <string>

class CompilerVala: public Compiler
{
	public:
		void localCompile();
		void localLink();
		
		std::vector<CList> compileList(const StrSet& sourceList, bool rcCheck = true);
		std::string getLdCall(bool rlCheck);
};

#endif

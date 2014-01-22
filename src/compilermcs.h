// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef COMPILERMCS_H
#define COMPILERMCS_H

#include "compiler.h"
#include <vector>
#include <string>

class CompilerMcs: public Compiler
{
	public:
		void localCompile();
		void localLink();
		
		std::vector<CList> compileList(bool rcCheck = true);
		std::string getLdCall(bool rlCheck);

	private:
		bool checkTarget(std::string target);
};

#endif

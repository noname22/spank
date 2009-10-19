// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef COMPILERGCC_H
#define COMPILERGCC_H

#include "compiler.h"
#include <vector>
#include <string>

class CompilerGcc: public Compiler
{
	public:
		CompilerGcc();

		bool localCompile();
		bool localLink();
		
		std::vector<CList> compileList(bool rcCheck = true);
		std::string getLdCall(bool rlCheck);
	private:
		bool checkRecompile(std::string src, std::string obj);
		void markRecompile(std::string src, std::string obj);
		bool checkLibs();
		bool pkgCall(std::string switches);
		std::string pkgGetFlags(std::string lib, bool cflags);

		std::string getPercent(int current, int of);

		bool hasPkgConfig;
};

#endif

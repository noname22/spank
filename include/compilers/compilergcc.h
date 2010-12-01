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
		bool checkRecompileRecursive(std::string src, std::string obj, int depth = 0);
		bool checkRecompile(std::string src, std::string obj);
		void markRecompile(std::string src, std::string obj);
		bool checkLibs();
		bool pkgCall(std::string switches);
		
		std::string pkgGetFlags(std::string lib, bool cflags); // TODO remove this

		std::string getPercent(int current, int of);

		void setIncludePaths();
		std::string lookUpIncludeFile(std::string filename, bool quoted);

		bool hasPkgConfig;
		enum IncPathType { Quoted, Bracket };
		std::vector<std::string> incPaths[2]; // first is quoted paths, second is <> paths
};

#endif

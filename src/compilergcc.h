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
#include <set>

class CompilerGcc: public Compiler
{
	public:
	CompilerGcc();

	bool localCompile();
	bool localLink();
	
	std::set<std::string> getSourceList();
	std::vector<CList> compileList(bool rcCheck = true);
	std::string getLdCall(bool rlCheck);
	std::string genCFlags(std::string filename, bool includeLibs = false, std::string language = "");
	std::string parseCFlags(std::string valName);

	private:
	bool compileFileByFile();
	bool compileSingleCall();
	bool compileAmalgamate();

	bool checkRecompileRecursive(std::vector<std::string> stack, std::string src, std::string obj, int depth = 0);
	bool checkRecompile(std::string src, std::string obj);
	void markRecompile(std::string src, std::string obj);
	bool checkLibs();
	bool pkgCall(std::string switches);

	std::string guessLanguage(std::string filename);
	
	std::string pkgGetFlags(std::string lib, bool cflags); // TODO remove this

	std::string getPercent(int current, int of);

	void setIncludePaths(std::string filename);
	std::string lookUpIncludeFile(std::string src, std::string filename, bool quoted);

	bool hasPkgConfig;
	enum IncPathType { Quoted, Bracket };
	std::vector<std::string> incPaths[2]; // first is quoted paths, second is <> paths
};

#endif

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
#include "tools.h"

class CompilerGcc: public Compiler
{
	public:
	CompilerGcc();

	bool localCompile();
	bool localLink();
	
	StrSet getSourceList();
	std::vector<CList> compileList(bool rcCheck = true);
	std::string getLdCall(bool rlCheck);
	std::string genCFlags(std::string filename, bool includeLibs = false, std::string language = "");
	std::string parseCFlags(std::string valName);

	private:
	bool compileFileByFile(StrSet list);
	bool compileSingleCall(StrSet list);
	bool compileAmalgamate(StrSet list);

	bool checkRecompileRecursive(StrVec stack, std::string src, std::string obj, int depth = 0);
	bool checkRecompile(std::string src, std::string obj);
	void markRecompile(std::string src, std::string obj);
	bool checkLibs();
	bool pkgCall(std::string switches);
	StrSet getStdLibs(StrSet sources);

	std::string guessLanguage(std::string filename);
	std::string compilerFromLanguage(std::string lang);
	
	std::string pkgGetFlags(std::string lib, bool cflags); // TODO remove this

	std::string getPercent(int current, int of);

	void setIncludePaths(std::string filename);
	std::string lookUpIncludeFile(std::string src, std::string filename, bool quoted);

	bool hasPkgConfig;
	enum IncPathType { Quoted, Bracket };
	StrVec incPaths[2]; // first is quoted paths, second is <> paths
	StrSet sources;
};

#endif

// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <stdexcept>

class CompilerException : public std::runtime_error {
	public:
	CompilerException(std::string str) : std::runtime_error(str) {}
};

struct CList{
	std::string src;
	std::string obj;
	std::string call;
};

class Compiler
{
	public:
	void compile();
	void link();
	void clean();

	virtual std::string genCFlags(std::string filename, bool includeLibs = false, std::string language = "");

	virtual void localCompile() = 0;
	virtual void localLink() = 0;
	virtual void localClean();

	virtual ~Compiler();

	virtual std::string getLdCall(bool rlCheck) = 0;
	virtual std::vector<CList> compileList(bool rcCheck = true) = 0;

	virtual bool checkLibs();

	protected:
	void buildDeps();
	bool checkExclude(std::string src);
	int countLines(std::string name); 
};

#endif


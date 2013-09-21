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

struct CList{
	std::string src;
	std::string obj;
	std::string call;
};

class Compiler
{
	public:
	bool compile();
	bool link();
	bool clean();

	virtual std::string genCFlags(bool includeLibs = false);

	virtual bool localCompile() = 0;
	virtual bool localLink() = 0;
	virtual bool localClean();

	virtual ~Compiler();

	virtual std::string getLdCall(bool rlCheck) = 0;
	virtual std::vector<CList> compileList(bool rcCheck = true) = 0;

	virtual bool checkLibs();

	protected:
	bool buildDeps();
	bool checkExclude(std::string src);
	int countLines(std::string name); 
};

#endif


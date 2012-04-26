// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef SPANK_H
#define SPANK_H

#include <iostream> 
#include <vector>

enum{
	BANNER_LOGO,
	BANNER_USAGE,
	BANNER_SEEHELP
};

enum{
	TEMPLATE_DEFAULT,
	TEMPLATE_C,
	TEMPLATE_C99,
	TEMPLATE_CS,
	TEMPLATE_CPP,
	TEMPLATE_CPP11,
	TEMPLATE_VALA
};

class Spank{
	public:
		Spank(int argc, char** argv);

	private:
	
		void setTemplate(int type);

		void printBanner(int banner);
		void handleArgs(int argc, const char* const* argv);

		int getAction();

		bool templateOnce;
};


#endif

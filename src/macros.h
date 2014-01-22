#ifndef MACROS_H
#define MACROS_H

#include <string>
#include <sstream>

#define MAX(a, b) (a) < (b) ? (b) : (a)	
#define MIN(a, b) (a) > (b) ? (b) : (a)	

#define SETSTR(__str, __add) \
        do{\
        std::stringstream __tmp;\
        __tmp << __add;\
        __str = __tmp.str();\
}while(0);

#define FORMSTR(__str, __add) \
        std::string __str; \
        do{\
        std::stringstream __tmp;\
        __tmp << __add;\
        __str = __tmp.str();\
}while(0);

#define ADDSTR(__str, __add) \
	do{\
		std::stringstream __tmp;\
		__tmp << __str << __add;\
		__str = __tmp.str();\
	}while(0);

// throws an exception of the type _ex if _exp is false, _msg accepts stream input
#define AssertEx(_exp, _ex, _msg) if(!(_exp)){ FORMSTR(_mstr, _msg); throw _ex(_mstr); }

// throws an exception of the type _ex, _msg accepts stream input
#define ThrowEx(_ex, _msg) do{ FORMSTR(_mstr, _msg); throw _ex(_mstr); } while(0);

#endif

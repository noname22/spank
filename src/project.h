// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#ifndef PROJECT_H
#define PROJECT_H

#include "singleton.h"
#include "config.h"

#define PROJECT Project::getInstance()

typedef Singleton<Config> Project;

#endif

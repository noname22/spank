// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

// This class contains functions returning base-class pointers to approperiate singletons.
// Like, if you're on a unix-system, getFilesInstance() returns "Files"-pointer to "FilesUnix" singleton instance

#include "singleton.h"

#include "filesunix.h"
#include "compilers/compilergcc.h"
#include "compilers/compilermcs.h"
#include "compilers/compilervala.h"
#include "exporters/exportshell.h"
#include "exporters/exportmakefile.h"

#include "installers/installerunix.h"
#include "installers/installerdeb.h"

#define FILES System::getFilesInstance()
#define COMPILER System::getCompilerInstance()
#define EXPORT System::getExportInstance()
#define INSTALLER System::getInstallerInstance()

// All file enviroments (only one atm.)
typedef Singleton<FilesUnix> SFilesUnix;

// All compiler systems
typedef Singleton<CompilerGcc> SCompilerGcc;
typedef Singleton<CompilerMcs> SCompilerMcs;
typedef Singleton<CompilerVala> SCompilerVala;

// All exporters
typedef Singleton<Export> SExport; 			// Default exporter (always fails)
typedef Singleton<ExportShell> SExportShell; 		// I'm bringin' sexy back (yeah)
typedef Singleton<ExportMakefile> SExportMakefile; 	// Makefile

// All installers
typedef Singleton<Installer> SInstaller;
typedef Singleton<InstallerUnix> SInstallerUnix;
typedef Singleton<InstallerDeb> SInstallerDeb;

class System
{
	public:
		static Files* getFilesInstance();
		static Compiler* getCompilerInstance();
		static Export* getExportInstance();
		static Installer* getInstallerInstance();
};

SPANK!

About:
	Spank is a minimal build system mainly developed for the C and C++ parts of GCC.
	It features a very easy setup for small projects or test-code.

Autors:
	Right now, only me, Fredrik Hultin

Contact:
	Homepage: 	http://nurd.se/~noname/spank
	E-mail: 	noname @ the very same domain

Requirements:
	Spank requires find and md5sum to work properly. For the default recompilation check to work it needs the c preprocessor (cpp), this can be avoided by specifying the basic recompilation check with the argument -rcceck simple. Spank also requires gcc if you want to use it for anything.

Elusive bugs and limitations:
	- In some rare cases spank fails to realize that a file, that has syntax errors, needs to be rebuilt.
	- The debian package generator doesn't handle dependencies
	- The documentation for the project is lacking, will be completed with v. 1.0

Changelog:
	Version 0.9.1
		- Fixed bug in preprocessor recompile check
		- Fixed bug where spank tried to build everything in a directory, even if there was no project, when a ~/.spank/config.spank existed
		- Fixed bug in deb generator where version number contained spaces

	Version 0.9 (2009/05/15)
		- Installers: spank can install projects and build packages (-installer <installer> install). Right now:
			- Debian packages
			- Installation on unix style systems
		- Multiple jobs support (-jobs <n>, can also be set in ~/.spank/config.spank as jobs <n>)
		- More bug fixes

	Version 0.7
		- Exporters now have their own temp dir
		- build.sh works again, spank can be built w/o spank again (doh!)
		- Loads ~/.spank/config.spank for local configuration
		- The temp directory is of course ~/.spank/<project>.tempfiles, and not ~/.femtoxml/ (double doh!)
		- More bug fixes

	Version 0.6
		- The exporters now support pkg-config
		- Temp directory is now in ~/.femtoxml/... rather than in the project dir
		- More bug fixes

	Version 0.5
		- Support for library dependencies via pkg-config
		- More bug fixes in preprocessor recompilation check in gcc
		- Support for mono c# compiler (template cs)
		- Minor cosmetic changes
		- Probably more bug fixes, see svn changelog (if you can ;).

	Version 0.3
		- Better output when there's nothing to do ("Nothing to do." rather than "Compiling, linking etc.")
		- Makefile script exporter
		- Bug fix in preprocessor recompilation check
		- Probably more bug fixes, see svn changelog (if you can ;).

	Version 0.2
		- Compiler modulization
		- Exporter modulization
		- Shell script exporter
		- Using preprocessor to check if a file needs recompilation

	Version 0.1
		- Initial release

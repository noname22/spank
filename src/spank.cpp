// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include <cstdlib>
#include <stdexcept>

#include "spank.h"

#include "settings.h"
#include "project.h"
#include "tools.h"
#include "system.h"
#include "macros.h"

class SpankException : public std::runtime_error {
	public:
	SpankException(std::string str) : std::runtime_error(str) {}
};

int Spank::run(int argc, char** argv)
{
	try { 
		handleArgs(argc, argv);

		Log::getInstance()->restrictDebugOutputToFiles(PROJECT->getValues("debug-only-files"));

		if(!FILES->createDir(FILES->getHomeDir())){
			return 1;
		}

		FILES->initializeTmpDir();
		
		if(PROJECT->getValueStr("spank") == ""){
			// if spank is started from a relative path, like ../spank
			// do a realpath lookup for it to make sure that's the command used
			// when recursing.

			std::string spankBin = FILES->realpath(argv[0]);

			if(spankBin == "")
				spankBin = argv[0];			

			PROJECT->setValue("spank", spankBin);
		}

		// compare against last extraarg and trigger rebuild if they differ, since then it's a new configuration
		std::string extraarg = PROJECT->getValueStr("extraarg");
		std::string lastExtraarg;

		if(Tools::loadTempValue("extraarg", lastExtraarg)){
			LOG("last extraarg: " << lastExtraarg, LOG_DEBUG);
			LOG("current extraarg: " << extraarg, LOG_DEBUG);

			if(extraarg != lastExtraarg){
				LOG("building with new build configuration, forcing clean (if building)", LOG_VERBOSE);
				PROJECT->setValue("forceclean", "true");
			}
		}

		Tools::saveTempValue("extraarg", extraarg);
		std::string action = PROJECT->getValueStr("action");
			
		// forcing clean, if requested
		if(PROJECT->getValueBool("forceclean") && (action != "clean" && action != "rebuild")){
			LOG("Clean forced", LOG_DEBUG);
			COMPILER->clean();
			FILES->initializeTmpDir();
			// HACK
			Tools::saveTempValue("extraarg", extraarg);
		}

		if(action == "build"){
			LOG("Action: build", LOG_DEBUG);

			COMPILER->compile();
			COMPILER->link();
			postBuild();

			LOG("done building", LOG_VERBOSE);
		}
		
		else if(action == "install"){
			LOG("Action: install", LOG_DEBUG);
			
			INSTALLER->setPrefix();
			COMPILER->compile();
			COMPILER->link();
			postBuild();
			INSTALLER->install(false);
			
			LOG("done installing", LOG_VERBOSE);
		}

		else if(action == "fake-install"){
			LOG("Action: fake-install", LOG_DEBUG);

			INSTALLER->install(true);

			LOG("done fake-installing", LOG_VERBOSE);
		}

		else if(action == "rebuild"){
			LOG("Action: rebuild", LOG_DEBUG);
			COMPILER->clean();
			LOG("done cleaning", LOG_VERBOSE);

			FILES->initializeTmpDir();

			// HACK since clean deletes the temp vars but the project is going to be rebuilt
			//      the extraarg needs to be resaved, or rebuild -> build with different
			//      configurations isn't going to trigger force clean

			Tools::saveTempValue("extraarg", extraarg);

			COMPILER->compile();
			COMPILER->link();
			postBuild();

			LOG("done rebuilding", LOG_VERBOSE);
		}

		else if(action == "clean"){
			LOG("Action: clean", LOG_DEBUG);

			COMPILER->clean();

			LOG("done cleaning", LOG_VERBOSE);
		}

		else if(action == "export"){
			LOG("Action: export", LOG_DEBUG);

			EXPORT->exp( PROJECT->getValueStr("exportfile", 0) );

			LOG("done exporting", LOG_VERBOSE);
		}

		else if(action == "list"){
			LOG("Action: list", LOG_DEBUG);
			LOG("", LOG_INFO);
			std::vector<Section> secs = PROJECT->getSectionList();
			if(secs.size() == 0)
				LOG("no build configurations specified in project file(s)", LOG_INFO);
			else
				Config::printSectionList(secs);
		}

		else{
			printBanner(BANNER_SEEHELP);
		}
	}

	catch (std::runtime_error ex) {
		LOG("failed: " << ex.what(), LOG_FATAL);
		return 1;
	}

	return 0;
}

void Spank::postBuild()
{
	LOG("post build", LOG_DEBUG);
	if(PROJECT->getValueBool("dep_printinfo")){
		std::string target = FILES->realpath(PROJECT->getValueStr("dep_target"));
		LASSERT(target != "", "could not locate target");
		std::cerr << "target: " << target << std::endl;
		std::cerr << "targettype: " << PROJECT->getValueStr("targettype") << std::endl;

		std::string sep = PROJECT->getValueBool("addhyphen") ? "-" : " ", prefix;
		prefix.append(sep);

		std::cerr << "cflags: " << PROJECT->getValueStr("dep_cflags", prefix, sep, " ") << 
			PROJECT->getValueStr("dep_extra_cflags", prefix, sep, " ") <<
			" `" << PROJECT->getValueStr("pkg-config") <<
			PROJECT->getValueStr("lib", " --cflags ", " ", "`") <<
			std::endl;

		std::cerr << "ldflags: " << PROJECT->getValueStr("dep_ldflags", prefix, sep, " ") << 
			PROJECT->getValueStr("dep_extra_ldflags", prefix, sep, " ") << 
			" `" << PROJECT->getValueStr("pkg-config") <<
			PROJECT->getValueStr("lib", " --libs ", " ", "`") <<
			std::endl;
	}
}

void Spank::setDefaultConfig()
{
	std::string currPath = FILES->realpath(".");
	LASSERT(currPath != "", "could not locate current path!");

	// Common defaults
	PROJECT->setValue("project", "project.spank");
	PROJECT->addValue("project", "spank/project.spank");
	PROJECT->setValue("config", "/etc/spank.conf");
	PROJECT->addValue("config", "$(HOME)/.spank/config.spank");
	PROJECT->setValue("target_name", "$(name)");
	PROJECT->setValue("target", "$(target_prefix)$(target_name)$(target_suffix)");
	PROJECT->setValue("sourcedir", ".");
	PROJECT->setValue("action", "none");
	PROJECT->setValue("template", "default");
	PROJECT->setValue("help", "no");
	PROJECT->setValue("tmpdir", "");
	PROJECT->setValue("verbosity", "2");
	PROJECT->setValue("showbuildconfigs", "false");
	PROJECT->setValue("showconfig", "false");
	PROJECT->setValue("ldflags", "");
	PROJECT->setValue("exclude", "");
	PROJECT->setValue("rccheck", "recursive");
	PROJECT->setValue("pp", "$(compiler) -E");
	PROJECT->setValue("linker", "$(compiler)");
	PROJECT->setValue("spankdefs", "true");
	PROJECT->setValue("exporter", "makefile");
	PROJECT->setValue("exportfile", "");
	PROJECT->setValue("lib", "");
	PROJECT->setValue("lib-static", "");
	PROJECT->setValue("pkg-config", "PKG_CONFIG_PATH=$PKG_CONFIG_PATH:.:spank:$(prefix)/lib/pkgconfig pkg-config");
	PROJECT->setValue("find", "find");
	PROJECT->setValue("spank", "");
	PROJECT->setValue("tar", "tar");
	PROJECT->setValue("ar", "$(host_dash)ar");
	PROJECT->setValue("stripsrc", "false");
	PROJECT->setValue("homedir", "$(HOME)/.spank");
	PROJECT->setValue("jobs", "2");
	PROJECT->setValue("targettype", "binary");
	PROJECT->setValue("fpic", "-fPIC");
	PROJECT->setValue("projectpath", currPath);
	PROJECT->setValue("compilation-strategy", "file-by-file");
	PROJECT->setValue("stdlibs", "no");
	PROJECT->setValue("extraarg", "");
	PROJECT->setValue("forceclean", "false");
	PROJECT->setValue("debug-only-files", "");
			
	// building related (reset with templates)
	PROJECT->setValue("compiler", "");
	PROJECT->setValue("sources", "");
	PROJECT->setValue("template", "");
	PROJECT->setValue("compilertype", "");
	PROJECT->setValue("cflags", "");
	PROJECT->addValue("cflags", "");
	PROJECT->setValue("language", "");

	// set to eg. g++ depending on language
	PROJECT->setValue("compiler-from-language", "gcc"); 

	// Inter-project dependencies
	PROJECT->setValue("depends", "");
	PROJECT->setValue("depaction", "build");
	
	PROJECT->setValue("dep_printinfo", "no");

	// TODO merge this with general library info
	PROJECT->setValue("dep_target", "$(target)");
	PROJECT->setValue("dep_cflags", "I$(projectpath)/include");
	PROJECT->setValue("dep_ldflags", "L$(projectpath) -l$(name)");
	PROJECT->setValue("dep_extra_ldflags", "");
	PROJECT->setValue("dep_extra_cflags", "");

	PROJECT->addValue("addhyphen", "true");
	PROJECT->addValue("prebuildscript", "");
	PROJECT->addValue("postbuildscript", "");
	PROJECT->addValue("oncleanscript", "");
	PROJECT->addValue("version", "0.1");
	PROJECT->addValue("name", "untitled_project");
	PROJECT->addValue("homepage", "none");
	PROJECT->addValue("email", "nomail@example.com");
	PROJECT->addValue("author", "author of $(name)");
	PROJECT->addValue("description", "$(name) $(version)");
	PROJECT->addValue("include", "");

	// Target platform defaults (posix/native)

	PROJECT->setValue("target_platform", "posix");
	
	PROJECT->setValue("host", "");
	PROJECT->setValue("host_dash", "");
	PROJECT->setValue("prefix", "/usr/$(host)");

	PROJECT->setValue("binary_prefix", "");
	PROJECT->setValue("binary_suffix", "");
	PROJECT->setValue("lib-shared_prefix", "lib");
	PROJECT->setValue("lib-shared_suffix", ".so.0");
	PROJECT->setValue("lib-static_prefix", "lib");
	PROJECT->setValue("lib-static_suffix", ".a");

	PROJECT->setValue("target_prefix", "$(binary_prefix)");
	PROJECT->setValue("target_suffix", "$(binary_suffix)");

	// Installer defaults

	PROJECT->addValue("inst_maintainer", "$(author)");
	PROJECT->addValue("inst_maintainer_email", "$(email)");
	PROJECT->addValue("inst_packagename", "$(target)");
	PROJECT->addValue("inst_libpackagename", "lib$(target)");

	PROJECT->addValue("installer", "unix");
	PROJECT->addValue("inst_sudo", "sudo");

	PROJECT->addValue("inst_prefix", "");

	PROJECT->addValue("inst_prescript", "");
	PROJECT->addValue("inst_postscript", "");
	PROJECT->addValue("inst_rmprescript", "");
	PROJECT->addValue("inst_rmpostscript", "");

	PROJECT->addValue("inst_copy", "");
	PROJECT->addValue("inst_mkdir", "");

	PROJECT->addValue("inst_arch", "host");

	// Deb installer

	PROJECT->addValue("inst_deb_depends", "");
	PROJECT->addValue("inst_deb_replaces", "");
	PROJECT->addValue("inst_deb_conflicts", "");
	PROJECT->addValue("inst_deb_section", "");
	PROJECT->addValue("inst_deb_priority", "");
	PROJECT->addValue("inst_deb_installedsize", "");
	PROJECT->addValue("inst_deb_originalmaintainer", "");
	PROJECT->addValue("inst_deb_source", "");
}

void Spank::setTemplate(int type)
{
	// Template specific defaults
	switch(type){
		case TEMPLATE_CPP:
		case TEMPLATE_CPP11:
			LOG("Using template: c++", LOG_VERBOSE);
			PROJECT->setValue("compiler", "$(host_dash)g++");
			PROJECT->setValue("sources", "*.cpp");
			PROJECT->setValue("template", "c++");
			PROJECT->setValue("compilertype", "gcc");
			PROJECT->setValue("cflags", "Wall");
			PROJECT->addValue("cflags", "g");
			PROJECT->setValue("language", "c++");
			if(type == TEMPLATE_CPP11)
				PROJECT->addValue("cflags", "std=c++0x");
			break;
		
		case TEMPLATE_VALA:
			LOG("Using template: vala", LOG_VERBOSE);
			PROJECT->setValue("compiler", "valac");
			PROJECT->setValue("sources", "*.vala");
			PROJECT->setValue("template", "vala");
			PROJECT->setValue("rccheck", "simple");
			PROJECT->setValue("compilertype", "vala");
			PROJECT->setValue("language", "vala");
			break;

		case TEMPLATE_CS:
			LOG("Using template: c#", LOG_VERBOSE);
			PROJECT->setValue("compiler", "gmcs");
			PROJECT->setValue("sources", "*.cs");
			PROJECT->setValue("template", "cs");
			PROJECT->setValue("rccheck", "simple");
			PROJECT->setValue("compilertype", "mcs");
			PROJECT->setValue("language", "cs");
			break;

		case TEMPLATE_C:
		case TEMPLATE_C99:
			LOG("Using template: c", LOG_VERBOSE);
			PROJECT->setValue("compiler", "$(host_dash)gcc");
			PROJECT->setValue("template", "c");
			PROJECT->setValue("compilertype", "gcc");
			PROJECT->setValue("cflags", "Wall");
			PROJECT->addValue("cflags", "g");
			PROJECT->setValue("language", "c");
			PROJECT->setValue("sources", "*.c");
			if(type == TEMPLATE_C99)
				PROJECT->addValue("cflags", "std=c99");
			break;
		
		case TEMPLATE_GCC_AUTO:
			LOG("Using template: gcc-auto (default)", LOG_VERBOSE);
			// note: no break
		default:
			LOG("defualt template", LOG_DEBUG);
			PROJECT->setValue("template", "gcc-auto");

			PROJECT->setValue("cflags", "Wall");
			PROJECT->addValue("cflags", "g");
			PROJECT->addValue("cflags", ".c/std=c99");
			PROJECT->addValue("cflags", ".cpp/std=c++0x");
			PROJECT->addValue("cflags", ".cc/std=c++0x");
			PROJECT->addValue("cflags", ".cxx/std=c++0x");

			PROJECT->setValue("compilertype", "gcc");
			PROJECT->setValue("compiler", "$(host_dash)gcc");
			
			PROJECT->setValue("language", "none");  // gcc detects by extension

			PROJECT->setValue("sources", "*.c");    // c
			PROJECT->addValue("sources", "*.cpp");  // c++
			PROJECT->addValue("sources", "*.cxx"); 
			PROJECT->addValue("sources", "*.cc"); 
			PROJECT->addValue("sources", "*.m");    // objective-c
			PROJECT->addValue("sources", "*.mm");   // objective-c++
			PROJECT->addValue("sources", "*.f");    // fortran
			PROJECT->addValue("sources", "*.f90");  // fortran 90
			PROJECT->addValue("sources", "*.java"); // gjc/java
			PROJECT->addValue("sources", "*.go");   // gccgo/go
			PROJECT->addValue("sources", "*.pas");  // pascal
			//PROJECT->addValue("sources", "*.d");  // gdc/d   -x none can't detect d for some reason

			PROJECT->setValue("stdlibs", "dynamic");
			break;
	}
}


void Spank::printBanner(int banner)
{
	switch(banner){
		case BANNER_LOGO:
			LOG(SPANK_NAME << " " << SPANK_VERSION << " - Welcome to easy street.", LOG_VERBOSE);
			break;

		case BANNER_USAGE:
			LOG("Usage:", LOG_INFO);
			LOG("\tspank [-args] <action> [<alt. project file> or <build configuration>]", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tActions:", LOG_INFO);
			LOG("\t\tbuild         Compiles and links a project", LOG_INFO);
			LOG("\t\trebuild       Cleans, compiles and links a project", LOG_INFO);
			LOG("\t\tclean         Removes the target and temp-files", LOG_INFO);
			LOG("\t\texport        Exports the project to the format given by the 'exporter' argument", LOG_INFO);
			LOG("\t\t                Valid exporters are: sh (shell script) and makefile (make)", LOG_INFO);
			LOG("\t\tinstall       Compiles, links and installs a project", LOG_INFO);
			LOG("\t\tfake-install  Fakes an install (prints the install procedure to screen instead of doing anything)", LOG_INFO);
			LOG("\t\tlist          Lists available build configurations", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tAlternative Project File", LOG_INFO);
			LOG("\t\t<project file>.spank spank/<project file>.spank is added to the list of files to load", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tArguments", LOG_INFO);
			LOG("\t\tTo see available args use 'spank -showconfig'.", LOG_INFO);
			LOG("\t\tThe args can be set either by a built in template", LOG_INFO);
			LOG("\t\ta project file or in the command line.", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\t\tThe project file overrides the built in templates.", LOG_INFO);
			LOG("\t\tThe command line overrides the project file and the templates.", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tTemplates (spank -template <template>):", LOG_INFO);
			LOG("\t\tc (default)", LOG_INFO);
			LOG("\t\tc++", LOG_INFO);
			LOG("\t\tcs (for c#)", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tExporters (spank --exporter <exporter> export):", LOG_INFO);
			LOG("\t\tmakefile (default) - Exports to a make makefile", LOG_INFO);
			LOG("\t\tsh - Exports to a shell script", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tInstallers (spank --installer <installer> install):", LOG_INFO);
			LOG("\t\tunix (default) - Installs project on a unix like system", LOG_INFO);
			LOG("\t\tdeb - Makes a .deb file for installation on a dpkg-based system", LOG_INFO);
			break;
		case BANNER_SEEHELP:
			LOG("See -help for details", LOG_INFO);
			break;
	}
}

void Spank::handleArgs(int argc, const char* const* argv){
	setDefaultConfig();

	AssertEx(PROJECT->fromCmdLine(argc, argv), SpankException, "could not parse command line");

	if(PROJECT->getValueBool("help")){
		printBanner(BANNER_LOGO);
		printBanner(BANNER_USAGE);
		exit(0);
	}

	if(PROJECT->getValueBool("showbuildconfigs")){
		Config::printSectionList(PROJECT->getSectionList());
		exit(0);
	}

	Log::getInstance()->setLogLevel(PROJECT->getValueInt("verbosity", 0));
	
	printBanner(BANNER_LOGO);

	// determine section
	std::string section = "default";
	std::string extraarg = PROJECT->getValueStr("extraarg");
	std::vector<Section> secs = PROJECT->getSectionList();

	if(extraarg != ""){
		// if the default project doesn't contain the section specified as an extra argument
		// try old style loading of extra project files
		if(Config::listHasSection(secs, extraarg)){
			section = extraarg;
		}else{
			FORMSTR(tmp2, extraarg << ".spank");
			PROJECT->setValue("project", tmp2, VAR_CMDLINE); 
			FORMSTR(tmp3, "spank/" << extraarg << ".spank");
			PROJECT->addValue("project", tmp3, VAR_CMDLINE); 
		
			// reload sections since projects were added
			secs = PROJECT->getSectionList();
		}
	}

	if(secs.size() > 0){
		if(section == "default"){
			Section dfl = Config::getDefaultSectionFromList(secs);

			if(dfl.name == ""){
				LOG("no default build configuration in project file(s), please specify one explicitly", LOG_INFO);
				Config::printSectionList(secs);
				ThrowEx(SpankException, "no build configuration specified");
			}

			section = dfl.name;
		}

		if(!Config::listHasSection(secs, section)){
			Config::printSectionList(secs);
			ThrowEx(SpankException, "no such build configuration: " << section);
		}
	}

	LOG("using build configuration: " << section, LOG_VERBOSE);

	bool noConfig = true;

	for(int i=0; i < PROJECT->getNumValues("config"); i++){
		PROJECT->loadConfig(PROJECT->getValueStr("config", i));
	}

	for(int i=0; i < PROJECT->getNumValues("project"); i++){
		if(PROJECT->loadConfig(PROJECT->getValueStr("project", i), section)){
			noConfig = false;
		}
	}

	std::string tmpl = PROJECT->getValueStr("template", 0);

	if(tmpl == "c++" || tmpl == "cpp"){
		setTemplate(TEMPLATE_CPP);
	}else if(tmpl == "c++0x" || tmpl == "c++11" || tmpl == "cpp0x" || tmpl == "cpp11"){
		setTemplate(TEMPLATE_CPP11);
	}else if(tmpl == "c"){
		setTemplate(TEMPLATE_C);
	}else if(tmpl == "c99"){
		setTemplate(TEMPLATE_C99);
	}else if(tmpl == "cs" || tmpl == "csharp"){
		setTemplate(TEMPLATE_CS);
	}else if(tmpl == "vala"){
		setTemplate(TEMPLATE_VALA);
	}else{
		setTemplate(TEMPLATE_GCC_AUTO);
	}

	// Set the target prefix and suffix depnding on target type	
	std::string targetType = PROJECT->getValueStr("targettype");
	if(targetType == "lib-static"){
		PROJECT->setValue("target_prefix", "$(lib-static_prefix)");
		PROJECT->setValue("target_suffix", "$(lib-static_suffix)");
	}
	else if(targetType == "lib-shared"){
		PROJECT->setValue("target_prefix", "$(lib-shared_prefix)");
		PROJECT->setValue("target_suffix", "$(lib-shared_suffix)");
	}

	// predefined platforms	(other than the default: posix)
	std::string targetPlatform = PROJECT->getValueStr("target_platform");

	if(targetPlatform == "mingw32" || targetPlatform == "mingw64"){
		PROJECT->setValue("host", targetPlatform == "mingw32" ? "i686-w64-mingw32" : "x86_64-w64-mingw32");
		PROJECT->setValue("binary_prefix", "");
		PROJECT->setValue("binary_suffix", targetPlatform == "mingw32" ? ".exe" : "_64.exe");
		PROJECT->setValue("lib-shared_prefix", "");
		PROJECT->setValue("lib-shared_suffix", ".dll");
		PROJECT->setValue("inst_prefix", "/usr/$(host)/");
	}
	else if(targetPlatform != "posix"){
		LOG("unknown target platform: '" << targetPlatform << "', assuming posix", LOG_WARNING);
	}

	if(PROJECT->getValueStr("host") != ""){
		PROJECT->setValue("host_dash", "$(host)-");
	}

	if(PROJECT->getValueBool("showconfig", 0)){
		PROJECT->dumpConfig();
	}

	AssertEx(!noConfig, SpankException, "No project file found");
}

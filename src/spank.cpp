// Spank
//
// http://nurd.se/~noname/spank
//
// File authors:
// 	Fredrik Hultin
//
// License: GPLv2

#include <cstdlib>

#include "spank.h"

#include "settings.h"
#include "project.h"
#include "tools.h"
#include "system.h"

Spank::Spank(int argc, char** argv)
{
	templateOnce = false;

	handleArgs(argc, argv);

	if(!FILES->createDir(FILES->getHomeDir())){
		return;
	}

	std::string action = PROJECT->getValueStr("action", 0);
	if(action == "compile"){
		LOG("Action: compile", LOG_DEBUG);
		if(COMPILER->compile()){
			LOG("done compiling", LOG_INFO);
		}else{
			exit(1);
		}
	} 
	
	else if(action == "build"){
		LOG("Action: build", LOG_DEBUG);
		if(COMPILER->compile() && COMPILER->link()){
			LOG("done building", LOG_INFO);
		}
	}
	
	else if(action == "install"){
		LOG("Action: install", LOG_DEBUG);
		INSTALLER->setPrefix();
		if(COMPILER->compile() && COMPILER->link() && INSTALLER->install(false)){
			LOG("done installing", LOG_INFO);
		}
	}

	else if(action == "fake-install"){
		LOG("Action: fake-install", LOG_DEBUG);
		if(INSTALLER->install(true)){
			LOG("done fake-installing", LOG_INFO);
		}
	}

	else if(action == "rebuild"){
		LOG("Action: rebuild", LOG_DEBUG);
		if(COMPILER->clean()){
			LOG("done cleaning", LOG_INFO);
			if(COMPILER->compile() && COMPILER->link()){
				LOG("done rebuilding", LOG_INFO);
			}else{
				exit(1);
			}
		}else{
			exit(1);
		}
	}

	else if(action == "link"){
		LOG("Action: link", LOG_DEBUG);
		if(COMPILER->link()){
			LOG("done linking", LOG_INFO);
		}else{
			exit(1);
		}
	}
	
	else if(action == "clean"){
		LOG("Action: clean", LOG_DEBUG);
		if(COMPILER->clean()){
			LOG("done cleaning", LOG_INFO);
		}else{
			exit(1);
		}
	}

	else if(action == "export"){
		LOG("Action: export", LOG_DEBUG);
		if(EXPORT->exp( PROJECT->getValueStr("exportfile", 0) )){
			LOG("done exporting", LOG_INFO);
		}else{
			LOG("Export failed", LOG_ERROR);
			exit(1);
		}
	}

	else{
		printBanner(BANNER_SEEHELP);
	}
}


void Spank::setTemplate(int type)
{
	if(!templateOnce){
		templateOnce = true;

		// Common defaults
		
		PROJECT->setValue("project", "project.spank");
		PROJECT->addValue("project", "spank/project.spank");
		PROJECT->setValue("config", "$(HOME)/.spank/config.spank");
		PROJECT->setValue("target", "out");
		PROJECT->setValue("sourcedir", ".");
		PROJECT->setValue("cflags", "Iinclude");
		PROJECT->setValue("action", "none");
		PROJECT->setValue("template", "default");
		PROJECT->setValue("help", "no");
		PROJECT->setValue("tmpdir", "");
		PROJECT->setValue("verbosity", "2");
		PROJECT->setValue("showconfig", "false");
		PROJECT->setValue("ldflags", "");
		PROJECT->setValue("exclude", "");
		PROJECT->setValue("rccheck", "pp");
		PROJECT->setValue("pp", "$(compiler) -E");
		PROJECT->setValue("spankdefs", "true");
		PROJECT->setValue("exporter", "makefile");
		PROJECT->setValue("exportfile", "");
		PROJECT->setValue("lib", "");
		PROJECT->setValue("pkg-config", "export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:.:spank && pkg-config");
		PROJECT->setValue("tar", "tar");
		PROJECT->setValue("ar", "ar");
		PROJECT->setValue("stripsrc", "false");
		PROJECT->setValue("homedir", "$(HOME)/.spank");
		PROJECT->setValue("jobs", "2");
		PROJECT->addValue("addhyphen", "true");
		PROJECT->addValue("prebuildscript", "");
		PROJECT->addValue("postbuildscript", "");
		PROJECT->addValue("oncleanscript", "");
		PROJECT->addValue("version", "0.1");
		PROJECT->addValue("name", "untitled project");
		PROJECT->addValue("homepage", "none");
		PROJECT->addValue("email", "nomail@example.com");
		PROJECT->addValue("author", "author of $(name)");
		PROJECT->addValue("description", "$(name) $(version)");
		PROJECT->addValue("include", "");
		PROJECT->setValue("targettype", "binary");
		/*PROJECT->setValue("fpic", "fPIC");*/

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
	}
		
	// Template specific defaults

	switch(type){
		case TEMPLATE_CPP:
			LOG("Using template: c++", LOG_INFO);
			PROJECT->setValue("compiler", "g++");
			PROJECT->setValue("sources", "*.cpp");
			PROJECT->setValue("template", "c++");
			break;

		case TEMPLATE_CS:
			LOG("Using template: c#", LOG_INFO);
			PROJECT->setValue("compiler", "gmcs");
			PROJECT->setValue("sources", "*.cs");
			PROJECT->setValue("template", "cs");
			PROJECT->setValue("cflags", "");
			PROJECT->setValue("rccheck", "simple");
			break;

		case TEMPLATE_C:
			LOG("Using template: c", LOG_INFO);
			PROJECT->setValue("template", "c");
			// note: no break
		
		default:
			PROJECT->setValue("compiler", "gcc");
			PROJECT->setValue("sources", "*.c");
			break;
	}

}


void Spank::printBanner(int banner)
{
	switch(banner){
		case BANNER_LOGO:
			LOG(SPANK_NAME << " " << SPANK_VERSION << " - Welcome to easy street.", LOG_INFO);
			break;

		case BANNER_USAGE:
			LOG("Usage:", LOG_INFO);
			LOG("\tspank [-args] <action> [<alt. project file>]", LOG_INFO);
			LOG("", LOG_INFO);
			LOG("\tActions:", LOG_INFO);
			LOG("\t\tinstall\tCompiles, links and installs a project", LOG_INFO);
			LOG("\t\tfake-install\tFakes an install (prints the install procedure to screen instead of doing anything)", LOG_INFO);
			LOG("\t\tinstall\tCompiles, links and installs a project", LOG_INFO);
			LOG("\t\tbuild\tCompiles and links a project", LOG_INFO);
			LOG("\t\trebuild\tCleans, compiles and links a project", LOG_INFO);
			LOG("\t\tcompile\tCompiles the project sources", LOG_INFO);
			LOG("\t\tlink\tLinks the compiled files", LOG_INFO);
			LOG("\t\tclean\tRemoves the target and temp-files", LOG_INFO);
			LOG("\t\texport\tExports the project to the format given by the 'exporter' argument", LOG_INFO);
			LOG("\t\t\tValid exporters are: sh (shell script) and makefile (make)", LOG_INFO);
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

	setTemplate(TEMPLATE_DEFAULT);

	if(!PROJECT->fromCmdLine(argc, argv)){
		printBanner(BANNER_LOGO);
		exit(1);
	}else if(PROJECT->getValueBool("help", 0)){
		printBanner(BANNER_LOGO);
		printBanner(BANNER_USAGE);
		exit(0);
	}else{
		LOGI->setLogLevel(PROJECT->getValueInt("verbosity", 0));
		printBanner(BANNER_LOGO);

		bool noConfig = true;

		for(int i=0; i < PROJECT->getNumValues("config"); i++){
			PROJECT->loadConfig(PROJECT->getValueStr("config", i));
		}
	
		for(int i=0; i < PROJECT->getNumValues("project"); i++){
			if(PROJECT->loadConfig(PROJECT->getValueStr("project", i))){
				noConfig = false;
			}
		}
	
		std::string tmpl = PROJECT->getValueStr("template", 0);

		if(tmpl == "c++" || tmpl == "cpp"){
			setTemplate(TEMPLATE_CPP);
		}else if(tmpl == "c"){
			setTemplate(TEMPLATE_C);
		}else if(tmpl == "cs" || tmpl == "csharp"){
			setTemplate(TEMPLATE_CS);
		}else{
			setTemplate(TEMPLATE_DEFAULT);
		}

		if(PROJECT->getValueBool("showconfig", 0)){
			PROJECT->dumpConfig();
		}
		
		if(noConfig){
			LOG("No project file found.", LOG_ERROR);
			exit(1);
		}

	}
}

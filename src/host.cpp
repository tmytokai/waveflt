#include "host.h"
#include "module.h"

Host::Host()
  : id(0)
{}


Host::~Host()
{
    std::vector<Module*>::iterator it = sources.begin();
    for( ; it != sources.end(); ++it ) delete (*it);

    it = modules.begin();
    for( ; it != modules.end(); ++it ) delete (*it);

    it = outputs.begin();
    for( ; it != outputs.end(); ++it ) delete (*it);
}


void Host::regist( Module* module )
{
    if( module->get_name() == "Source" ) sources.push_back(module);
    else if( module->get_name() == "Output" ) outputs.push_back(module);
    else modules.push_back(module);

    ++id;
}


void Host::init()
{
    std::vector<Module*>::iterator it = sources.begin();
    for( ; it != sources.end(); ++it ) (*it)->init();
}


const bool Host::is_over()
{
    std::vector<Module*>::iterator it = outputs.begin();
    for( ; it != outputs.end(); ++it ) if( ! (*it)->is_over() ) return false;

    return true;
}


const std::string Host::get_config()
{
    std::string str;
    std::vector<Module*>::iterator it = sources.begin();
    for( ; it != sources.end(); ++it ) str += (*it)->get_config();
    return str;
}


void Host::start()
{
    std::vector<Module*>::iterator it = sources.begin();
    for( ; it != sources.end(); ++it ) (*it)->start();
}


void Host::process()
{
    std::vector<Module*>::iterator it = outputs.begin();
    for( ; it != outputs.end(); ++it ) (*it)->process();
}


const std::string Host::get_status()
{
    std::string str;
    std::vector<Module*>::iterator it = outputs.begin();
    for( ; it != outputs.end(); ++it ) str += (*it)->get_status();
    return str;
}


const std::string Host::get_result()
{
    std::string str;
    std::vector<Module*>::iterator it = sources.begin();
    for( ; it != sources.end(); ++it ) str += (*it)->get_result();
    return str;
}

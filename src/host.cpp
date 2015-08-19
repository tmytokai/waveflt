#include "host.h"
#include "module.h"

Host::Host()
  : id(0)
{}


Host::~Host()
{
    std::vector<Module*>::iterator it = modules.begin();
    for( ; it != modules.end(); ++it ) delete (*it);
}


void Host::regist( Module* module )
{
    modules.push_back(module);
    ++id;
}

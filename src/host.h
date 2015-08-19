// host class

#ifndef _HOST_H
#define _HOST_H

#include <vector>

class Module;

class Host
{
  private:

    std::vector<Module*> modules;
    unsigned int id;
    
  public:

    Host();
    ~Host();

    const unsigned int get_id() const { return id; }
    void regist( Module* module );
};

#endif

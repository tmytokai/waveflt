// host class

#ifndef _HOST_H
#define _HOST_H

#include <vector>
#include <string>

class Module;

class Host
{
  private:

    std::vector<Module*> sources;
    std::vector<Module*> modules;
    unsigned int id;
    
  public:

    Host();
    ~Host();

    const unsigned int get_id() const { return id; }
    void regist( Module* module );

    void init();
    const std::string get_config();
    void start();
    const std::string get_result();
};

#endif

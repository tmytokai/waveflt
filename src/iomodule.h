// abstract class of IO module

#ifndef _IOMODULE_H
#define _IOMODULE_H

#include "module.h"
#include "doublebuffer.h"

class IO;

class IOModule : public Module
{
  protected:

    const std::string filename;
    IO* io;
    DoubleBuffer data;
    bool raw_mode; // if Source and Output are connected directly, use only raw data instead of double data

  public:

    IOModule( const std::string& _name, const std::string& _filename ) : Module(_name), filename(_filename), io(NULL), raw_mode(false){}
    virtual ~IOModule(){}

    const unsigned int get_max_points() const { return data.max_points; }
    const unsigned int get_points() const { return data.points; }
    const double* get_data( const unsigned int ch ) { return data[ch]; }
};

#endif

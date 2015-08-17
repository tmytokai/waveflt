// abstract class of IO module

#ifndef _IOMODULE_H
#define _IOMODULE_H

#include "module.h"
#include "doublebuffer.h"
#include "io.h"

class IOModule : public Module
{
  protected:

    const std::string filename;
    IO* io;
    DoubleBuffer data;

  public:

    IOModule( const std::string& _name, const unsigned int _id, const std::string& _filename ) : Module(_name,_id), filename(_filename), io(NULL){}
    virtual ~IOModule(){}

    const unsigned int get_max_points() const { return data.max_points; }
    const unsigned int get_points() const { return data.points; }
    const double* get_data( const unsigned int ch ) { return data[ch]; }

	// Override
	virtual void reset(){

		Module::reset();

		if( io ) delete io;
		io = NULL;
		data.reset();
	}

	virtual void clear_buffer(){

		Module::clear_buffer();

		data.clear_buffer();
	}
};

#endif

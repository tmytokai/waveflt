// output class

#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "iomodule.h"


class Output : public IOModule
{
  public:

    Output( Host* host, const std::string& _filename );
    virtual ~Output();

    void process(){ requested( data.left() ); }

    // Override
    virtual void connect( Module* _next );

    // Override
    virtual void reset();
    virtual void clear_buffer();
    virtual void init();
    virtual const std::string get_config() const;
    virtual void start();
    virtual void exec_event();
    virtual void requested( const unsigned int points_required );
    virtual void received( Module* sender, DoubleBuffer& data );
    virtual const std::string get_result() const;
};

#endif

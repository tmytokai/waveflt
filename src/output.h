// output class

#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "iomodule.h"


class Output : public IOModule
{
  public:

    Output( const std::string& _filename );
    virtual ~Output();

    void process(){ requested( data.left() ); }

    // Override
    virtual void connect( Module* _next );

    // Override
    virtual void reset_all();
    virtual void clear_all_buffer();
    virtual void init();
    virtual void show_config() const;
    virtual void start();
    virtual void exec_event();
    virtual void requested( const unsigned int points_required );
    virtual void received( Module* sender, DoubleBuffer& data, const bool fin );
    virtual void show_result() const;
};

#endif

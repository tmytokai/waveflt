// source class

#ifndef _SOURCE_H
#define _SOURCE_H

#include "iomodule.h"


class Source : public IOModule
{
  public:

    Source( const std::string& _filename );
    virtual ~Source();

    // Override
    virtual void connected( Module* _prev );

    // Override
    virtual void reset_all();
    virtual void clear_all_buffer();
    virtual void init();
    virtual const std::string get_config() const;
    virtual void start();
    virtual void exec_event();
    virtual void requested( const unsigned int points_required );
    virtual void received( Module* sender, DoubleBuffer& data );
    virtual void show_result() const;
};

#endif

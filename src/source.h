// source class

#ifndef _SOURCE_H
#define _SOURCE_H

#include "iomodule.h"


class Source : public IOModule
{
  private:

    unsigned long long total_read_points; // total read points

  public:

    Source( Host* host, const std::string& _filename );
    virtual ~Source();

    // Override
    virtual void connected( Module* _prev );

    // Override
    virtual void reset();
    virtual void clear_buffer();
    virtual void init();
    virtual const std::string get_config() const;
    virtual void start();
    virtual void exec_event();
    virtual void process(){}
    virtual void requested( const unsigned int points_required );
    virtual void received( Module* sender, DoubleBuffer& data );
    virtual const std::string get_status() const;
    virtual const std::string get_result() const;
};

#endif

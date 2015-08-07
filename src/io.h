// abstract class of IO source

#ifndef _IO_H
#define _IO_H

#include <string>

enum
{
    IOMODE_READ = 0,
    IOMODE_WRITE,
    IOMODE_INIT
};


class IO
{
  protected:

    bool dbg;
    std::string name;
    unsigned int mode;

  public:

    IO( const std::string& _name): dbg(false), name(_name), mode(IOMODE_INIT){}
    virtual ~IO(){}

    void debugmode(){ dbg = true; }
    const std::string& get_name() const { return name; }
    const unsigned int get_mode() const { return mode; }

    virtual void open( const unsigned int mode ) = 0;
    virtual void close() = 0;
    virtual void seek( const unsigned long long offset_byte ) = 0;
    virtual const unsigned int read( void* data, const unsigned int byte ) = 0;
    virtual const unsigned int write( const void* data, const unsigned int byte ) = 0;
};

#endif
